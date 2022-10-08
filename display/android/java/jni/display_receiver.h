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

#ifndef PLUGINS_DISPLAY_ANDROID_JAVA_JNI_DISPLAY_RECEIVER_H
#define PLUGINS_DISPLAY_ANDROID_JAVA_JNI_DISPLAY_RECEIVER_H

#include "plugins/display/display.h"

namespace OHOS::Plugin {
class DisplayReceiver {
public:
    static void ReceiveFun(DisplayInfo displayInfo, OH_Plugin_AsyncCallbackInfo* ptr);
};
} // namespace OHOS::Plugin

#endif // PLUGINS_DISPLAY_ANDROID_JAVA_JNI_DISPLAY_RECEIVER_H
