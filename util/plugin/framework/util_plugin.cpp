/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "plugins/util/plugin/util.h"
#include "../include/util_plugin.h"

namespace OHOS::Util {

std::unique_ptr<Plugin::Util> UtilPlugin::plugin = Plugin::Util::Create();

std::string UtilPlugin::EncodeIntoChinese(std::string input, std::string encoding)
{
    if (!plugin) {
        return "";
    }
    return plugin->EncodeIntoChinese(input, encoding);
}

std::string UtilPlugin::Decode(std::string input, std::string encoding)
{
    if (!plugin) {
        return "";
    }
    return plugin->Decode(input, encoding);
}
} // namespace OHOS::Util
