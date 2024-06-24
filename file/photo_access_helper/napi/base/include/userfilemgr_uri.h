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

#ifndef OHOS_FILEMANAGEMENT_USERFILEMGR_URI_H
#define OHOS_FILEMANAGEMENT_USERFILEMGR_URI_H

#include <string>

namespace OHOS {
namespace Media {
const std::string MEDIALIBRARY_DATA_URI = "datashare:///media";
const std::string MEDIA_OPERN_KEYWORD = "operation";
const std::string MEDIA_QUERYOPRN = "query_operation";
const std::string OPRN_QUERY = "query";
const std::string OPRN_UPDATE = "update";
const std::string OPRN_INDEX = "index";

// UserFileManager operation constants
const std::string UFM_PHOTO = "userfilemgr_photo_operation";
const std::string UFM_AUDIO = "userfilemgr_audio_operation";
const std::string UFM_ALBUM = "userfilemgr_photo_album_operation";

// UserFileManager photo operation constants
const std::string UFM_UPDATE_PHOTO = MEDIALIBRARY_DATA_URI + "/" + UFM_PHOTO + "/" + OPRN_UPDATE;
const std::string UFM_GET_INDEX = MEDIALIBRARY_DATA_URI + "/" + UFM_PHOTO + "/" + OPRN_INDEX;

// UserFileManager album operation constants
const std::string UFM_UPDATE_PHOTO_ALBUM = MEDIALIBRARY_DATA_URI + "/" + UFM_ALBUM + "/" + OPRN_UPDATE;

// PhotoAccessHelper operation constants
const std::string PAH_PHOTO = "phaccess_photo_operation";
const std::string PAH_ALBUM = "phaccess_album_operation";
const std::string PAH_MAP = "phaccess_map_operation";

// PhotoAccessHelper photo operation constants
const std::string PAH_UPDATE_PHOTO = MEDIALIBRARY_DATA_URI + "/" + PAH_PHOTO + "/" + OPRN_UPDATE;
const std::string PAH_QUERY_PHOTO = MEDIALIBRARY_DATA_URI + "/" + PAH_PHOTO + "/" + OPRN_QUERY;

// PhotoAccessHelper album operation constants
const std::string PAH_UPDATE_PHOTO_ALBUM = MEDIALIBRARY_DATA_URI + "/" + PAH_ALBUM + "/" + OPRN_UPDATE;
const std::string PAH_QUERY_PHOTO_MAP = MEDIALIBRARY_DATA_URI + "/" + PAH_MAP + "/" + OPRN_QUERY;
} // namespace Media
} // namespace OHOS

#endif // OHOS_FILEMANAGEMENT_USERFILEMGR_URI_H
