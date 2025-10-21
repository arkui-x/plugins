/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef VIBRATOR_AGENT_TYPE_H
#define VIBRATOR_AGENT_TYPE_H
#include <string>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NAME_MAX_LEN
#define NAME_MAX_LEN 128
#endif /* NAME_MAX_LEN */
#ifndef VIBRATOR_USER_DATA_SIZE
#define VIBRATOR_USER_DATA_SIZE 104
#endif /* VIBRATOR_USER_DATA_SIZE */

/**
 * @brief Describes the vibration effect of the vibrator when a user adjusts the timer.
 *
 * @since 1
 */
inline const char *VIBRATOR_TYPE_CLOCK_TIMER = "haptic.clock.timer";

/**
 * @brief Describes the vibration effect of the vibrator when authentication fails.
 *
 * @since 11
 */
inline const char *VIBRATOR_TYPE_FAIL = "haptic.notice.fail";

/**
 * @brief Describes the vibration effect of the vibrator when charging.
 *
 * @since 11
 */
inline const char *VIBRATOR_TYPE_CHARGING = "haptic.calib.charge";

/**
 * @brief Describes the vibration effect of the vibrator when long pressed.
 *
 * @since 11
 */
inline const char *VIBRATOR_TYPE_LONG_PRESS_LIGHT = "haptic.long_press_light";

/**
 * @brief Describes the vibration effect of the vibrator when long pressed.
 *
 * @since 11
 */
inline const char *VIBRATOR_TYPE_LONG_PRESS_MEDIUM = "haptic.long_press_medium";

/**
 * @brief Describes the vibration effect of the vibrator when long pressed.
 *
 * @since 11
 */
inline const char *VIBRATOR_TYPE_LONG_PRESS_HEAVY = "haptic.long_press_medium";

/**
 * @brief Describes the vibration effect of the vibrator when slide.
 *
 * @since 11
 */
inline const char *VIBRATOR_TYPE_SLIDE_LIGHT = "haptic.slide";

/**
 * @brief Describes the vibration effect of the vibrator when the threshold is reached.
 *
 * @since 11
 */
inline const char *VIBRATOR_TYPE_THRESHOID = "haptic.common.threshold";

/**
 * @brief Describes the hard vibration effect of the vibrator.
 *
 * @since 12
 */
inline const char *VIBRATOR_TYPE_HARD = "haptic.effect.hard";

/**
 * @brief Describes the soft vibration effect of the vibrator.
 *
 * @since 12
 */
inline const char *VIBRATOR_TYPE_SOFT = "haptic.effect.soft";

/**
 * @brief Describes the sharp vibration effect of the vibrator.
 *
 * @since 12
 */
inline const char *VIBRATOR_TYPE_SHARP = "haptic.effect.sharp";

/**
 * @brief Describes the vibration effect of the vibrator when slide.
 *
 * @since 12
 */
inline const char *VIBRATOR_TYPE_SLIDE = "haptic.slide";

/**
 * @brief Describes the vibration effect of the succeed notice.
 *
 * @since 16
 */
inline const char *VIBRATOR_TYPE_NOTICE_SUCCESS = "haptic.notice.success";

/**
 * @brief Describes the vibration effect of the failed notice.
 *
 * @since 16
 */
inline const char *VIBRATOR_TYPE_NOTICE_FAILURE = "haptic.notice.fail";

/**
 * @brief Describes the vibration effect of the warning notice.
 *
 * @since 16
 */
inline const char *VIBRATOR_TYPE_NOTICE_WARNING = "haptic.notice.warning";

/**
 * @brief Enumerates vibration usages.
 *
 * @since 9
 */
typedef enum VibratorUsage {
    USAGE_UNKNOWN = 0,            /**< Vibration is used for unknown, lowest priority */
    USAGE_ALARM = 1,              /**< Vibration is used for alarm */
    USAGE_RING = 2,               /**< Vibration is used for ring */
    USAGE_NOTIFICATION = 3,       /**< Vibration is used for notification */
    USAGE_COMMUNICATION = 4,      /**< Vibration is used for communication */
    USAGE_TOUCH = 5,              /**< Vibration is used for touch */
    USAGE_MEDIA = 6,              /**< Vibration is used for media */
    USAGE_PHYSICAL_FEEDBACK = 7,  /**< Vibration is used for physical feedback */
    USAGE_SIMULATE_REALITY = 8,   /**< Vibration is used for simulate reality */
    USAGE_MAX
} VibratorUsage;

/**
 * @brief Vibration effect description file.
 *
 * @since 11
 */
typedef struct VibratorFileDescription {
    int32_t fd = -1;
    int64_t offset = -1;
    int64_t length = -1;
} VibratorFileDescription;

/**
 * @brief The type of vibration event.
 *
 * @since 11
 */
typedef enum VibratorEventType {
    EVENT_TYPE_UNKNOWN = -1, /**< Unknown vibration event type */
    EVENT_TYPE_CONTINUOUS = 0, /**< Continuous vibration event type */
    EVENT_TYPE_TRANSIENT = 1, /**< Transient vibration event type */
} VibratorEventType;

/**
 * @brief Vibration curve adjustment point.
 *
 * @since 11
 */
typedef struct VibratorCurvePoint {
    int32_t time = -1;
    int32_t intensity = -1;
    int32_t frequency = -1;
} VibratorCurvePoint;

/**
 * @brief The vibration event.
 *
 * @since 11
 */
typedef struct VibratorEvent {
    VibratorEventType type = EVENT_TYPE_UNKNOWN;
    int32_t time = -1;
    int32_t duration = -1;
    int32_t intensity = -1;
    int32_t frequency = -1;
    int32_t index = 0; // 0:both 1:left 2:right
    int32_t pointNum = 0;
    VibratorCurvePoint *points = nullptr;
} VibratorEvent;

/**
 * @brief Describe the vibration pattern, including the vibration event sequence.
 *
 * @since 11
 */
typedef struct VibratorPattern {
    int32_t time = -1;
    int32_t eventNum = 0;
    int32_t patternDuration = 0;
    VibratorEvent *events = nullptr;
} VibratorPattern;

/**
 * @brief Describes the vibration package structure, including the vibration pattern sequence.
 *
 * @since 11
 */
typedef struct VibratorPackage {
    int32_t patternNum = 0; // pattern
    int32_t packageDuration = 0;
    VibratorPattern *patterns = nullptr;
} VibratorPackage;

/**
 * @brief Vibration effect adjustment parameters.
 *
 * @since 11
 */
typedef struct VibratorParameter {
    int32_t intensity = 100;  // from 0 to 100
    int32_t frequency = 0;    // from -100 to 100
    int32_t reserved = 0;
    uint32_t sessionId = 0;
} VibratorParameter;

/**
 * @brief Represents the information about a vibrator device in the system.
 *
 * @since 19
 */
typedef struct VibratorInfos {
    int32_t deviceId;
    int32_t vibratorId;
    std::string deviceName;
    bool isSupportHdHaptic;
    bool isLocalVibrator;
} VibratorInfos;

/**
 * @brief Represents the parameters for querying vibrator information.
 *
 * @since 19
 */
typedef struct VibratorIdentifier {
    int32_t deviceId = -1;
    int32_t vibratorId = -1;
    bool operator<(const VibratorIdentifier &other) const
    {
        if (deviceId != other.deviceId) {
            return deviceId < other.deviceId;
        }
        return vibratorId < other.vibratorId;
    }
} VibratorIdentifier;

/**
 * @brief Defines the vibration effect information.
 *
 * The information include the capability to set the effect and the vibration duration of the effect.
 *
 * @since 19
 */
typedef struct EffectInfo {
    bool isSupportEffect = false;
} EffectInfo;

/**
 * @brief Defines the plug state events for the vibrator device.
 *
 * This enumeration represents the various plug and unplug state events
 * that can occur with the vibrator device, including its connection status.
 *
 * @since 19
 */
typedef enum VibratorPlugState {
    PLUG_STATE_EVENT_UNKNOWN = -1,  /* Unknown plug and unplug state event */
    PLUG_STATE_EVENT_PLUG_OUT = 0,  /* Event indicating that the vibrator is unplugged */
    PLUG_STATE_EVENT_PLUG_IN = 1,   /* Event indicating that the vibrator is plugged in */
} VibratorPlugState;

/**
 * @brief Contains information about a vibrator device's status.
 *
 * This structure holds the current plug state of the vibrator device and its
 * unique identifier. It is used to monitor and manage the vibrator's connectivity
 * and operational mode.
 *
 * @since 19
 */
typedef struct VibratorStatusEvent {
    VibratorPlugState type = PLUG_STATE_EVENT_UNKNOWN;
    int32_t deviceId = 0;
    int32_t vibratorCnt = 0;
    int64_t timestamp = 0;
} VibratorStatusEvent;

/**
 * @brief Defines the callback for data reporting by the sensor agent.
 *
 * @since 19
 */
typedef void (*RecordVibratorPlugCallback)(VibratorStatusEvent *statusEvent);

/**
 * @brief Defines a reserved field for the sensor data subscriber.
 *
 * @since 19
 */
typedef struct UserData {
    char userData[VIBRATOR_USER_DATA_SIZE];  /* Reserved for the sensor data subscriber */
} UserData;

/**
 * @brief Defines information about the sensor data subscriber.
 *
 * @since 19
 */
typedef struct VibratorUser {
    RecordVibratorPlugCallback callback;       /* Callback for reporting sensor data */
    UserData *userData = nullptr;              /* Reserved field for the sensor data subscriber */
} VibratorUser;
/** @} */
#ifdef __cplusplus
};
#endif

#endif  // endif VIBRATOR_AGENT_TYPE_H