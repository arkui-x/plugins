/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_FILE_FILEURI_URI_H
#define PLUGINS_FILE_FILEURI_URI_H

#include <string>
#include <vector>

#include "parcel.h"

namespace OHOS {
class Uri : public Parcelable {
public:
    explicit Uri(const std::string& uriString);
    ~Uri();

    std::string GetScheme();

    std::string GetSchemeSpecificPart();

    std::string GetAuthority();

    std::string GetHost();

    int GetPort();

    std::string GetUserInfo();

    std::string GetQuery();

    std::string GetPath();

    void GetPathSegments(std::vector<std::string>& segments);

    std::string GetFragment();

    bool IsHierarchical();

    bool IsAbsolute();

    bool IsRelative();

    bool Equals(const Uri& other) const;

    int CompareTo(const Uri& other) const;

    std::string ToString() const;

    bool operator==(const Uri& other) const;

    bool Marshalling(Parcel& parcel) const override;

    static Uri* Unmarshalling(Parcel& parcel);
private:
    std::string uriString_;
    std::string scheme_;
    std::string ssp_;
    std::string authority_;
    std::string host_;
    int port_;
    std::string userInfo_;
    std::string query_;
    std::string path_;
    std::string fragment_;
    size_t cachedSsi_;
    size_t cachedFsi_;
};
} // namespace OHOS
#endif // PLUGINS_FILE_FILEURI_URI_H