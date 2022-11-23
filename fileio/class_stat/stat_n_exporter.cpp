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

#include "stat_n_exporter.h"
#include "common/napi/n_async/n_async_work_callback.h"
#include "common/napi/n_async/n_async_work_promise.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <sstream>

#include "securec.h"

#include "log.h"
#include "common/napi/n_class.h"
#include "common/napi/n_func_arg.h"
#include "../uni_error.h"
#include "stat_entity.h"

namespace OHOS {
namespace DistributedFS {
namespace ModuleFileIO {
using namespace std;

static napi_value CheckStatMode(napi_env env, napi_callback_info info, mode_t mode)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    bool check = (statEntity->stat_.st_mode & S_IFMT) == mode;
    return NVal::CreateBool(env, check).val_;
}

napi_value StatNExporter::IsBlockDevice(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFBLK);
}

napi_value StatNExporter::IsCharacterDevice(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFCHR);
}

napi_value StatNExporter::IsDirectory(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFDIR);
}

napi_value StatNExporter::IsFIFO(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFIFO);
}

napi_value StatNExporter::IsFile(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFREG);
}

napi_value StatNExporter::IsSocket(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFSOCK);
}

napi_value StatNExporter::IsSymbolicLink(napi_env env, napi_callback_info info)
{
    return CheckStatMode(env, info, S_IFLNK);
}

napi_value StatNExporter::GetDev(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_dev).val_;
}

napi_value StatNExporter::GetIno(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_ino).val_;
}

napi_value StatNExporter::GetMode(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_mode).val_;
}

napi_value StatNExporter::GetNlink(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_nlink).val_;
}

napi_value StatNExporter::GetUid(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_uid).val_;
}

napi_value StatNExporter::GetGid(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_gid).val_;
}

napi_value StatNExporter::GetRdev(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_rdev).val_;
}

napi_value StatNExporter::GetSize(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_size).val_;
}

napi_value StatNExporter::GetBlksize(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_blksize).val_;
}

napi_value StatNExporter::GetBlocks(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

    return NVal::CreateInt64(env, statEntity->stat_.st_blocks).val_;
}

napi_value StatNExporter::GetAtime(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }

#ifdef IOS_PLATFORM
    return NVal::CreateInt64(env, statEntity->stat_.st_atime).val_;
#else
    return NVal::CreateInt64(env, statEntity->stat_.st_atim.tv_sec).val_;
#endif
}

napi_value StatNExporter::GetMtime(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }
#ifdef IOS_PLATFORM
    return NVal::CreateInt64(env, statEntity->stat_.st_mtime).val_;
#else
    return NVal::CreateInt64(env, statEntity->stat_.st_mtim.tv_sec).val_;
#endif
}

napi_value StatNExporter::GetCtime(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto statEntity = NClass::GetEntityOf<StatEntity>(env, funcArg.GetThisVar());
    if (!statEntity) {
        return nullptr;
    }
#ifdef IOS_PLATFORM
    return NVal::CreateInt64(env, statEntity->stat_.st_ctime).val_;
#else
    return NVal::CreateInt64(env, statEntity->stat_.st_ctim.tv_sec).val_;
#endif
}

napi_value StatNExporter::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    unique_ptr<StatEntity> statEntity = make_unique<StatEntity>();
    if (!NClass::SetEntityFor<StatEntity>(env, funcArg.GetThisVar(), move(statEntity))) {
        stringstream ss;
        ss << "INNER BUG. Failed to wrap entity for obj stat";
        UniError(EIO).ThrowErr(env, ss.str());
        return nullptr;
    }
    return funcArg.GetThisVar();
}

bool StatNExporter::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("isBlockDevice", IsBlockDevice),
        NVal::DeclareNapiFunction("isCharacterDevice", IsCharacterDevice),
        NVal::DeclareNapiFunction("isDirectory", IsDirectory),
        NVal::DeclareNapiFunction("isFIFO", IsFIFO),
        NVal::DeclareNapiFunction("isFile", IsFile),
        NVal::DeclareNapiFunction("isSocket", IsSocket),
        NVal::DeclareNapiFunction("isSymbolicLink", IsSymbolicLink),

        NVal::DeclareNapiGetter("dev", GetDev),
        NVal::DeclareNapiGetter("ino", GetIno),
        NVal::DeclareNapiGetter("mode", GetMode),
        NVal::DeclareNapiGetter("nlink", GetNlink),
        NVal::DeclareNapiGetter("uid", GetUid),
        NVal::DeclareNapiGetter("gid", GetGid),
        NVal::DeclareNapiGetter("rdev", GetRdev),
        NVal::DeclareNapiGetter("size", GetSize),
        NVal::DeclareNapiGetter("blksize", GetBlksize),
        NVal::DeclareNapiGetter("blocks", GetBlocks),
        NVal::DeclareNapiGetter("atime", GetAtime),
        NVal::DeclareNapiGetter("mtime", GetMtime),
        NVal::DeclareNapiGetter("ctime", GetCtime),
    };

    string className = GetClassName();
    bool succ = false;
    napi_value classValue = nullptr;
    tie(succ, classValue) = NClass::DefineClass(exports_.env_, className, StatNExporter::Constructor, std::move(props));
    if (!succ) {
        UniError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to define class");
        return false;
    }
    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        UniError(EIO).ThrowErr(exports_.env_, "INNER BUG. Failed to save class");
        return false;
    }

    return exports_.AddProp(className, classValue);
}

string StatNExporter::GetClassName()
{
    return StatNExporter::className_;
}

StatNExporter::StatNExporter(napi_env env, napi_value exports) : NExporter(env, exports) {}

StatNExporter::~StatNExporter() {}
} // namespace ModuleFileIO
} // namespace DistributedFS
} // namespace OHOS