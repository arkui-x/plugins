/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2026. All rights reserved.
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

#include <vector>

#include "hilog/log.h"
#include "string_ex.h"

using OHOS::HiviewDFX::HiLog;
using std::string;

namespace OHOS {
namespace {
const string NOT_CACHED = "NOT VALID";
const string EMPTY = "";
const size_t NOT_FOUND = string::npos;
const int NOT_CALCULATED = -2;
const int PORT_NONE = -1;
const char SCHEME_SEPARATOR = ':';
const char SCHEME_FRAGMENT = '#';
const char LEFT_SEPARATOR = '/';
const char RIGHT_SEPARATOR = '\\';
const char QUERY_FLAG = '?';
const char USER_HOST_SEPARATOR = '@';
const char PORT_SEPARATOR = ':';
const size_t POS_INC = 1;
const size_t POS_INC_MORE = 2;
const size_t POS_INC_AGAIN = 3;
}; // namespace

Uri::Uri(const string& uriString)
{
    cachedSsi_ = NOT_FOUND;
    cachedFsi_ = NOT_FOUND;
    port_ = NOT_CALCULATED;

    if (uriString.empty()) {
        return;
    }

    uriString_ = uriString;
    scheme_ = NOT_CACHED;
    ssp_ = NOT_CACHED;
    authority_ = NOT_CACHED;
    host_ = NOT_CACHED;
    userInfo_ = NOT_CACHED;
    query_ = NOT_CACHED;
    path_ = NOT_CACHED;
    fragment_ = NOT_CACHED;

    if (!CheckScheme()) {
        uriString_ = EMPTY;
        HILOG_IMPL(LOG_CORE, LOG_ERROR, 0xD001305, "URI", "Scheme wrong");
    }
}

Uri::~Uri() {}

bool Uri::CheckScheme()
{
    scheme_ = ParseScheme();
    if (scheme_.empty()) {
        return true;
    }

    auto isAlpha = [](char ch) -> bool { return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'); };
    auto isDigit = [](char ch) -> bool { return (ch >= '0' && ch <= '9'); };
    auto isSchemeChar = [&](char ch) -> bool {
        return isAlpha(ch) || isDigit(ch) || ch == '+' || ch == '-' || ch == '.';
    };

    // RFC 3986: scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
    if (!isAlpha(scheme_.front())) {
        return false;
    }
    for (size_t i = 1; i < scheme_.size(); ++i) {
        if (!isSchemeChar(scheme_[i])) {
            return false;
        }
    }
    return true;
}

string Uri::GetScheme()
{
    if (uriString_.empty()) {
        return EMPTY;
    }

    if (scheme_ == NOT_CACHED) {
        scheme_ = ParseScheme();
    }
    return scheme_;
}

string Uri::ParseScheme()
{
    size_t ssi = FindSchemeSeparator();
    return (ssi == NOT_FOUND) ? EMPTY : uriString_.substr(0, ssi);
}

string Uri::GetSchemeSpecificPart()
{
    if (uriString_.empty()) {
        return EMPTY;
    }

    return (ssp_ == NOT_CACHED) ? (ssp_ = ParseSsp()) : ssp_;
}

string Uri::ParseSsp()
{
    size_t ssi = FindSchemeSeparator();
    size_t fsi = FindFragmentSeparator();

    size_t start = (ssi == NOT_FOUND) ? 0 : (ssi + 1);
    size_t end = (fsi == NOT_FOUND) ? uriString_.size() : fsi;

    // Return everything between ssi and fsi.
    string ssp = EMPTY;
    if (end > start) {
        ssp = uriString_.substr(start, end - start);
    }

    return ssp;
}

string Uri::GetAuthority()
{
    if (uriString_.empty()) {
        return EMPTY;
    }

    if (authority_ == NOT_CACHED) {
        authority_ = ParseAuthority();
    }
    return authority_;
}

string Uri::ParseAuthority()
{
    size_t ssi = FindSchemeSeparator();
    if (ssi == NOT_FOUND) {
        return EMPTY;
    }

    size_t length = uriString_.length();
    // If "//" follows the scheme separator, we have an authority.
    if ((length > (ssi + POS_INC_MORE)) && (uriString_.at(ssi + POS_INC) == LEFT_SEPARATOR) &&
        (uriString_.at(ssi + POS_INC_MORE) == LEFT_SEPARATOR)) {
        // Look for the start of the path, query, or fragment, or the end of the string.
        size_t start = ssi + POS_INC_AGAIN;
        size_t end = start;

        while (end < length) {
            char ch = uriString_.at(end);
            if ((ch == LEFT_SEPARATOR) || (ch == RIGHT_SEPARATOR) || (ch == QUERY_FLAG) || (ch == SCHEME_FRAGMENT)) {
                break;
            }

            end++;
        }

        return uriString_.substr(start, end - start);
    } else {
        return EMPTY;
    }
}

string Uri::GetUserInfo()
{
    if (uriString_.empty()) {
        return EMPTY;
    }

    if (userInfo_ == NOT_CACHED) {
        userInfo_ = ParseUserInfo();
    }
    return userInfo_;
}

string Uri::ParseUserInfo()
{
    string authority = GetAuthority();
    if (authority.empty()) {
        return EMPTY;
    }

    size_t end = authority.find_last_of(USER_HOST_SEPARATOR);
    return (end == NOT_FOUND) ? EMPTY : authority.substr(0, end);
}

string Uri::GetHost()
{
    if (uriString_.empty()) {
        return EMPTY;
    }

    if (host_ == NOT_CACHED) {
        host_ = ParseHost();
    }
    return host_;
}

string Uri::ParseHost()
{
    string authority = GetAuthority();
    if (authority.empty()) {
        return EMPTY;
    }

    // Parse out user info and then port.
    size_t userInfoSeparator = authority.find_last_of(USER_HOST_SEPARATOR);
    size_t start = (userInfoSeparator == NOT_FOUND) ? 0 : (userInfoSeparator + 1);
    size_t portSeparator = authority.find_first_of(PORT_SEPARATOR, start);
    size_t end = (portSeparator == NOT_FOUND) ? authority.size() : portSeparator;

    string host = EMPTY;
    if (start < end) {
        host = authority.substr(start, end - start);
    }

    return host;
}

int Uri::GetPort()
{
    if (uriString_.empty()) {
        return PORT_NONE;
    }

    if (port_ == NOT_CALCULATED) {
        port_ = ParsePort();
    }
    return port_;
}

int Uri::ParsePort()
{
    string authority = GetAuthority();
    if (authority.empty()) {
        return PORT_NONE;
    }

    // Make sure we look for the port separtor *after* the user info separator.
    size_t userInfoSeparator = authority.find_last_of(USER_HOST_SEPARATOR);
    size_t start = (userInfoSeparator == NOT_FOUND) ? 0 : (userInfoSeparator + 1);
    size_t portSeparator = authority.find_first_of(PORT_SEPARATOR, start);
    if (portSeparator == NOT_FOUND) {
        return PORT_NONE;
    }

    start = portSeparator + 1;
    string portString = authority.substr(start);

    int value = PORT_NONE;
    return StrToInt(portString, value) ? value : PORT_NONE;
}

string Uri::GetQuery()
{
    if (uriString_.empty()) {
        return EMPTY;
    }

    if (query_ == NOT_CACHED) {
        query_ = ParseQuery();
    }
    return query_;
}

string Uri::ParseQuery()
{
    size_t ssi = FindSchemeSeparator();
    if (ssi == NOT_FOUND) {
        ssi = 0;
    }
    size_t qsi = uriString_.find_first_of(QUERY_FLAG, ssi);
    if (qsi == NOT_FOUND) {
        return EMPTY;
    }

    size_t start = qsi + 1;
    size_t fsi = FindFragmentSeparator();
    if (fsi == NOT_FOUND) {
        return uriString_.substr(start);
    }

    if (fsi < qsi) {
        // Invalid.
        return EMPTY;
    }

    return uriString_.substr(start, fsi - start);
}

string Uri::GetPath()
{
    if (uriString_.empty()) {
        return EMPTY;
    }

    if (path_ == NOT_CACHED) {
        path_ = ParsePath();
    }
    return path_;
}

void Uri::GetPathSegments(std::vector<std::string>& segments)
{
    if (uriString_.empty()) {
        return;
    }
    if (path_ == NOT_CACHED) {
        path_ = ParsePath();
    }

    size_t previous = 0;
    size_t current;
    while ((current = path_.find(LEFT_SEPARATOR, previous)) != std::string::npos) {
        if (previous < current) {
            segments.emplace_back(path_.substr(previous, current - previous));
        }
        previous = current + POS_INC;
    }
    // Add in the final path segment.
    if (previous < path_.length()) {
        segments.emplace_back(path_.substr(previous));
    }
}

string Uri::ParsePath()
{
    size_t ssi = FindSchemeSeparator();
    // If the URI is absolute.
    if (ssi != NOT_FOUND) {
        // Is there anything after the ':'?
        if ((ssi + 1) == uriString_.length()) {
            // Opaque URI.
            return EMPTY;
        }

        // A '/' after the ':' means this is hierarchical.
        if (uriString_.at(ssi + 1) != LEFT_SEPARATOR) {
            // Opaque URI.
            return EMPTY;
        }
    } else {
        // All relative URIs are hierarchical.
    }

    return ParsePath(ssi);
}

string Uri::ParsePath(size_t ssi)
{
    size_t length = uriString_.length();

    // Find start of path.
    size_t pathStart = (ssi == NOT_FOUND) ? 0 : (ssi + POS_INC);
    if ((length > (pathStart + POS_INC)) && (uriString_.at(pathStart) == LEFT_SEPARATOR) &&
        (uriString_.at(pathStart + POS_INC) == LEFT_SEPARATOR)) {
        // Skip over authority to path.
        pathStart += POS_INC_MORE;

        while (pathStart < length) {
            char ch = uriString_.at(pathStart);
            if ((ch == QUERY_FLAG) || (ch == SCHEME_FRAGMENT)) {
                return EMPTY;
            }

            if ((ch == LEFT_SEPARATOR) || (ch == RIGHT_SEPARATOR)) {
                break;
            }

            pathStart++;
        }
    }

    // Find end of path.
    size_t pathEnd = pathStart;
    while (pathEnd < length) {
        char ch = uriString_.at(pathEnd);
        if ((ch == QUERY_FLAG) || (ch == SCHEME_FRAGMENT)) {
            break;
        }

        pathEnd++;
    }

    return uriString_.substr(pathStart, pathEnd - pathStart);
}

string Uri::GetFragment()
{
    if (uriString_.empty()) {
        return EMPTY;
    }

    if (fragment_ == NOT_CACHED) {
        fragment_ = ParseFragment();
    }
    return fragment_;
}

string Uri::ParseFragment()
{
    size_t fsi = FindFragmentSeparator();
    return (fsi == NOT_FOUND) ? EMPTY : uriString_.substr(fsi + 1);
}

size_t Uri::FindSchemeSeparator()
{
    if (cachedSsi_ == NOT_FOUND) {
        cachedSsi_ = uriString_.find_first_of(SCHEME_SEPARATOR);
    }
    return cachedSsi_;
}

size_t Uri::FindFragmentSeparator()
{
    if (cachedFsi_ == NOT_FOUND) {
        cachedFsi_ = uriString_.find_first_of(SCHEME_FRAGMENT, FindSchemeSeparator());
    }
    return cachedFsi_;
}

bool Uri::IsHierarchical()
{
    if (uriString_.empty()) {
        return false;
    }

    size_t ssi = FindSchemeSeparator();
    if (ssi == NOT_FOUND) {
        // All relative URIs are hierarchical.
        return true;
    }

    if (uriString_.length() == (ssi + 1)) {
        // No ssp.
        return false;
    }

    // If the ssp starts with a '/', this is hierarchical.
    return (uriString_.at(ssi + 1) == LEFT_SEPARATOR);
}

bool Uri::IsAbsolute()
{
    if (uriString_.empty()) {
        return false;
    }

    return !IsRelative();
}

bool Uri::IsRelative()
{
    if (uriString_.empty()) {
        return false;
    }

    // Note: We return true if the index is 0
    return FindSchemeSeparator() == NOT_FOUND;
}

bool Uri::Equals(const Uri& other) const
{
    return uriString_ == other.ToString();
}

int Uri::CompareTo(const Uri& other) const
{
    return uriString_.compare(other.ToString());
}

string Uri::ToString() const
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
