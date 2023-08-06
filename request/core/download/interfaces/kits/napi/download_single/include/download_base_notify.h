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

#ifndef PLUGINS_REQUEST_DOWNLOAD_BASE_NOTIFY_H
#define PLUGINS_REQUEST_DOWNLOAD_BASE_NOTIFY_H

#include <string>

#include "async_call.h"
#include "i_download_task.h"
#include "download_notify_interface.h"
#include "napi/native_api.h"
#include "noncopyable.h"

namespace OHOS::Plugin::Request::Download {
struct NotifyData {
    napi_env env;
    napi_ref ref;
    uint32_t paramNumber;
    std::mutex mutex;
    std::vector<uint32_t> params;
};
struct NotifyDataPtr {
    std::shared_ptr<NotifyData> notifyData;
};
class DownloadBaseNotify : public DownloadNotifyInterface {
public:
    ACE_DISALLOW_COPY_AND_MOVE(DownloadBaseNotify);

    explicit DownloadBaseNotify(napi_env env, uint32_t paramNumber, napi_ref ref);
    virtual ~DownloadBaseNotify();

    void OnCallBack(const std::vector<uint32_t> &params) override;
    NotifyDataPtr *GetNotifyDataPtr();

private:
    std::shared_ptr<NotifyData> notifyData_;
};
} // namespace OHOS::Plugin::Request::Download
#endif // PLUGINS_REQUEST_DOWNLOAD_BASE_NOTIFY_H