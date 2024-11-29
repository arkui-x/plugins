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
#define MLOG_TAG "FileUtils"

#include "media_file_utils.h"

#include <algorithm>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <ftw.h>
#include <regex>
#include <sstream>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>

#include "directory_ex.h"
#include "media_column.h"
#include "log.h"
#include "medialibrary_db_const.h"
#include "medialibrary_errno.h"
#include "medialibrary_type_const.h"
#include "string_ex.h"
#include "media_file_utils.h"

using namespace std;

namespace OHOS::Media {
static const mode_t CHOWN_RWX_USR_GRP = 02771;
static const mode_t CHOWN_RW_USR_GRP = 0660;
constexpr size_t DISPLAYNAME_MAX = 255;
const int32_t OPEN_FDS = 64;
const std::string PATH_PARA = "path=";
constexpr size_t EMPTY_DIR_ENTRY_COUNT = 2;  // Empty dir has 2 entry: . and ..
constexpr size_t DEFAULT_TIME_SIZE = 32;

string MediaFileUtils::GetFileName(const string &filePath)
{
    string fileName;

    if (!(filePath.empty())) {
        size_t lastSlash = filePath.rfind('/');
        if (lastSlash != string::npos) {
            if (filePath.size() > (lastSlash + 1)) {
                fileName = filePath.substr(lastSlash + 1);
            }
        }
    }

    return fileName;
}

bool MediaFileUtils::IsDirectory(const string &dirName, shared_ptr<int> errCodePtr)
{
    struct stat statInfo {};

    if (stat(dirName.c_str(), &statInfo) == SUCCESS) {
        if (statInfo.st_mode & S_IFDIR) {
            return true;
        }
    } else if (errCodePtr != nullptr) {
        *errCodePtr = errno;
        return false;
    }

    return false;
}

int32_t MediaFileUtils::CheckStringSize(const string &str, const size_t max)
{
    size_t size = str.length();
    if (size == 0) {
        return -EINVAL;
    }
    if (size > max) {
        return -ENAMETOOLONG;
    }
    return E_OK;
}

static inline bool RegexCheck(const string &str, const string &regexStr)
{
    const regex express(regexStr);
    return regex_search(str, express);
}

static inline int32_t CheckTitle(const string &title)
{
    static const string TITLE_REGEX_CHECK = R"([\.\\/:*?"'`<>|{}\[\]])";
    if (RegexCheck(title, TITLE_REGEX_CHECK)) {
        LOGE("Failed to check title regex: %{private}s", title.c_str());
        return -EINVAL;
    }
    return E_OK;
}

int32_t MediaFileUtils::CheckDisplayName(const string &displayName)
{
    int err = CheckStringSize(displayName, DISPLAYNAME_MAX);
    if (err < 0) {
        return err;
    }
    if (displayName.at(0) == '.') {
        return -EINVAL;
    }
    string title = GetTitleFromDisplayName(displayName);
    if (title.empty()) {
        return -EINVAL;
    }
    return CheckTitle(title);
}

int32_t MediaFileUtils::CheckFileDisplayName(const string &displayName)
{
    int err = CheckStringSize(displayName, DISPLAYNAME_MAX);
    if (err < 0) {
        return err;
    }
    if (displayName.at(0) == '.') {
        return -EINVAL;
    }
    static const string TITLE_REGEX_CHECK = R"([\\/:*?"'`<>|{}\[\]])";
    if (RegexCheck(displayName, TITLE_REGEX_CHECK)) {
        LOGE("Failed to check displayName regex: %{private}s", displayName.c_str());
        return -EINVAL;
    }
    return E_OK;
}

void MediaFileUtils::FormatRelativePath(string &relativePath)
{
    if (relativePath.empty()) {
        return;
    }
    if (relativePath.back() != '/') {
        relativePath += '/';
    }
    if (relativePath.front() == '/') {
        relativePath = relativePath.substr(1);
    }
}

int32_t MediaFileUtils::CheckAlbumName(const string &albumName)
{
    int err = CheckStringSize(albumName, DISPLAYNAME_MAX);
    if (err < 0) {
        return err;
    }

    static const string ALBUM_NAME_REGEX = R"([\.\\/:*?"'`<>|{}\[\]])";
    if (RegexCheck(albumName, ALBUM_NAME_REGEX)) {
        LOGE("Failed to check album name regex: %{private}s", albumName.c_str());
        return -EINVAL;
    }
    return E_OK;
}

string MediaFileUtils::GetTitleFromDisplayName(const string &displayName)
{
    string title;
    if (!displayName.empty()) {
        string::size_type pos = displayName.find_last_of('.');
        if (pos == string::npos) {
            return "";
        }
        title = displayName.substr(0, pos);
    }
    return title;
}

string MediaFileUtils::GetNetworkIdFromUri(const string &uri)
{
 //   return MediaFileUri(uri).GetNetworkId();
 return "";
}

string MediaFileUtils::UpdatePath(const string &path, const string &uri)
{
    string retStr = path;
    LOGI("MediaFileUtils::UpdatePath path = %{private}s, uri = %{private}s", path.c_str(), uri.c_str());
    if (path.empty() || uri.empty()) {
        return retStr;
    }

    string networkId = GetNetworkIdFromUri(uri);
    if (networkId.empty()) {
        LOGI("MediaFileUtils::UpdatePath retStr = %{private}s", retStr.c_str());
        return retStr;
    }

    size_t pos = path.find(MEDIA_DATA_DEVICE_PATH);
    if (pos == string::npos) {
        return retStr;
    }

    string beginStr = path.substr(0, pos);
    if (beginStr.empty()) {
        return retStr;
    }

    string endStr = path.substr(pos + MEDIA_DATA_DEVICE_PATH.length());
    if (endStr.empty()) {
        return retStr;
    }

    retStr = beginStr + networkId + endStr;
    LOGI("MediaFileUtils::UpdatePath retStr = %{private}s", retStr.c_str());
    return retStr;
}

MediaType MediaFileUtils::GetMediaType(const string &filePath)
{
    if (filePath.empty()) {
        return MEDIA_TYPE_ALL;
    }
    return Media::MEDIA_TYPE_IMAGE;
}

string MediaFileUtils::SplitByChar(const string &str, const char split)
{
    size_t splitIndex = str.find_last_of(split);
    return (splitIndex == string::npos) ? ("") : (str.substr(splitIndex + 1));
}

string MediaFileUtils::GetExtensionFromPath(const string &path)
{
    string extention = SplitByChar(path, '.');
    if (!extention.empty()) {
        transform(extention.begin(), extention.end(), extention.begin(), ::tolower);
    }
    return extention;
}

std::string MediaFileUtils::GetMediaTypeUri(MediaType mediaType)
{
    switch (mediaType) {
        case MEDIA_TYPE_AUDIO:
            return MEDIALIBRARY_AUDIO_URI;
        case MEDIA_TYPE_VIDEO:
            return MEDIALIBRARY_VIDEO_URI;
        case MEDIA_TYPE_IMAGE:
            return MEDIALIBRARY_IMAGE_URI;
        case MEDIA_TYPE_SMARTALBUM:
            return MEDIALIBRARY_SMARTALBUM_CHANGE_URI;
        case MEDIA_TYPE_DEVICE:
            return MEDIALIBRARY_DEVICE_URI;
        case MEDIA_TYPE_FILE:
        default:
            return MEDIALIBRARY_FILE_URI;
    }
}

bool MediaFileUtils::StartsWith(const std::string &str, const std::string &prefix)
{
    return str.compare(0, prefix.size(), prefix) == 0;
}

void MediaFileUtils::UriAppendKeyValue(string &uri, const string &key, std::string value)
{
    string uriKey = key + '=';
    if (uri.find(uriKey) != string::npos) {
        return;
    }

    char queryMark = (uri.find('?') == string::npos) ? '?' : '&';
    string append = queryMark + key + '=' + value;

    size_t pos = uri.find('#');
    if (pos == string::npos) {
        uri += append;
    } else {
        uri.insert(pos, append);
    }
}

string MediaFileUtils::GetExtraUri(const string &displayName, const string &path, const bool isNeedEncode)
{
    string extraUri = "/" + GetTitleFromDisplayName(GetFileName(path)) + "/" + displayName;
    if (!isNeedEncode) {
        return extraUri;
    }
    return MediaFileUtils::Encode(extraUri);
}

string MediaFileUtils::GetUriByExtrConditions(const string &prefix, const string &fileId, const string &suffix)
{
    return prefix + fileId + suffix;
}

string MediaFileUtils::Encode(const string &uri)
{
    const unordered_set<char> uriCompentsSet = {
        ';', ',', '/', '?', ':', '@', '&',
        '=', '+', '$', '-', '_', '.', '!',
        '~', '*', '(', ')', '\''
    };
    const int32_t encodeLen = 2;
    ostringstream outPutStream;
    outPutStream.fill('0');
    outPutStream << std::hex;

    for (unsigned char tmpChar : uri) {
        if (std::isalnum(tmpChar) || uriCompentsSet.find(tmpChar) != uriCompentsSet.end()) {
            outPutStream << tmpChar;
        } else {
            outPutStream << std::uppercase;
            outPutStream << '%' << std::setw(encodeLen) << static_cast<unsigned int>(tmpChar);
            outPutStream << std::nouppercase;
        }
    }

    return outPutStream.str();
}

string MediaFileUtils::RemoveDocsFromRelativePath(const string &relativePath)
{
    if (MediaFileUtils::StartsWith(relativePath, DOCS_PATH)) {
        return relativePath.substr(DOCS_PATH.size());
    }
    return relativePath;
}
} // namespace OHOS::Media
