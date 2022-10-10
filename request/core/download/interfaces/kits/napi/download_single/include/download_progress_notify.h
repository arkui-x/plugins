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

#ifndef PLUGINS_REQUEST_DOWNLOAD_PROGRESS_NOTIFY_H
#define PLUGINS_REQUEST_DOWNLOAD_PROGRESS_NOTIFY_H

#include <string>

#include "async_call.h"
#include "download_base_notify.h"
#include "download_task.h"
#include "napi/native_api.h"
#include "noncopyable.h"

namespace OHOS::Plugin::Request::Download {
class DownloadProgressNotify final : public DownloadBaseNotify {
public:
    ACE_DISALLOW_COPY_AND_MOVE(DownloadProgressNotify);

    explicit DownloadProgressNotify(napi_env env, const std::string &type, const DownloadTask *task, napi_ref ref);
    virtual ~DownloadProgressNotify();

    void OnCallBack(uint32_t firstArgv, uint32_t secondArgv) override;
};
} // namespace OHOS::Plugin::Request::Download

#endif // PLUGINS_REQUEST_DOWNLOAD_PROGRESS_NOTIFY_H