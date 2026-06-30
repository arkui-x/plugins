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

#include "uri.h"

namespace OHOS {
namespace {
const std::string EMPTY = "";
const std::string FILE_SCHEME_PREFIX = "file://";
const int PORT_NONE = -1;
const size_t NOT_FOUND = std::string::npos;
const char QUERY_SEPARATOR = '?';
const char FRAGMENT_SEPARATOR = '#';
const char PATH_SEPARATOR = '/';

size_t FindPathEnd(const std::string& text, size_t start)
{
    size_t queryPos = text.find(QUERY_SEPARATOR, start);
    size_t fragmentPos = text.find(FRAGMENT_SEPARATOR, start);
    size_t endPos = text.size();
    if (queryPos != NOT_FOUND && queryPos < endPos) {
        endPos = queryPos;
    }
    if (fragmentPos != NOT_FOUND && fragmentPos < endPos) {
        endPos = fragmentPos;
    }
    return endPos;
}
} // namespace

Uri::Uri(const std::string& uriString)
{
    uriString_ = uriString;
    scheme_ = EMPTY;
    ssp_ = EMPTY;
    authority_ = EMPTY;
    host_ = EMPTY;
    port_ = PORT_NONE;
    userInfo_ = EMPTY;
    query_ = EMPTY;
    path_ = EMPTY;
    fragment_ = EMPTY;
    cachedSsi_ = NOT_FOUND;
    cachedFsi_ = NOT_FOUND;
}

Uri::~Uri() {}

std::string Uri::GetScheme()
{
    return EMPTY;
}

std::string Uri::GetSchemeSpecificPart()
{
    return EMPTY;
}

std::string Uri::GetAuthority()
{
    return EMPTY;
}

std::string Uri::GetHost()
{
    return EMPTY;
}

int Uri::GetPort()
{
    return PORT_NONE;
}

std::string Uri::GetUserInfo()
{
    return EMPTY;
}

std::string Uri::GetQuery()
{
    return EMPTY;
}

std::string Uri::GetPath()
{
    if (uriString_.empty()) {
        return EMPTY;
    }

    if (uriString_.find(FILE_SCHEME_PREFIX) == 0) {
        std::string remainder = uriString_.substr(FILE_SCHEME_PREFIX.size());
        size_t pathStart = remainder.find(PATH_SEPARATOR);
        if (pathStart == NOT_FOUND) {
            return EMPTY;
        }
        size_t pathEnd = FindPathEnd(remainder, pathStart);
        return remainder.substr(pathStart, pathEnd - pathStart);
    }

    size_t schemePos = uriString_.find(':');
    if (schemePos != NOT_FOUND) {
        return EMPTY;
    }

    size_t pathEnd = FindPathEnd(uriString_, 0);
    return uriString_.substr(0, pathEnd);
}

void Uri::GetPathSegments(std::vector<std::string>& segments)
{
    std::string path = GetPath();
    if (path.empty()) {
        return;
    }

    size_t previous = 0;
    size_t current = 0;
    while ((current = path.find(PATH_SEPARATOR, previous)) != std::string::npos) {
        if (previous < current) {
            segments.emplace_back(path.substr(previous, current - previous));
        }
        previous = current + 1;
    }
    if (previous < path.length()) {
        segments.emplace_back(path.substr(previous));
    }
}

std::string Uri::GetFragment()
{
    return EMPTY;
}

bool Uri::IsHierarchical()
{
    return false;
}

bool Uri::IsAbsolute()
{
    return false;
}

bool Uri::IsRelative()
{
    return false;
}

bool Uri::Equals(const Uri& other) const
{
    return uriString_ == other.ToString();
}

int Uri::CompareTo(const Uri& other) const
{
    return uriString_.compare(other.ToString());
}

std::string Uri::ToString() const
{
    return uriString_;
}

bool Uri::operator==(const Uri& other) const
{
    return uriString_ == other.ToString();
}

bool Uri::Marshalling(Parcel& parcel) const
{
    return parcel.WriteString(uriString_);
}

Uri* Uri::Unmarshalling(Parcel& parcel)
{
    return new Uri(parcel.ReadString());
}
} // namespace OHOS