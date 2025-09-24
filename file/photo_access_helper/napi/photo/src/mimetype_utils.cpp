/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "mimetype_utils.h"

#include <algorithm>
#include <fstream>

#include "log.h"
#include "medialibrary_errno.h"


using std::string;
using std::vector;
using std::unordered_map;

namespace OHOS {
namespace Media {

MediaType MimeTypeUtils::GetMediaTypeFromMimeType(const string &mimeType)
{
    size_t pos = mimeType.find_first_of("/");
    if (pos == string::npos) {
        LOGE("Invalid mime type: %{private}s", mimeType.c_str());
        return MEDIA_TYPE_FILE;
    }
    string prefix = mimeType.substr(0, pos);
    if (prefix == "audio") {
        return MEDIA_TYPE_AUDIO;
    } else if (prefix == "video") {
        return MEDIA_TYPE_VIDEO;
    } else if (prefix == "image") {
        return MEDIA_TYPE_IMAGE;
    } else {
        return MEDIA_TYPE_FILE;
    }
}
}
}
