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

#ifndef PLUGINS_DOWNLOADMANAGER_ANDROID_JAVA_JNI_DOWNLOADMANAGER_RECEIVER_H
#define PLUGINS_DOWNLOADMANAGER_ANDROID_JAVA_JNI_DOWNLOADMANAGER_RECEIVER_H

#include <functional>
#include <string>
#include <vector>
#include "constant.h"

namespace OHOS::Plugin::Request::Download {
using ProgressResultCallback = std::function<void(const std::vector<int> &progressResult)>;
using MimeTypeCallback = std::function<void(const std::string &mimeType)>;
using NetworkTypeCallback = std::function<void(NetworkType networkType)>;

class DownloadManagerReceiver final {
public:
    DownloadManagerReceiver() = default;
    ~DownloadManagerReceiver() = default;

    static void JSRegisterProgressResult(ProgressResultCallback callback);
    static void JSRegisterMimeType(MimeTypeCallback callback);
    static void JSRegisterNetworkType(NetworkTypeCallback callback);
    static void OnRequestDataCallback(const std::vector<int> &progressResult);
    static void OnRequestMimeTypeCallback(const std::string &mimeType);
    static void OnRequestNetworkTypeCallback(NetworkType networkType);

private:
    static ProgressResultCallback progressResultCallback_;
    static MimeTypeCallback mimeTypeCallback_;
    static NetworkTypeCallback networkTypeCallback_;
};
} // namespace OHOS::Plugin::Request::Download
#endif // PLUGINS_DOWNLOADMANAGER_ANDROID_JAVA_JNI_DOWNLOADMANAGER_RECEIVER_H