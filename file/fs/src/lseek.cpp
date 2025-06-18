/*
 * Copyright (c) 2024 -2025 Huawei Device Co., Ltd.
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
#include "lseek.h"

#include <cstring>
#include <tuple>
#include <unistd.h>

#include "class_file/file_entity.h"
#include "class_file/file_n_exporter.h"
#include "common_func.h"
#include "filemgmt_libhilog.h"
#include "fs_utils.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace std;
using namespace LibN;

static tuple<bool, int, int, int> GetLseekArg(napi_env env, const NFuncArg &funcArg)
{
    auto [resGetFirstArg, fd] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32();
    if (!resGetFirstArg) {
        HILOGE("Invalid fd");
        NError(EINVAL).ThrowErr(env);
        return { false, -1, -1, -1 };
    }

    auto [resGetSecondArg, offset] = NVal(env, funcArg[NARG_POS::SECOND]).ToInt64();
    if (!resGetSecondArg) {
        HILOGE("Invalid offset");
        NError(EINVAL).ThrowErr(env);
        return { false, -1, -1, -1 };
    }

    SeekPos whence = SeekPos::START;
    if (funcArg.GetArgc() == NARG_CNT::THREE) {
        auto [resGetThirdArg, pos] = NVal(env, funcArg[NARG_POS::THIRD]).ToInt32(SeekPos::START);
        if (!resGetThirdArg) {
            HILOGE("Invalid whence");
            NError(EINVAL).ThrowErr(env);
            return { false, -1, -1, -1 };
        }
        if (pos < SeekPos::START || pos > SeekPos::END) {
            HILOGE("Invalid whence value");
            NError(EINVAL).ThrowErr(env);
            return { false, -1, -1, -1 };
        }
        whence = static_cast<SeekPos>(pos);
    }

    return { true, fd, offset, whence };
}

napi_value Lseek::Sync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO, NARG_CNT::THREE)) {
        HILOGE("Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [resGetLseekArg, fd, offset, whence] = GetLseekArg(env, funcArg);
    if (!resGetLseekArg) {
        return nullptr;
    }

    int ret = lseek(fd, offset, whence);
    if (ret < 0) {
        HILOGE("Failed to lseek error %{public}d", ret);
#ifdef IOS_PLATFORM
        NError(EINVAL).ThrowErr(env);
#else
        NError(errno).ThrowErr(env);
#endif
        return nullptr;
    }

    return NVal::CreateInt64(env, ret).val_;
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS