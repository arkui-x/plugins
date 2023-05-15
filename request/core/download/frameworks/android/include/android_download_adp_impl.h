/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_REQUEST_DOWNLOAD_ANDROID_DOWNLOAD_ADP_IMPL_H
#define PLUGINS_REQUEST_DOWNLOAD_ANDROID_DOWNLOAD_ADP_IMPL_H

#include "android_download_adp.h"
#include "i_download_task.h"

namespace OHOS::Plugin::Request::Download {

class AndroidDownloadAdpImpl final : public AndroidDownloadAdp {
public:
    AndroidDownloadAdpImpl();
    ~AndroidDownloadAdpImpl();
    void Download(const DownloadConfig &config, void *downloadProgress) override;
    void Remove() override;
    void Suspend() override;
    bool Restore() override;
    void GetNetworkType(void *network) override;

private:
    std::string downloadUrl_;
    std::string fileName_;
};

} // namespace OHOS::Plugin::Request::Download

#endif // PLUGINS_REQUEST_DOWNLOAD_ANDROID_DOWNLOAD_ADP_IMPL_H