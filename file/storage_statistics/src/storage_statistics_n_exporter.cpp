/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
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

#include "storage_statistics_n_exporter.h"

#include <cerrno>
#include <memory>
#include <string>
#include <sys/statvfs.h>

#include "n_async_work_callback.h"
#include "n_async_work_promise.h"
#include "stage_asset_manager.h"
#include "storage_statistics_platform.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleStorageStatistics {
using namespace OHOS::FileManagement::LibN;
namespace {
constexpr int32_t STORAGE_STATISTICS_UNKNOWN_ERROR = E_UNKNOWN_ERROR;
constexpr int32_t STORAGE_STATISTICS_INODE_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + E_GET_INODE;
constexpr int32_t STORAGE_STATISTICS_BUNDLE_INODE_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + E_GET_BUNDLE_INODES;

std::shared_ptr<AbilityRuntime::Platform::StageAssetManager> GetAssetManager()
{
    return AbilityRuntime::Platform::StageAssetManager::GetInstance();
}

int32_t GetCurrentBundleStatsCore(BundleStatsValue& stats)
{
    return GetCurrentBundleStatsByPlatform(stats);
}

int32_t GetCurrentBundleInodesCore(int64_t& inodes)
{
    return GetCurrentBundleInodesByPlatform(inodes);
}

std::string GetStatvfsPath()
{
    auto assetManager = GetAssetManager();
    if (assetManager == nullptr) {
        return "";
    }
    std::string filesDir = assetManager->GetFilesDir();
    return filesDir;
}

int32_t GetStatvfsInfo(struct statvfs& diskInfo)
{
    std::string path = GetStatvfsPath();
    if (path.empty()) {
        HILOGE("storage_statistics statvfs: path is empty");
        return EIO;
    }
    int ret = statvfs(path.c_str(), &diskInfo);
    if (ret != 0) {
        HILOGE("storage_statistics statvfs: failed, path=%{public}s errno=%{public}d",
            path.c_str(), errno == 0 ? EIO : errno);
        return errno == 0 ? EIO : errno;
    }
    return ERRNO_NOERR;
}

napi_value BuildBundleStatsNapiObject(napi_env env, const BundleStatsValue& stats)
{
    NVal bundleObject = NVal::CreateObject(env);
    bundleObject.AddProp("appSize", NVal::CreateInt64(env, stats.appSize).val_);
    bundleObject.AddProp("cacheSize", NVal::CreateInt64(env, stats.cacheSize).val_);
    bundleObject.AddProp("dataSize", NVal::CreateInt64(env, stats.dataSize).val_);
    return bundleObject.val_;
}

} // namespace

napi_value GetCurrentBundleStats(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto result = std::make_shared<BundleStatsValue>();
    auto cbExec = [result]() -> NError {
        int32_t err = GetCurrentBundleStatsCore(*result);
        if (err != ERRNO_NOERR) {
            HILOGE("storage_statistics getCurrentBundleStats: platform query failed, err=%{public}d", err);
            return NError(STORAGE_STATISTICS_UNKNOWN_ERROR);
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [result](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { env, BuildBundleStatsNapiObject(env, *result) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule("GetCurrentBundleStats", cbExec, cbComplete).val_;
    }

    NVal cb(env, funcArg[NARG_POS::FIRST]);
    return NAsyncWorkCallback(env, thisVar, cb, "GetCurrentBundleStats")
        .Schedule("GetCurrentBundleStats", cbExec, cbComplete)
        .val_;
}

napi_value GetTotalSize(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto resultSize = std::make_shared<int64_t>();
    auto cbExec = [resultSize]() -> NError {
        struct statvfs diskInfo;
        int32_t err = GetStatvfsInfo(diskInfo);
        if (err != ERRNO_NOERR) {
            return NError(STORAGE_STATISTICS_UNKNOWN_ERROR);
        }
        *resultSize = static_cast<int64_t>(diskInfo.f_frsize) * static_cast<int64_t>(diskInfo.f_blocks);
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [resultSize](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, *resultSize) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule("GetTotalSize", cbExec, cbComplete).val_;
    }

    NVal cb(env, funcArg[NARG_POS::FIRST]);
    return NAsyncWorkCallback(env, thisVar, cb, "GetTotalSize").Schedule("GetTotalSize", cbExec, cbComplete).val_;
}

napi_value GetTotalSizeSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    struct statvfs diskInfo;
    int32_t err = GetStatvfsInfo(diskInfo);
    if (err != ERRNO_NOERR) {
        NError(STORAGE_STATISTICS_UNKNOWN_ERROR).ThrowErr(env);
        return nullptr;
    }
    int64_t totalSize = static_cast<int64_t>(diskInfo.f_frsize) * static_cast<int64_t>(diskInfo.f_blocks);
    return NVal::CreateInt64(env, totalSize).val_;
}

napi_value GetFreeSize(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto resultSize = std::make_shared<int64_t>();
    auto cbExec = [resultSize]() -> NError {
        struct statvfs diskInfo;
        int32_t err = GetStatvfsInfo(diskInfo);
        if (err != ERRNO_NOERR) {
            return NError(STORAGE_STATISTICS_UNKNOWN_ERROR);
        }
        *resultSize = static_cast<int64_t>(diskInfo.f_frsize) * static_cast<int64_t>(diskInfo.f_bavail);
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [resultSize](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, *resultSize) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule("GetFreeSize", cbExec, cbComplete).val_;
    }

    NVal cb(env, funcArg[NARG_POS::FIRST]);
    return NAsyncWorkCallback(env, thisVar, cb, "GetFreeSize").Schedule("GetFreeSize", cbExec, cbComplete).val_;
}

napi_value GetFreeSizeSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    struct statvfs diskInfo;
    int32_t err = GetStatvfsInfo(diskInfo);
    if (err != ERRNO_NOERR) {
        NError(STORAGE_STATISTICS_UNKNOWN_ERROR).ThrowErr(env);
        return nullptr;
    }
    int64_t freeSize = static_cast<int64_t>(diskInfo.f_frsize) * static_cast<int64_t>(diskInfo.f_bavail);
    return NVal::CreateInt64(env, freeSize).val_;
}

napi_value GetTotalInodes(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto resultInodes = std::make_shared<int64_t>();
    auto cbExec = [resultInodes]() -> NError {
        struct statvfs diskInfo;
        int32_t err = GetStatvfsInfo(diskInfo);
        if (err != ERRNO_NOERR) {
            return NError(STORAGE_STATISTICS_INODE_ERROR);
        }
        *resultInodes = static_cast<int64_t>(diskInfo.f_files);
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [resultInodes](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, *resultInodes) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule("GetTotalInodes", cbExec, cbComplete).val_;
    }

    NVal cb(env, funcArg[NARG_POS::FIRST]);
    return NAsyncWorkCallback(env, thisVar, cb, "GetTotalInodes").Schedule("GetTotalInodes", cbExec, cbComplete).val_;
}

napi_value GetFreeInodes(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto resultInodes = std::make_shared<int64_t>();
    auto cbExec = [resultInodes]() -> NError {
        struct statvfs diskInfo;
        int32_t err = GetStatvfsInfo(diskInfo);
        if (err != ERRNO_NOERR) {
            return NError(STORAGE_STATISTICS_INODE_ERROR);
        }
        *resultInodes = static_cast<int64_t>(diskInfo.f_ffree);
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [resultInodes](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, *resultInodes) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule("GetFreeInodes", cbExec, cbComplete).val_;
    }

    NVal cb(env, funcArg[NARG_POS::FIRST]);
    return NAsyncWorkCallback(env, thisVar, cb, "GetFreeInodes").Schedule("GetFreeInodes", cbExec, cbComplete).val_;
}

napi_value GetCurrentBundleInodes(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto resultInodes = std::make_shared<int64_t>();
    auto cbExec = [resultInodes]() -> NError {
        int32_t err = GetCurrentBundleInodesCore(*resultInodes);
        if (err != ERRNO_NOERR) {
            HILOGE("storage_statistics getCurrentBundleInodes: platform query failed, err=%{public}d", err);
            return NError(STORAGE_STATISTICS_BUNDLE_INODE_ERROR);
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [resultInodes](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, *resultInodes) };
    };

    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule("GetCurrentBundleInodes", cbExec, cbComplete).val_;
    }

    NVal cb(env, funcArg[NARG_POS::FIRST]);
    return NAsyncWorkCallback(env, thisVar, cb, "GetCurrentBundleInodes")
        .Schedule("GetCurrentBundleInodes", cbExec, cbComplete)
        .val_;
}
} // namespace ModuleStorageStatistics
} // namespace FileManagement
} // namespace OHOS
