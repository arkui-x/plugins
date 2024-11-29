/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef RDB_UTILS_RESULT_H
#define RDB_UTILS_RESULT_H

#include <string>
#include <vector>
#include <map>

namespace OHOS {
namespace Media {
using namespace OHOS::NativeRdb;
/**
 * The RdbUtilsResult class of RDB.
 */
class RdbUtilsResult {
public:
    RdbUtilsResult(const bool isAlbum);
    ~RdbUtilsResult();
    std::string GetWhereClause() const;
    std::vector<std::string> GetWhereArgs() const;
    bool EqualTo(const std::string &field, const ValueObject &value);
    bool And();
    bool OrderByAsc(const std::string &field);
    bool OrderByDesc(const std::string &field);
    bool Limit(const int limit);
    bool In(const std::string &field, const std::vector<std::string> &values);

    int GetLimit() const;
    int GetOffset() const;
    int GetAlbumType() const;
    long long GetAlbumSubType() const;
    std::string GetLocalIdentifier() const;
    std::map<std::string, int> GetOrder() const;
    bool Offset(const int offset);
    bool IsNeedAnd() const;
    bool IsAscending() const;
private:
    std::string whereClause;
    std::map<std::string, int> order;
    int limit;
    int offset;
    bool isNeedAnd;
    bool isAlbum_;
    int albumType;
    long long albumSubtype;
    std::string localIdentifier;

    void Initial();
    bool CheckParameter(
        const std::string &methodName, const std::string &field) const;
    void CheckIsNeedAnd();
    void AppendWhereClauseWithInOrNotIn(const std::string &methodName, const std::string &field,
        const std::vector<std::string> &replaceValues);

};
} // namespace Media
} // namespace OHOS

#endif