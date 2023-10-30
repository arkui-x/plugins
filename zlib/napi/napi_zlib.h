/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_APPEXECFWK_LIBZIP_ZLIB_H
#define OHOS_APPEXECFWK_LIBZIP_ZLIB_H
#include <string>

#include "napi/native_api.h"
#include "napi_arg.h"
#include "napi_zlib_common.h"
#include "zip_utils.h"

namespace OHOS {
namespace AppExecFwk {
namespace LIBZIP {

/**
 * @brief FlushType data initialization.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value FlushTypeInit(napi_env env, napi_value exports);
/**
 * @brief CompressLevel data initialization.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value CompressLevelInit(napi_env env, napi_value exports);

/**
 * @brief CompressStrategy data initialization.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value CompressStrategyInit(napi_env env, napi_value exports);

/**
 * @brief MemLevel data initialization.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value MemLevelInit(napi_env env, napi_value exports);

/**
 * @brief zlib NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value ZlibInit(napi_env env, napi_value exports);

bool InitParam(CallZipUnzipParam& param, napi_env env, NapiArg& args, bool isZipFile);

napi_value CompressFile(napi_env env, napi_callback_info info);
napi_value DecompressFile(napi_env env, napi_callback_info info);

} // namespace LIBZIP
} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_APPEXECFWK_LIBZIP_ZLIB_H