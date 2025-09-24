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

#include "userfile_client.h"
#include "photo_client.h"

#include "medialibrary_errno.h"
#include "log.h"
#include "datashare_predicates.h"

using namespace std;

namespace OHOS {
namespace Media {
shared_ptr<PhotoClient> UserFileClient::plugin = nullptr;

bool UserFileClient::IsValid()
{
    return plugin != nullptr;
}

void UserFileClient::Init(napi_env env, napi_callback_info info)
{
    plugin = PhotoClient::GetInstance();
}

void UserFileClient::startPhotoPicker(std::string &type) {
    if (plugin != nullptr) {
        plugin->startPhotoPicker(type);
    }
}

std::string UserFileClient::InsertExt(int photoType, const std::string &extension,
    const std::string &title, int &errCode)
{
    if (!IsValid()) {
        LOGE("InsertExt fail, helper null");
        return "";
    }
    return plugin->InsertExt(photoType, extension, title, errCode);
}

std::string UserFileClient::GetMimeTypeFromExtension(const std::string &extension)
{
    if (!IsValid()) {
        LOGE("GetMimeTypeFromExtension fail, helper null");
        return "";
    }
    return plugin->GetMimeTypeFromExtension(extension);
}

shared_ptr<ResultSet> UserFileClient::QueryAlbum(DataShare::DataSharePredicates &predicates,
    std::vector<std::string> &columns, int &errCode)
{
    if (!IsValid()) {
        LOGE("QueryAlbum fail, helper null");
        return nullptr;
    }
    return plugin->QueryAlbum(predicates, columns, errCode);
}

shared_ptr<ResultSet> UserFileClient::Query(DataShare::DataSharePredicates &predicates,
    std::vector<std::string> &columns, int &errCode)
{
    if (!IsValid()) {
        LOGE("Query fail, helper null");
        return nullptr;
    }
    return plugin->Query(predicates, columns, errCode);
}

int UserFileClient::Update(std::string &uri, const DataShare::DataSharePredicates &predicates,
    const DataShare::DataShareValuesBucket &value)
{
    if (!IsValid()) {
        LOGE("update fail, helper null");
        return E_FAIL;
    }
    return 0;
}
}
}
