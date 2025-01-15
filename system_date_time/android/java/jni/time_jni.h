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

#ifndef PLUGINS_TIME_ANDROID_JAVA_JNI_TIME_JNI_H
#define PLUGINS_TIME_ANDROID_JAVA_JNI_TIME_JNI_H

#include <jni.h>
#include <memory>
#include <mutex>

namespace OHOS::Time {
// using Json = nlohmann::json;
class TimeJni final {
public:
    TimeJni() = delete;
    ~TimeJni() = delete;
    static bool Register(void* env);

    // JAVA call C++
    static void NativeInit(JNIEnv* env, jobject jobj);

    // C++ call JAVA
    static int32_t GetTimeZone(std::string &timezoneId);
};
} // namespace OHOS::Time

#endif // PLUGINS_TIME_ANDROID_JAVA_JNI_TIME_JNI_H