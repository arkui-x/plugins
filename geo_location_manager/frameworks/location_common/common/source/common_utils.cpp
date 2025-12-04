/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include <random>
#include <sys/time.h>
#include <sstream>
#include <chrono>

#include "common_utils.h"

namespace OHOS {
namespace Location {
static std::random_device g_randomDevice;
static std::mt19937 g_gen(g_randomDevice());
static std::uniform_int_distribution<> g_dis(0, 15);   // random between 0 and 15
static std::uniform_int_distribution<> g_dis2(8, 11);  // random between 8 and 11

void CountDownLatch::Wait(int time)
{
    LBSLOGI(LOCATOR_STANDARD, "enter wait, time = %{public}d", time);
    std::unique_lock<std::mutex> lock(mutex_);
    if (count_ == 0) {
        LBSLOGE(LOCATOR_STANDARD, "count_ = 0");
        return;
    }
    condition_.wait_for(lock, std::chrono::seconds(time / MILLI_PER_SEC), [&]() {return count_ == 0;});
}

void CountDownLatch::CountDown()
{
    LBSLOGI(LOCATOR_STANDARD, "enter CountDown");
    std::unique_lock<std::mutex> lock(mutex_);
    int oldC = count_.load();
    while (oldC > 0) {
        if (count_.compare_exchange_strong(oldC, oldC - 1)) {
            if (oldC == 1) {
                LBSLOGI(LOCATOR_STANDARD, "notify_all");
                condition_.notify_all();
            }
            break;
        }
        oldC = count_.load();
    }
}

int CountDownLatch::GetCount()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return count_;
}

void CountDownLatch::SetCount(int count)
{
    std::unique_lock<std::mutex> lock(mutex_);
    count_ = count;
}

int64_t CommonUtils::GetCurrentTime()
{
    struct timespec times = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &times);
    int64_t second = static_cast<int64_t>(times.tv_sec);
    return second;
}

int64_t CommonUtils::GetCurrentTimeMilSec()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto milliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return milliSeconds;
}

int64_t CommonUtils::GetCurrentTimeStamp()
{
    struct timeval currentTime;
    gettimeofday(&currentTime, nullptr);
    return static_cast<int64_t>(currentTime.tv_sec);
}

std::string CommonUtils::GenerateUuid()
{
    std::stringstream ss;
    int i;
    ss << std::hex;
    for (i = 0; i < 8; i++) {  // first group 8 bit for UUID
        ss << g_dis(g_gen);
    }
    ss << "-";
    for (i = 0; i < 4; i++) {  // second group 4 bit for UUID
        ss << g_dis(g_gen);
    }
    ss << "-4";
    for (i = 0; i < 3; i++) {  // third group 3 bit for UUID
        ss << g_dis(g_gen);
    }
    ss << "-";
    ss << g_dis2(g_gen);
    for (i = 0; i < 3; i++) {  // fourth group 3 bit for UUID
        ss << g_dis(g_gen);
    }
    ss << "-";
    for (i = 0; i < 12; i++) {  // fifth group 12 bit for UUID
        ss << g_dis(g_gen);
    };
    return ss.str();
}

} // namespace Location
} // namespace OHOS
