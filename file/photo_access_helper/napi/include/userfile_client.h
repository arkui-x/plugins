/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_USER_FILE_CLIENT_H
#define INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_USER_FILE_CLIENT_H

#include <string>

#include "photo_client.h"
#include "napi_error.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "photo_picker_napi.h"

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
class UserFileClient {
public:
    EXPORT UserFileClient() {}
    EXPORT virtual ~UserFileClient() {}
    EXPORT static bool IsValid();
    EXPORT static void Init(napi_env env, napi_callback_info info);
    EXPORT static void startPhotoPicker(std::string &type);

private:
    static std::shared_ptr<PhotoClient> plugin;
};
}
}

#endif // INTERFACES_KITS_JS_MEDIALIBRARY_INCLUDE_USER_FILE_CLIENT_H