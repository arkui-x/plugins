/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_MOVEDIR_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_MOVEDIR_H

#include <string>

#include "filemgmt_libn.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {

constexpr int DIRMODE_MIN = 0;
constexpr int DIRMODE_MAX = 3;

constexpr int FILE_DISMATCH = 0;
constexpr int FILE_MATCH = 1;
constexpr int MOVEDIR_DEFAULT_PERM = 0770;

enum ModeOfMoveDir {
    DIRMODE_DIRECTORY_THROW_ERR = 0,
    DIRMODE_FILE_THROW_ERR,
    DIRMODE_FILE_REPLACE,
    DIRMODE_DIRECTORY_REPLACE
};

class MoveDir final {
public:
    static napi_value Sync(napi_env env, napi_callback_info info);
    static napi_value Async(napi_env env, napi_callback_info info);
};

struct ErrFiles {
    std::string srcFiles;
    std::string destFiles;
    ErrFiles(const std::string& src, const std::string& dest) : srcFiles(src), destFiles(dest) {}
    ~ErrFiles() = default;
};

const std::string PROCEDURE_MOVEDIR_NAME = "FileIOMoveDir";
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_MOVEDIR_H