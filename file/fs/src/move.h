/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_MOVE_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_MOVE_H

#include <string>

#include "filemgmt_libn.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
class Move final {
public:
    static napi_value Sync(napi_env env, napi_callback_info info);
    static napi_value Async(napi_env env, napi_callback_info info);
};
constexpr int MODE_FORCE_MOVE = 0;
constexpr int MODE_THROW_ERR = 1;
const std::string PROCEDURE_MOVE_NAME = "FileIOMove";
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif