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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_APPEXECFWK_ERRORS_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_APPEXECFWK_ERRORS_H

#include "errors.h"

namespace OHOS {
enum {
    APPEXECFWK_MODULE_COMMON = 0x00,
    APPEXECFWK_MODULE_APPMGR = 0x01,
    APPEXECFWK_MODULE_BUNDLEMGR = 0x02,
    // Reserved 0x03 ~ 0x0f for new modules, Event related modules start from 0x10
    APPEXECFWK_MODULE_EVENTMGR = 0x10,
    APPEXECFWK_MODULE_HIDUMP = 0x11
};

// Error code for BundleMgr
constexpr ErrCode APPEXECFWK_BUNDLEMGR_ERR_OFFSET = ErrCodeOffset(SUBSYS_APPEXECFWK, APPEXECFWK_MODULE_BUNDLEMGR);
enum {
    // zlib errcode
    ERR_ZLIB_SRC_FILE_DISABLED = 8521240,
    ERR_ZLIB_DEST_FILE_DISABLED = 8521241,
    ERR_ZLIB_SERVICE_DISABLED = 8521242,
    ERR_ZLIB_SRC_FILE_FORMAT_ERROR = 8521243,
};
} // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_APPEXECFWK_ERRORS_H
