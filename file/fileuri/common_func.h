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

#ifndef PLUGINS_FILE_FILEURI_COMMON_FUNC_H
#define PLUGINS_FILE_FILEURI_COMMON_FUNC_H

#include <string>

namespace OHOS {
namespace AppFileService {
class CommonFunc {
public:
    static std::string GetSelfBundleName();
    static std::string GetUriFromPath(const std::string& path);
    static bool EndsWith(const std::string& str, const std::string& suffix);
};
} // namespace AppFileService
} // namespace OHOS
#endif // PLUGINS_FILE_FILEURI_COMMON_FUNC_H
