/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_REQUEST_UPLOAD_IUPLOAD_TASK
#define PLUGINS_REQUEST_UPLOAD_IUPLOAD_TASK

#include "upload_common.h"

namespace OHOS::Plugin::Request::Upload {
class IUploadTask {
public:
    virtual void OnProgress(curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) = 0;
    virtual void OnFail() = 0;
    virtual void OnComplete() = 0;
    int taskId_;
};

class IUploadTaskRelease {
public:
    virtual void OnUploadTaskRelease(void *arg) = 0;
};
} // end of OHOS::Plugin::Request::Upload
#endif