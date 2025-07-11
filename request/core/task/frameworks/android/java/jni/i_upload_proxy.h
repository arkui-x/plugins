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

#ifndef PLUGINS_REQUEST_I_UPLOAD_PROXY_H
#define PLUGINS_REQUEST_I_UPLOAD_PROXY_H

#include <functional>
#include <string>

namespace OHOS::Plugin::Request {
using UploadCallback = std::function<void(int64_t taskId, const std::string &type, const std::string &params)>;
class IUploadProxy {
public:
    virtual ~IUploadProxy() = default;
    virtual bool Start(UploadCallback callback) = 0;
    virtual bool Remove() = 0;
    virtual bool Stop() = 0;
    virtual bool Pause() = 0;
    virtual bool Resume() = 0;
};
} // namespace OHOS::Plugin::Request
#endif // PLUGINS_REQUEST_UPLOAD_PROXY_H