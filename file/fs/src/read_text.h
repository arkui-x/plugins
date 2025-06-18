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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_READ_TEXT_H
#define INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_READ_TEXT_H

#include "filemgmt_libn.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace OHOS::FileManagement::LibN;
class ReadText final {
public:
    static napi_value Async(napi_env env, napi_callback_info info);
    static napi_value Sync(napi_env env, napi_callback_info info);
};

struct AsyncReadTextArg {
    NRef _refReadBuf;
    std::string buffer;
    int64_t len = 0;

    explicit AsyncReadTextArg(NVal refReadBuf) : _refReadBuf(refReadBuf){};
    ~AsyncReadTextArg() = default;
};

const std::string PROCEDURE_READTEXT_NAME = "FileIOReadText";
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FS_PROPERTIES_READ_TEXT_H