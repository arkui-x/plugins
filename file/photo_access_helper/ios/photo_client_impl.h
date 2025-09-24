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

#ifndef PLUGINS_PHOTO_ACCESS_HELPER_PHOTO_CLIENT_IMPL_IOS_H
#define PLUGINS_PHOTO_ACCESS_HELPER_PHOTO_CLIENT_IMPL_IOS_H

#include <string>
#include <vector>

#include "photo_client.h"

namespace OHOS {
namespace Media {
class PhotoClientImpl : public PhotoClient {
public:
    std::shared_ptr<ResultSet> QueryAlbum(DataShare::DataSharePredicates &predicates,
        std::vector<std::string> &columns, int &errCode) override;
    std::shared_ptr<ResultSet> Query(DataShare::DataSharePredicates &predicates,
        std::vector<std::string> &columns, int &errCode) override;
    int Update(std::string &uri, const std::string &predicates,
        const std::string &value) override;
    void startPhotoPicker(std::string &type) override;
    std::string InsertExt(int photoType, const std::string &extension,
        const std::string &title, int &errCode) override;
    std::string GetMimeTypeFromExtension(const std::string &extension) override;
};
}
}

#endif // PLUGINS_PHOTO_ACCESS_HELPER_PHOTO_CLIENT_IMPL_IOS_H