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

// medialibary directory and file type control { 2000, 2099 }
constexpr int32_t E_DIR_CTRL_OFFSET = 2000;
constexpr int32_t E_CHECK_EXTENSION_FAIL =                  MEDIA_LIBRARY_ERR(E_DIR_CTRL_OFFSET, 0);
constexpr int32_t E_DELETE_DIR_FAIL =                       MEDIA_LIBRARY_ERR(E_DIR_CTRL_OFFSET, 1);
constexpr int32_t E_CHECK_MEDIATYPE_FAIL =                  MEDIA_LIBRARY_ERR(E_DIR_CTRL_OFFSET, 2);
constexpr int32_t E_CHECK_ROOT_DIR_FAIL =                   MEDIA_LIBRARY_ERR(E_DIR_CTRL_OFFSET, 3);
constexpr int32_t E_CHECK_MEDIATYPE_MATCH_EXTENSION_FAIL =  MEDIA_LIBRARY_ERR(E_DIR_CTRL_OFFSET, 4);
constexpr int32_t E_FILE_NAME_INVALID =                     MEDIA_LIBRARY_ERR(E_DIR_CTRL_OFFSET, 5);
constexpr int32_t E_ALBUM_OPER_ERR =                        MEDIA_LIBRARY_ERR(E_DIR_CTRL_OFFSET, 6);
constexpr int32_t E_DIR_OPER_ERR =                          MEDIA_LIBRARY_ERR(E_DIR_CTRL_OFFSET, 7);
constexpr int32_t E_SAME_PATH =                             MEDIA_LIBRARY_ERR(E_DIR_CTRL_OFFSET, 8);
constexpr int32_t E_DIR_CHECK_DIR_FAIL =                    MEDIA_LIBRARY_ERR(E_DIR_CTRL_OFFSET, 9);

// medialibary recycle, trash { 2100, 2199 }
constexpr int32_t E_TRASH_OFFSET = 2100;
constexpr int32_t E_RECYCLE_FILE_IS_NULL = MEDIA_LIBRARY_ERR(E_TRASH_OFFSET, 0);
constexpr int32_t E_IS_RECYCLED =          MEDIA_LIBRARY_ERR(E_TRASH_OFFSET, 1);
constexpr int32_t E_RECYCLE_DIR_FAIL =     MEDIA_LIBRARY_ERR(E_TRASH_OFFSET, 2);
constexpr int32_t E_CREATE_TRASHDIR_FAIL = MEDIA_LIBRARY_ERR(E_TRASH_OFFSET, 3);
constexpr int32_t E_MAKE_HASHNAME_FAIL =   MEDIA_LIBRARY_ERR(E_TRASH_OFFSET, 4);
constexpr int32_t E_GET_EXTENSION_FAIL =   MEDIA_LIBRARY_ERR(E_TRASH_OFFSET, 5);


// medialibary distributed { 2200, 2299 }
constexpr int32_t E_DISTRIBUTED_OFFSET = 2200;
constexpr int32_t E_DEVICE_OPER_ERR = MEDIA_LIBRARY_ERR(E_DISTRIBUTED_OFFSET, 0);

// medialibary thumbanail { 2300, 2399 }
constexpr int32_t E_THUMBNAIL_OFFSET = 2300;
constexpr int32_t E_THUMBNAIL_CONNECT_TIMEOUT = MEDIA_LIBRARY_ERR(E_THUMBNAIL_OFFSET, 0);
constexpr int32_t E_THUMBNAIL_HAS_OPENED_FAIL = MEDIA_LIBRARY_ERR(E_THUMBNAIL_OFFSET, 1);
constexpr int32_t E_THUMBNAIL_LOCAL_CREATE_FAIL = MEDIA_LIBRARY_ERR(E_THUMBNAIL_OFFSET, 2);
constexpr int32_t E_THUMBNAIL_REMOTE_CREATE_FAIL = MEDIA_LIBRARY_ERR(E_THUMBNAIL_OFFSET, 3);
constexpr int32_t E_THUMBNAIL_SERVICE_NULLPTR = MEDIA_LIBRARY_ERR(E_THUMBNAIL_OFFSET, 4);
constexpr int32_t E_THUMBNAIL_INVALID_SIZE = MEDIA_LIBRARY_ERR(E_THUMBNAIL_OFFSET, 5);
constexpr int32_t E_THUMBNAIL_UNKNOWN = MEDIA_LIBRARY_ERR(E_THUMBNAIL_OFFSET, 6);

// medialibary scanner { 2400, 2499 }
constexpr int32_t E_SCANNER_OFFSET = 2400;
constexpr int32_t E_FILE_HIDDEN = MEDIA_LIBRARY_ERR(E_SCANNER_OFFSET, 0);
constexpr int32_t E_DIR_HIDDEN = MEDIA_LIBRARY_ERR(E_SCANNER_OFFSET, 1);
constexpr int32_t E_RDB = MEDIA_LIBRARY_ERR(E_SCANNER_OFFSET, 2);
constexpr int32_t E_DATA = MEDIA_LIBRARY_ERR(E_SCANNER_OFFSET, 3);
constexpr int32_t E_SYSCALL = MEDIA_LIBRARY_ERR(E_SCANNER_OFFSET, 4);
constexpr int32_t E_SCANNED = MEDIA_LIBRARY_ERR(E_SCANNER_OFFSET, 5);
constexpr int32_t E_AVMETADATA = MEDIA_LIBRARY_ERR(E_SCANNER_OFFSET, 6);
constexpr int32_t E_IMAGE = MEDIA_LIBRARY_ERR(E_SCANNER_OFFSET, 7);
constexpr int32_t E_NO_RESULT = MEDIA_LIBRARY_ERR(E_SCANNER_OFFSET, 8);
constexpr int32_t E_STOP = MEDIA_LIBRARY_ERR(E_SCANNER_OFFSET, 9);
constexpr int32_t E_IS_PENDING = MEDIA_LIBRARY_ERR(E_SCANNER_OFFSET, 10);

// media file extension module error code { 3000, 3099 }
constexpr int32_t E_MEDIA_FILE_OFFSET = 3000;
constexpr int32_t E_URI_INVALID =               MEDIA_LIBRARY_ERR(E_MEDIA_FILE_OFFSET, 0);
constexpr int32_t E_DISTIBUTED_URI_NO_SUPPORT = MEDIA_LIBRARY_ERR(E_MEDIA_FILE_OFFSET, 1);
constexpr int32_t E_URI_IS_NOT_ALBUM =          MEDIA_LIBRARY_ERR(E_MEDIA_FILE_OFFSET, 2);
constexpr int32_t E_URI_IS_NOT_FILE =           MEDIA_LIBRARY_ERR(E_MEDIA_FILE_OFFSET, 3);
constexpr int32_t E_TWO_URI_ARE_THE_SAME =      MEDIA_LIBRARY_ERR(E_MEDIA_FILE_OFFSET, 4);
constexpr int32_t E_OPENFILE_INVALID_FLAG =     MEDIA_LIBRARY_ERR(E_MEDIA_FILE_OFFSET, 5);
constexpr int32_t E_INVALID_DISPLAY_NAME =      MEDIA_LIBRARY_ERR(E_MEDIA_FILE_OFFSET, 6);
constexpr int32_t E_DENIED_MOVE =               MEDIA_LIBRARY_ERR(E_MEDIA_FILE_OFFSET, 7);
constexpr int32_t E_UPDATE_DB_FAIL =            MEDIA_LIBRARY_ERR(E_MEDIA_FILE_OFFSET, 8);
constexpr int32_t E_DENIED_RENAME =             MEDIA_LIBRARY_ERR(E_MEDIA_FILE_OFFSET, 9);

// media smart album module error code { 4000, 4099 }
constexpr int32_t E_MEDIA_SMART_OFFSET = 4000;
constexpr int32_t E_PARENT_SMARTALBUM_IS_NOT_EXISTED = MEDIA_LIBRARY_ERR(E_MEDIA_SMART_OFFSET, 0);
constexpr int32_t E_PARENT_SMARTALBUM_CAN_NOT_DELETE = MEDIA_LIBRARY_ERR(E_MEDIA_SMART_OFFSET, 1);
constexpr int32_t E_DELETE_SMARTALBUM_MAP_FAIL =       MEDIA_LIBRARY_ERR(E_MEDIA_SMART_OFFSET, 2);
constexpr int32_t E_SMARTALBUM_IS_NOT_EXISTED =        MEDIA_LIBRARY_ERR(E_MEDIA_SMART_OFFSET, 3);
constexpr int32_t E_CHILD_CAN_NOT_ADD_SMARTALBUM =     MEDIA_LIBRARY_ERR(E_MEDIA_SMART_OFFSET, 4);
constexpr int32_t E_TRASHALBUM_CAN_NOT_DELETE =        MEDIA_LIBRARY_ERR(E_MEDIA_SMART_OFFSET, 5);
constexpr int32_t E_FAVORITEALBUM_CAN_NOT_DELETE =     MEDIA_LIBRARY_ERR(E_MEDIA_SMART_OFFSET, 6);
constexpr int32_t E_PARENT_CAN_NOT_ADDASSETS =         MEDIA_LIBRARY_ERR(E_MEDIA_SMART_OFFSET, 7);


// media on notify module error code { 16000, 16099 }
constexpr int32_t E_MEDIA_NOTIFY_OFFSET = 16000;
constexpr int32_t E_ASYNC_WORKER_IS_NULL =             MEDIA_LIBRARY_ERR(E_MEDIA_NOTIFY_OFFSET, 0);
constexpr int32_t E_NOTIFY_TASK_DATA_IS_NULL =         MEDIA_LIBRARY_ERR(E_MEDIA_NOTIFY_OFFSET, 1);
constexpr int32_t E_SOLVE_URIS_FAILED =                MEDIA_LIBRARY_ERR(E_MEDIA_NOTIFY_OFFSET, 2);
constexpr int32_t E_NOTIFY_CHANGE_EXT_FAILED =         MEDIA_LIBRARY_ERR(E_MEDIA_NOTIFY_OFFSET, 3);
constexpr int32_t E_PARCEL_GET_SIZE_FAILED =           MEDIA_LIBRARY_ERR(E_MEDIA_NOTIFY_OFFSET, 4);
constexpr int32_t E_GET_ALBUM_ID_FAILED =              MEDIA_LIBRARY_ERR(E_MEDIA_NOTIFY_OFFSET, 5);
constexpr int32_t E_DO_NOT_NEDD_SEND_NOTIFY =          MEDIA_LIBRARY_ERR(E_MEDIA_NOTIFY_OFFSET, 6);
} // namespace Media
} // namespace OHOS
#endif // OHOS_MEDIALIBRARY_ERRNO_H
