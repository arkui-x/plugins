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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_COPY_FILE_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_COPY_FILE_H

#include "common_func.h"
#include "filemgmt_libn.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
class CopyFile final {
public:
    static napi_value Async(napi_env env, napi_callback_info info);
    static napi_value Sync(napi_env env, napi_callback_info info);
};

class Para {
public:
    FileInfo src_;
    FileInfo dest_;

    Para(FileInfo src, FileInfo dest) : src_(move(src)), dest_(move(dest)) {};
};
constexpr size_t MAX_SIZE = 0x7ffff000;
#ifdef IOS_PLATFORM
constexpr size_t COPY_BLOCK_SIZE = 4096;
#endif
const string PROCEDURE_COPYFILE_NAME = "FileIOCopyFile";
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_COPY_FILE_H