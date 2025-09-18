/*
 * Copyright (C) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_USER_PHOTO_CLIENT_H
#define INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_USER_PHOTO_CLIENT_H

#include <string>
#include <vector>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "result_set.h"
#include "datashare_predicates.h"

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
class PhotoClient {
public:
    EXPORT PhotoClient() {}
    EXPORT virtual ~PhotoClient() {}

    static std::shared_ptr<PhotoClient> GetInstance();
    virtual std::shared_ptr<ResultSet> QueryAlbum(DataShare::DataSharePredicates &predicates,
        std::vector<std::string> &columns, int &errCode) = 0;
    virtual std::shared_ptr<ResultSet> Query(DataShare::DataSharePredicates &predicates,
        std::vector<std::string> &columns, int &errCode) = 0;
    virtual int Update(std::string &uri, const std::string &predicates,
        const std::string &value) = 0;
    virtual void startPhotoPicker(std::string &type) = 0;
    virtual std::string InsertExt(int photoType, const std::string &extension,
        const std::string &title, int &errCode) = 0;
    virtual std::string GetMimeTypeFromExtension(const std::string &extension) = 0;
};
}
}

#endif // INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_USER_PHOTO_CLIENT_H
