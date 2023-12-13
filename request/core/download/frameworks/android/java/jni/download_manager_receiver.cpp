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

#include "download_manager_receiver.h"

#include "log.h"
#include "android_download_task_impl.h"
#include "constant.h"

namespace OHOS::Plugin::Request::Download {
ProgressResultCallback DownloadManagerReceiver::progressResultCallback_ = nullptr;
MimeTypeCallback DownloadManagerReceiver::mimeTypeCallback_ = nullptr;
NetworkTypeCallback DownloadManagerReceiver::networkTypeCallback_ = nullptr;
void DownloadManagerReceiver::JSRegisterProgressResult(ProgressResultCallback callback)
{
    progressResultCallback_ = callback;
}

void DownloadManagerReceiver::JSRegisterMimeType(MimeTypeCallback callback)
{
    mimeTypeCallback_ = callback;
}

void DownloadManagerReceiver::JSRegisterNetworkType(NetworkTypeCallback callback)
{
    networkTypeCallback_ = callback;
}

void DownloadManagerReceiver::OnRequestDataCallback(const std::vector<int> &progressResult)
{
    DOWNLOAD_HILOGI("DownloadManagerReceiver: OnRequestDataCallback");
    if (progressResultCallback_) {
        progressResultCallback_(progressResult);
        progressResultCallback_ = nullptr;
    }
    DOWNLOAD_HILOGI("receive receivedSize: %d, totalSize: %d", progressResult[0],  progressResult[1]);
}

void DownloadManagerReceiver::OnRequestMimeTypeCallback(const std::string &mimeType)
{
    DOWNLOAD_HILOGI("DownloadManagerReceiver: OnRequestMimeTypeCallback");
    if (mimeTypeCallback_) {
        mimeTypeCallback_(mimeType);
        mimeTypeCallback_ = nullptr;
    }
    DOWNLOAD_HILOGI("receive mimeType: %s", mimeType.c_str());
}

void DownloadManagerReceiver::OnRequestNetworkTypeCallback(NetworkType networkType)
{
    DOWNLOAD_HILOGI("DownloadManagerReceiver: OnRequestNetworkTypeCallback");
    if (networkTypeCallback_) {
        networkTypeCallback_(networkType);
        networkTypeCallback_ = nullptr;
    }
    DOWNLOAD_HILOGI("receive networkType: %d", networkType);
}
} // namespace OHOS::Plugin::Request::Download