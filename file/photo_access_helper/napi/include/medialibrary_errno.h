/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_MEDIALIBRARY_ERRNO_H
#define OHOS_MEDIALIBRARY_ERRNO_H

#include <errno.h>
#include <stdint.h>

namespace OHOS {
namespace Media {
#define MEDIA_LIBRARY_ERR(offset, errCode) (-((offset) + (errCode)))

// common error code
// linux standard ERROR { 0, 200 }
constexpr int32_t E_OK = 0;
constexpr int32_t E_SUCCESS = 0;
#ifdef MEDIALIBRARY_MTP_ENABLE
constexpr int32_t MTP_SUCCESS = 0;
#endif
constexpr int32_t E_ERR = -1;
constexpr int32_t E_PERMISSION_DENIED = -EACCES;
constexpr int32_t E_NO_SUCH_FILE      = -ENOENT;
constexpr int32_t E_FILE_EXIST        = -EEXIST;
constexpr int32_t E_NO_MEMORY         = -ENOMEM;
constexpr int32_t E_NO_SPACE          = -ENOSPC;
constexpr int32_t E_CHECK_SYSTEMAPP_FAIL = 202;

// medialibary inner common err { 200, 1999 }
constexpr int32_t E_COMMON_OFFSET = 200;
constexpr int32_t E_COMMON_START =          MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 0);
constexpr int32_t E_FAIL =                  MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 0);
constexpr int32_t E_FILE_OPER_FAIL =        MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 1);
constexpr int32_t E_HAS_FS_ERROR =          MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 3);
constexpr int32_t E_CHECK_DIR_FAIL =        MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 4);
constexpr int32_t E_MODIFY_DATA_FAIL =      MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 5);
constexpr int32_t E_INVALID_VALUES =        MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 6);
constexpr int32_t E_INVALID_URI =           MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 7);
constexpr int32_t E_INVALID_FILEID =        MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 8);
constexpr int32_t E_INVALID_PATH =          MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 9);
constexpr int32_t E_VIOLATION_PARAMETERS =  MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 10);
constexpr int32_t E_RENAME_DIR_FAIL  =      MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 11);
constexpr int32_t E_GET_VALUEBUCKET_FAIL =  MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 12);
constexpr int32_t E_GET_ASSET_FAIL =        MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 13);
constexpr int32_t E_GET_HASH_FAIL =         MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 14);
constexpr int32_t E_GET_CLIENTBUNDLE_FAIL = MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 15);
constexpr int32_t E_EXIST_IN_DB =           MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 16);
constexpr int32_t E_INVALID_MODE =          MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 17);
constexpr int32_t E_INVALID_BUNDLENAME =    MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 18);
constexpr int32_t E_RENAME_FILE_FAIL =      MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 19);
constexpr int32_t E_DB_FAIL =               MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 20);
constexpr int32_t E_DELETE_DENIED =         MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 21);
constexpr int32_t E_HAS_DB_ERROR =          MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 22);
constexpr int32_t E_INVALID_ARGUMENTS =     MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 23);
constexpr int32_t E_SQL_CHECK_FAIL =        MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 24);
constexpr int32_t E_IS_PENDING_ERROR =      MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 25);
constexpr int32_t E_NEED_FURTHER_CHECK =    MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 26);
constexpr int32_t E_CHECK_NATIVE_SA_FAIL =  MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 27);
constexpr int32_t E_INVALID_TIMESTAMP =     MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 28);
constexpr int32_t E_GRANT_URI_PERM_FAIL =   MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 29);
constexpr int32_t E_IS_IN_COMMIT =          MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 30);
constexpr int32_t E_IS_IN_REVERT =          MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 31);
constexpr int32_t E_GET_PRAMS_FAIL =        MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 32);
constexpr int32_t E_COMMON_END =            MEDIA_LIBRARY_ERR(E_COMMON_OFFSET, 1799);
} // namespace Media
} // namespace OHOS
#endif // OHOS_MEDIALIBRARY_ERRNO_H
