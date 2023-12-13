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

#include "download_task_plugin_impl.h"

#include "log.h"

namespace OHOS::Plugin::Request::Download {
DownloadTaskPluginImpl::DownloadTaskPluginImpl(uint32_t taskId, const DownloadConfig &config)
    : IDownloadTask(taskId, config) {
}

DownloadTaskPluginImpl::~DownloadTaskPluginImpl(void)
{
    DOWNLOAD_HILOGD("Destructed download service task [%{public}d]", GetId());
}

void DownloadTaskPluginImpl::ExecuteTask()
{
    DOWNLOAD_HILOGD("ExecuteTask.");
}

bool DownloadTaskPluginImpl::Remove()
{
    DOWNLOAD_HILOGI("Remove Task[%{public}d], ",  GetId());

    if (eventCb_ != nullptr) {
        eventCb_("remove", GetId(), 0, 0);
    }
    return true;
}

IDownloadTask* IDownloadTask::CreateDownloadTask(uint32_t taskId, const DownloadConfig &config)
{
    return new (std::nothrow) DownloadTaskPluginImpl(taskId, config);
}
} // namespace OHOS::Plugin::Request::Download
