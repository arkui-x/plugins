/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_SRC_MOD_FILEIO_CLASS_STAT_N_EXPORTER_H
#define INTERFACES_KITS_JS_SRC_MOD_FILEIO_CLASS_STAT_N_EXPORTER_H

#include "common/napi/n_exporter.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
class StatNExporter final : public NExporter {
public:
    inline static const std::string className_ = "Stat";

    bool Export() override;
    std::string GetClassName() override;

    static napi_value Constructor(napi_env env, napi_callback_info cbinfo);

    static napi_value IsBlockDevice(napi_env env, napi_callback_info cbinfo);
    static napi_value IsCharacterDevice(napi_env env, napi_callback_info cbinfo);
    static napi_value IsDirectory(napi_env env, napi_callback_info cbinfo);
    static napi_value IsFIFO(napi_env env, napi_callback_info cbinfo);
    static napi_value IsFile(napi_env env, napi_callback_info cbinfo);
    static napi_value IsSocket(napi_env env, napi_callback_info cbinfo);
    static napi_value IsSymbolicLink(napi_env env, napi_callback_info cbinfo);

    static napi_value GetDev(napi_env env, napi_callback_info cbinfo);
    static napi_value GetIno(napi_env env, napi_callback_info cbinfo);
    static napi_value GetMode(napi_env env, napi_callback_info cbinfo);
    static napi_value GetNlink(napi_env env, napi_callback_info cbinfo);
    static napi_value GetUid(napi_env env, napi_callback_info cbinfo);
    static napi_value GetGid(napi_env env, napi_callback_info cbinfo);
    static napi_value GetRdev(napi_env env, napi_callback_info cbinfo);
    static napi_value GetSize(napi_env env, napi_callback_info cbinfo);
    static napi_value GetBlksize(napi_env env, napi_callback_info cbinfo);
    static napi_value GetBlocks(napi_env env, napi_callback_info cbinfo);
    static napi_value GetAtime(napi_env env, napi_callback_info cbinfo);
    static napi_value GetMtime(napi_env env, napi_callback_info cbinfo);
    static napi_value GetCtime(napi_env env, napi_callback_info cbinfo);

    StatNExporter(napi_env env, napi_value exports);
    ~StatNExporter() override;
};
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS
#endif // INTERFACES_KITS_JS_SRC_MOD_FILEIO_CLASS_STAT_N_EXPORTER_H