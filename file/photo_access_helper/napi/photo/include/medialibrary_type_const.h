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

#ifndef INTERFACES_INNERKITS_NATIVE_INCLUDE_MEDIA_LIB_SERVICE_CONST_H_
#define INTERFACES_INNERKITS_NATIVE_INCLUDE_MEDIA_LIB_SERVICE_CONST_H_

#include <unordered_set>
#include <vector>

#include "userfile_manager_types.h"

namespace OHOS {
namespace Media {
enum {
    MEDIA_GET_MEDIA_ASSETS = 0,
    MEDIA_GET_IMAGE_ASSETS = 1,
    MEDIA_GET_AUDIO_ASSETS = 2,
    MEDIA_GET_VIDEO_ASSETS = 3,
    MEDIA_GET_IMAGEALBUM_ASSETS = 4,
    MEDIA_GET_VIDEOALBUM_ASSETS = 5,
    MEDIA_CREATE_MEDIA_ASSET = 6,
    MEDIA_DELETE_MEDIA_ASSET = 7,
    MEDIA_MODIFY_MEDIA_ASSET = 8,
    MEDIA_COPY_MEDIA_ASSET   = 9,
    MEDIA_CREATE_MEDIA_ALBUM_ASSET  = 10,
    MEDIA_DELETE_MEDIA_ALBUM_ASSET  = 11,
    MEDIA_MODIFY_MEDIA_ALBUM_ASSET  = 12,
};

enum class FetchResType : int32_t {
    TYPE_FILE = 1,
    TYPE_ALBUM,
    TYPE_SMARTALBUM,
    TYPE_PHOTOALBUM
};

enum ResultSetDataType {
    TYPE_NULL = 0,
    TYPE_STRING,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_DOUBLE
};

const int32_t SUCCESS = 0;

constexpr int32_t DEFAULT_INT32 = 0;
constexpr int64_t DEFAULT_INT64 = 0;
const std::string DEFAULT_STR = "";
const bool DEFAULT_BOOL = false;
const std::string DEFAULT_MEDIA_PATH = "";

const int32_t DEFAULT_ALBUM_ID = 0;
const int32_t DEFAULT_COUNT = 0;
const std::string DEFAULT_MEDIA_ALBUM_URI = "";
const std::string DOCS_PATH = "Docs/";
} // namespace OHOS
} // namespace Media

#endif  // INTERFACES_INNERKITS_NATIVE_INCLUDE_MEDIA_LIB_SERVICE_CONST_H_
