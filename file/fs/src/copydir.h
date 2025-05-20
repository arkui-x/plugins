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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_COPYDIR_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_COPYDIR_H

#include <string>

#include "filemgmt_libn.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {

constexpr int COPYMODE_MIN = 0;
constexpr int COPYMODE_MAX = 1;
constexpr int COPYDIR_DEFAULT_PERM = 0770;

constexpr int DISMATCH = 0;
constexpr int MATCH = 1;

enum ModeOfCopyDir {
    DIRMODE_FILE_COPY_THROW_ERR = 0,
    DIRMODE_FILE_COPY_REPLACE
};

class CopyDir final {
public:
    static napi_value Sync(napi_env env, napi_callback_info info);
    static napi_value Async(napi_env env, napi_callback_info info);
};

struct ConflictFiles {
    std::string srcFiles;
    std::string destFiles;
    ConflictFiles(const std::string& src, const std::string& dest) : srcFiles(src), destFiles(dest) {}
    ~ConflictFiles() = default;
};

const std::string PROCEDURE_COPYDIR_NAME = "FileIOCopyDir";
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_COPYDIR_H