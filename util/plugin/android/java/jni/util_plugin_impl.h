/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_UTIL_PLUGIN_ANDROID_JAVA_JNI_UTIL_PLUGIN_IMPL_H
#define PLUGINS_UTIL_PLUGIN_ANDROID_JAVA_JNI_UTIL_PLUGIN_IMPL_H

#include <memory>

#include "plugins/util/plugin/util.h"

namespace OHOS::Plugin {
class UtilPluginImpl final : public Util {
public:
    UtilPluginImpl() = default;
    ~UtilPluginImpl() override = default;
    std::string EncodeIntoChinese(std::string input, std::string encoding) override;
    std::string Decode(std::string input, std::string encoding) override;
};
} // namespace OHOS::Plugin
#endif // PLUGINS_UTIL_PLUGIN_ANDROID_JAVA_JNI_UTIL_PLUGIN_IMPL_H
