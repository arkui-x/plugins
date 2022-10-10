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

#ifndef PLUGINS_REQUEST_LIBPLATFORM_DOWNLOAD_TASK_IMPL_H
#define PLUGINS_REQUEST_LIBPLATFORM_DOWNLOAD_TASK_IMPL_H

#include <stddef.h>
#include <stdint.h>

#include "constant.h"
#include "download_config.h"
#include "download_task.h"

namespace OHOS::Plugin::Request::Download {
class DownloadTaskPluginImpl : public DownloadTask {
public:
    DownloadTaskPluginImpl(uint32_t taskId, const DownloadConfig &config);
    virtual ~DownloadTaskPluginImpl(void);

    void ExecuteTask() override;
    bool Remove() override;
    void InstallCallback(DownloadTaskCallback cb) override;

private:
    DownloadConfig config_;
    DownloadTaskCallback eventCb_;
};
} // namespace OHOS::Plugin::Request::Download
#endif // PLUGINS_REQUEST_LIBPLATFORM_DOWNLOAD_TASK_IMPL_H
