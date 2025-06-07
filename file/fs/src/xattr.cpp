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

#include "xattr.h"

#include <memory>
#include <optional>
#include <sys/xattr.h>

#include "file_utils.h"
#include "filemgmt_libhilog.h"
#include "n_error.h"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace OHOS::FileManagement::LibN;
using namespace std;
constexpr size_t MAX_XATTR_SIZE = 4096;

static bool IsIllegalXattr(const char *key, const char *value)
{
    if (key == nullptr || value == nullptr) {
        return true;
    }
    bool isIllegalKey = strnlen(key, MAX_XATTR_SIZE + 1) > MAX_XATTR_SIZE;
    if (isIllegalKey) {
        HILOGE("key is too long");
    }
    bool isIllegalValue = strnlen(value, MAX_XATTR_SIZE + 1) > MAX_XATTR_SIZE;
    if (isIllegalValue) {
        HILOGE("value is too long");
    }
    return isIllegalKey || isIllegalValue;
}

static napi_value CheckAndThrow(napi_env env, int errCode)
{
    NError(errCode).ThrowErr(env);
    return nullptr;
}

static int32_t GetXattrCore(const char *path,
                            const char *key,
                            std::shared_ptr<string> result)
{
#ifdef IOS_PLATFORM
    ssize_t xAttrSize = getxattr(path, key, nullptr, 0, 0, 0);
#else
    ssize_t xAttrSize = getxattr(path, key, nullptr, 0);
#endif
    if (xAttrSize == -1) {
        HILOGW("Failed to get xattr value, errno is %{public}d", errno);
        *result = "";
        return ERRNO_NOERR;
    }
    if (xAttrSize == 0) {
        HILOGW("Xattr value is not exist");
        *result = "";
        return ERRNO_NOERR;
    }
    auto xattrValue = CreateUniquePtr<char[]>(static_cast<long>(xAttrSize) + 1);
#ifdef IOS_PLATFORM
    xAttrSize = getxattr(path, key, xattrValue.get(), static_cast<size_t>(xAttrSize), 0, 0);
#else
    xAttrSize = getxattr(path, key, xattrValue.get(), static_cast<size_t>(xAttrSize));
#endif
    if (xAttrSize == -1) {
        HILOGE("Failed to get xattr value, errno is %{public}d", errno);
        return errno;
    }
    xattrValue[xAttrSize] = '\0';
    *result = std::string(xattrValue.get());
    return ERRNO_NOERR;
}

napi_value Xattr::SetSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<int>(NARG_CNT::THREE))) {
        HILOGE("Number of arguments unmatched");
        return CheckAndThrow(env, EINVAL);
    }
    bool isSuccess = false;
    std::unique_ptr<char[]> path;
    std::unique_ptr<char[]> key;
    std::unique_ptr<char[]> value;
    tie(isSuccess, path, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::FIRST)]).ToUTF8StringPath();
    if (!isSuccess) {
        HILOGE("Invalid path");
        return CheckAndThrow(env, EINVAL);
    }
    tie(isSuccess, key, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::SECOND)]).ToUTF8String();
    if (!isSuccess) {
        HILOGE("Invalid xattr key");
        return CheckAndThrow(env, EINVAL);
    }
    tie(isSuccess, value, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::THIRD)]).ToUTF8String();
    if (!isSuccess || IsIllegalXattr(key.get(), value.get())) {
        HILOGE("Invalid xattr value");
        return CheckAndThrow(env, EINVAL);
    }
#ifdef IOS_PLATFORM
    if (setxattr(path.get(), key.get(), value.get(), strnlen(value.get(), MAX_XATTR_SIZE), 0, 0) < 0) {
        HILOGE("setxattr fail, ios errno is %{public}d", errno);
        errno = ENOENT;
        return CheckAndThrow(env, errno);
    }
#else
    if (setxattr(path.get(), key.get(), value.get(), strnlen(value.get(), MAX_XATTR_SIZE), 0) < 0) {
        HILOGE("setxattr fail, errno is %{public}d", errno);
        return CheckAndThrow(env, errno);
    }
#endif
    return NVal::CreateUndefined(env).val_;
}

napi_value Xattr::GetSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<int>(NARG_CNT::TWO))) {
        HILOGE("Number of arguments unmatched");
        return CheckAndThrow(env, EINVAL);
    }

    bool isSuccess = false;
    std::unique_ptr<char[]> path;
    std::unique_ptr<char[]> key;
    tie(isSuccess, path, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::FIRST)]).ToUTF8StringPath();
    if (!isSuccess) {
        HILOGE("Invalid path");
        return CheckAndThrow(env, EINVAL);
    }
    tie(isSuccess, key, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::SECOND)]).ToUTF8String();
    if (!isSuccess) {
        HILOGE("Invalid xattr key");
        return CheckAndThrow(env, EINVAL);
    }
    auto result = make_shared<std::string>();
    int32_t ret = GetXattrCore(path.get(), key.get(), result);
    if (ret != ERRNO_NOERR) {
        HILOGE("Invalid getxattr");
        return CheckAndThrow(env, ret);
    }
    return NVal::CreateUTF8String(env, *result).val_;
}

napi_value Xattr::GetAsync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<int>(NARG_CNT::TWO))) {
        HILOGE("Number of arguments unmatched");
        return CheckAndThrow(env, EINVAL);
    }
    bool isSuccess = false;
    std::unique_ptr<char[]> path;
    std::unique_ptr<char[]> key;
    tie(isSuccess, path, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::FIRST)]).ToUTF8StringPath();
    if (!isSuccess) {
        HILOGE("Invalid path");
        return CheckAndThrow(env, EINVAL);
    }
    tie(isSuccess, key, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::SECOND)]).ToUTF8String();
    if (!isSuccess) {
        HILOGE("Invalid xattr key");
        return CheckAndThrow(env, EINVAL);
    }
    auto result = make_shared<std::string>();
    string pathString(path.get());
    string keyString(key.get());
    auto cbExec = [path = move(pathString), key = move(keyString), result]() -> NError {
        int ret = GetXattrCore(path.c_str(), key.c_str(), result);
        return NError(ret);
    };
    auto cbComplete = [result](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return {NVal::CreateUTF8String(env, *result)};
    };
    static const std::string procedureName = "GetXattr";
    NVal thisVar(env, funcArg.GetThisVar());
    return NAsyncWorkPromise(env, thisVar)
        .Schedule(procedureName, cbExec, cbComplete)
        .val_;
}

napi_value Xattr::SetAsync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<int>(NARG_CNT::THREE))) {
        HILOGE("Number of arguments unmatched");
        return CheckAndThrow(env, EINVAL);
    }
    bool isSuccess = false;
    std::unique_ptr<char[]> path;
    std::unique_ptr<char[]> key;
    std::unique_ptr<char[]> value;
    tie(isSuccess, path, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::FIRST)]).ToUTF8StringPath();
    if (!isSuccess) {
        HILOGE("Invalid path");
        return CheckAndThrow(env, EINVAL);
    }
    tie(isSuccess, key, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::SECOND)]).ToUTF8String();
    if (!isSuccess) {
        HILOGE("Invalid xattr key");
        return CheckAndThrow(env, EINVAL);
    }
    tie(isSuccess, value, std::ignore) = NVal(env, funcArg[static_cast<int>(NARG_POS::THIRD)]).ToUTF8String();
    if (!isSuccess || IsIllegalXattr(key.get(), value.get())) {
        HILOGE("Invalid xattr value");
        return CheckAndThrow(env, EINVAL);
    }
    string pathString(path.get());
    string keyString(key.get());
    string valueString(value.get());
    auto cbExec = [path = move(pathString), key = move(keyString), value = move(valueString)]() -> NError {
#ifdef IOS_PLATFORM
        if (setxattr(path.c_str(), key.c_str(), value.c_str(), strnlen(value.c_str(), MAX_XATTR_SIZE), 0, 0) < 0) {
            HILOGE("setxattr fail, errno is %{public}d", errno);
            errno = ENOENT;
            return NError(errno);
        }
#else
        if (setxattr(path.c_str(), key.c_str(), value.c_str(), strnlen(value.c_str(), MAX_XATTR_SIZE), 0) < 0) {
            HILOGE("setxattr fail, errno is %{public}d", errno);
            return NError(errno);
        }
#endif
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [](napi_env env, NError err) -> NVal {
        return err ? NVal{env, err.GetNapiErr(env)} : NVal::CreateUndefined(env);
    };
    static const std::string procedureName = "SetXattr";
    NVal thisVar(env, funcArg.GetThisVar());
    return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS