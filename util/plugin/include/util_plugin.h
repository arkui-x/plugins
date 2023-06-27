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
#ifndef OHOS_UTIL_TEXTENCODER_H
#define OHOS_UTIL_TEXTENCODER_H

#include <string>

#include "plugins/util/plugin/util.h"

namespace OHOS::Util {
class UtilPlugin {
public:
    UtilPlugin() = default;
    virtual ~UtilPlugin() = default;
    static std::string EncodeIntoChinese(std::string input, std::string encoding);
    static std::string Decode(std::string input, std::string encoding);
private:
    static std::unique_ptr<Plugin::Util> plugin;
};
} // namespace OHOS::Util
#endif
