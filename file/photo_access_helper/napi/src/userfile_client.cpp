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

#include "userfile_client.h"
#include "photo_client.h"

#include "log.h"

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
}
}
