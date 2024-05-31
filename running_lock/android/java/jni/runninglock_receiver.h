/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_RUNNINGLOCK_ANDROID_JAVA_JNI_RUNNINGLOCK_RECEIVE_H
#define PLUGINS_RUNNINGLOCK_ANDROID_JAVA_JNI_RUNNINGLOCK_RECEIVE_H

#include <string>

#include "plugins/running_lock/runninglock.h"

namespace OHOS {
namespace Plugin {
class RunningLockReceiver {
public:
    static void ReceiveCallBack(RunningLockAsyncCallbackInfo *ptr);
};
}  // namespace Plugin
}  // namespace OHOS

#endif  // PLUGINS_RUNNINGLOCK_ANDROID_JAVA_JNI_RUNNINGLOCK_RECEIVE_H