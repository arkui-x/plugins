/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "sensor_service.h"

#include <android/looper.h>
#include <android/sensor.h>
#include <atomic>
#include <cmath>
#include <cstring>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "hilog/log.h"

namespace OHOS {
namespace Sensors {
namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t IS_NOT_SUPPORTED = 801;
constexpr int32_t SERVICE_EXCEPTION = 14500101;
constexpr int32_t LOOPER_POLL_TIMEOUT_MS = 100;
constexpr int32_t SENSOR_EVENT_QUEUE_ID = 1;
constexpr int64_t DEFAULT_INTERVAL_NS = 200000000;
constexpr double RADIAN_TO_DEGREE = 57.29577951308232;

#define SENSOR_LOGI(fmt, ...) HILOG_INFO(LOG_CORE, fmt, ##__VA_ARGS__)
#define SENSOR_LOGW(fmt, ...) HILOG_WARN(LOG_CORE, fmt, ##__VA_ARGS__)
#define SENSOR_LOGE(fmt, ...) HILOG_ERROR(LOG_CORE, fmt, ##__VA_ARGS__)

/**
 * @brief Translates the OpenHarmony sensor type to the Android NDK type.
 *
 * @return The Android sensor type string constant, or nullptr when the type
 *         is outside the scope of this adaptation.
 */
int32_t GetAndroidSensorType(int32_t sensorTypeId)
{
    switch (sensorTypeId) {
        case SENSOR_TYPE_ID_ACCELEROMETER:
            return ASENSOR_TYPE_ACCELEROMETER;
        case SENSOR_TYPE_ID_GYROSCOPE:
            return ASENSOR_TYPE_GYROSCOPE;
        case SENSOR_TYPE_ID_AMBIENT_LIGHT:
            return ASENSOR_TYPE_LIGHT;
        case SENSOR_TYPE_ID_BAROMETER:
            return ASENSOR_TYPE_PRESSURE;
        case SENSOR_TYPE_ID_PROXIMITY:
            return ASENSOR_TYPE_PROXIMITY;
        case SENSOR_TYPE_ID_ORIENTATION:
            return ASENSOR_TYPE_ROTATION_VECTOR;
        default:
            return -1;
    }
}

/**
 * @brief Converts a rotation vector sample to the OpenHarmony orientation
 *        angles alpha (z), beta (x) and gamma (y), in degrees.
 *
 * The algorithm mirrors the platform SensorManager.getOrientation result:
 * the quaternion is expanded into a rotation matrix, then the matrix is
 * decomposed into Tait-Bryan angles.
 */
void ConvertRotationVectorToOrientation(const float *input, SensorEventData &event)
{
    // The rotation vector stores the unit axis multiplied by sin(theta/2);
    // the fourth component carries cos(theta/2).
    float halfSinX = input[0];
    float halfSinY = input[1];
    float halfSinZ = input[2];
    float halfCos = (input[3] <= 0.0f) ? 0.0f : input[3];
    float norm = std::sqrt(halfSinX * halfSinX + halfSinY * halfSinY + halfSinZ * halfSinZ + halfCos * halfCos);
    if (norm < std::numeric_limits<float>::epsilon()) {
        return;
    }
    float qx = halfSinX / norm;
    float qy = halfSinY / norm;
    float qz = halfSinZ / norm;
    float qw = halfCos / norm;

    // Quaternion to rotation matrix; only the rows needed by the angle
    // extraction below are materialized.
    float r10 = 2.0f * (qx * qy + qw * qz);
    float r20 = 2.0f * (qx * qz - qw * qy);
    float r21 = 2.0f * (qy * qz + qw * qx);
    float r22 = 2.0f * (qw * qw + qz * qz) - 1.0f;

    double alpha = std::atan2(static_cast<double>(r10), static_cast<double>(r10 * 0.0f + 2.0f * (qw * qw + qx * qx)
        - 1.0f));
    double beta = std::atan2(-static_cast<double>(r20),
        std::sqrt(static_cast<double>(r21) * r21 + static_cast<double>(r22) * r22));
    double gamma = std::atan2(static_cast<double>(r21), static_cast<double>(r22));
    // Normalize alpha from (-180, 180] to [0, 360).
    if (alpha < 0.0) {
        alpha += 2.0 * M_PI;
    }
    event.values[0] = alpha * RADIAN_TO_DEGREE;
    event.values[1] = beta * RADIAN_TO_DEGREE;
    event.values[2] = gamma * RADIAN_TO_DEGREE;
}

/**
 * @brief Platform adapter based on the Android NDK sensor framework.
 *
 * A single worker thread owns one ALooper; every subscription adds an
 * ASensorEventQueue to that looper, so all sensor samples of the process
 * arrive on the same thread.
 */
class SensorServiceAndroid final : public ISensorService {
public:
    SensorServiceAndroid() = default;
    ~SensorServiceAndroid() override;

    int32_t SubscribeSensor(int32_t sensorTypeId, int64_t intervalNs,
                            const SensorDataCallback &callback) override;
    int32_t UnsubscribeSensor(int32_t sensorTypeId) override;

private:
    struct Subscription {
        const ASensor *sensor = nullptr;
        ASensorEventQueue *queue = nullptr;
        SensorDataCallback callback;
        int32_t sensorTypeId = -1;
    };

    void EnsureLooperStarted();
    static void *LooperThreadEntry(void *context);
    void RunLooper();
    static int HandleEvent(int fd, int events, void *data);
    void DispatchEvents(Subscription &subscription);

    std::mutex mutex_;
    ALooper *looper_ = nullptr;
    std::thread looperThread_;
    std::unordered_map<int32_t, Subscription> subscriptions_;
    ASensorManager *manager_ = nullptr;
};

SensorServiceAndroid::~SensorServiceAndroid()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto &item : subscriptions_) {
            if (item.second.queue != nullptr) {
                ASensorManager_destroyEventQueue(manager_, item.second.queue);
            }
        }
        subscriptions_.clear();
    }
    if (looper_ != nullptr) {
        ALooper_acquire(looper_);
        ALooper_wake(looper_);
        ALooper_release(looper_);
    }
    if (looperThread_.joinable()) {
        looperThread_.join();
    }
}

void SensorServiceAndroid::EnsureLooperStarted()
{
    if (manager_ == nullptr) {
        manager_ = ASensorManager_getInstanceForPackage("com.arkui.x");
    }
    if (looper_ == nullptr) {
        looper_ = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
        looperThread_ = std::thread(&SensorServiceAndroid::RunLooper, this);
    }
}

void SensorServiceAndroid::RunLooper()
{
    SENSOR_LOGI("Sensor looper thread started");
    while (true) {
        int events = 0;
        int source = 0;
        void *data = nullptr;
        int result = ALooper_pollAll(LOOPER_POLL_TIMEOUT_MS, &source, &events, &data);
        if (result == ALOOPER_POLL_WAKE) {
            SENSOR_LOGI("Sensor looper thread exiting");
            return;
        }
    }
}

int SensorServiceAndroid::HandleEvent(int fd, int events, void *data)
{
    auto *subscription = static_cast<Subscription *>(data);
    if (subscription == nullptr) {
        return 1; // Remove the callback registration.
    }
    auto *self = const_cast<SensorServiceAndroid *>(
        static_cast<const SensorServiceAndroid *>(nullptr));
    (void)self;
    (void)fd;
    (void)events;
    return 1;
}

void SensorServiceAndroid::DispatchEvents(Subscription &subscription)
{
    ASensorEvent event;
    while (ASensorEventQueue_getEvents(subscription.queue, &event, 1) > 0) {
        SensorEventData data;
        data.sensorTypeId = subscription.sensorTypeId;
        data.accuracy = event.acceleration.status;
        data.timestamp = static_cast<int64_t>(event.timestamp) * 1000; // us -> ns
        switch (subscription.sensorTypeId) {
            case SENSOR_TYPE_ID_ACCELEROMETER:
                data.values[0] = event.acceleration.x;
                data.values[1] = event.acceleration.y;
                data.values[2] = event.acceleration.z;
                break;
            case SENSOR_TYPE_ID_GYROSCOPE:
                data.values[0] = event.vector.x;
                data.values[1] = event.vector.y;
                data.values[2] = event.vector.z;
                break;
            case SENSOR_TYPE_ID_AMBIENT_LIGHT:
                data.values[0] = event.light;
                break;
            case SENSOR_TYPE_ID_BAROMETER:
                data.values[0] = event.pressure;
                break;
            case SENSOR_TYPE_ID_PROXIMITY:
                data.values[0] = event.distance;
                break;
            case SENSOR_TYPE_ID_ORIENTATION:
                ConvertRotationVectorToOrientation(event.data, data);
                break;
            default:
                break;
        }
        subscription.callback(data);
    }
}

int32_t SensorServiceAndroid::SubscribeSensor(int32_t sensorTypeId, int64_t intervalNs,
                                              const SensorDataCallback &callback)
{
    int32_t androidType = GetAndroidSensorType(sensorTypeId);
    if (androidType < 0) {
        SENSOR_LOGE("Sensor type %{public}d is out of adaptation scope", sensorTypeId);
        return IS_NOT_SUPPORTED;
    }
    if (callback == nullptr) {
        return IS_NOT_SUPPORTED;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    EnsureLooperStarted();
    if (manager_ == nullptr || looper_ == nullptr) {
        SENSOR_LOGE("Sensor manager or looper is unavailable");
        return SERVICE_EXCEPTION;
    }
    auto existed = subscriptions_.find(sensorTypeId);
    if (existed != subscriptions_.end()) {
        // Re-subscription refreshes the callback and keeps the queue alive.
        existed->second.callback = callback;
        ASensorEventQueue_setEventRate(existed->second.queue, existed->second.sensor,
                                       static_cast<int32_t>(intervalNs / 1000)); // ns -> us
        return ERR_OK;
    }
    const ASensor *sensor = ASensorManager_getDefaultSensor(manager_, androidType);
    if (sensor == nullptr) {
        SENSOR_LOGW("Device has no sensor of type %{public}d", sensorTypeId);
        return SERVICE_EXCEPTION;
    }
    Subscription subscription;
    subscription.sensor = sensor;
    subscription.sensorTypeId = sensorTypeId;
    subscription.callback = callback;
    subscription.queue = ASensorManager_createEventQueue(manager_, looper_, SENSOR_EVENT_QUEUE_ID, nullptr, nullptr);
    if (subscription.queue == nullptr) {
        SENSOR_LOGE("Create sensor event queue failed, type %{public}d", sensorTypeId);
        return SERVICE_EXCEPTION;
    }
    if (ASensorEventQueue_enableSensor(subscription.queue, sensor) != 0) {
        SENSOR_LOGE("Enable sensor failed, type %{public}d", sensorTypeId);
        ASensorManager_destroyEventQueue(manager_, subscription.queue);
        return SERVICE_EXCEPTION;
    }
    int64_t interval = intervalNs;
    if (interval <= 0) {
        interval = DEFAULT_INTERVAL_NS;
    }
    ASensorEventQueue_setEventRate(subscription.queue, sensor, static_cast<int32_t>(interval / 1000));
    subscriptions_.emplace(sensorTypeId, subscription);
    SENSOR_LOGI("Sensor %{public}d subscribed", sensorTypeId);
    return ERR_OK;
}

int32_t SensorServiceAndroid::UnsubscribeSensor(int32_t sensorTypeId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto existed = subscriptions_.find(sensorTypeId);
    if (existed == subscriptions_.end()) {
        return ERR_OK;
    }
    ASensorEventQueue_disableSensor(existed->second.queue, existed->second.sensor);
    ASensorManager_destroyEventQueue(manager_, existed->second.queue);
    subscriptions_.erase(existed);
    SENSOR_LOGI("Sensor %{public}d unsubscribed", sensorTypeId);
    return ERR_OK;
}
} // namespace

ISensorService &GetSensorService()
{
    static SensorServiceAndroid instance;
    return instance;
}
} // namespace Sensors
} // namespace OHOS
