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

#include "android_download_adp_impl.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <jni.h>
#include <chrono>

#include "inner_api/plugin_utils_inner.h"
#include "java/jni/download_manager_jni.h"
#include "log.h"
#include "plugin_utils.h"

namespace {
constexpr int FILE_NOT_EXIST_NUM = -1;
constexpr int WAIT_TIME = 100;
}  // namespace

namespace OHOS::Plugin::Request::Download {
std::shared_ptr<AndroidDownloadAdp> AndroidDownloadAdp::Instance()
{
    return std::make_shared<AndroidDownloadAdpImpl>();
}

bool AndroidDownloadAdp::IsPathValid(const std::string &filePath)
{
    auto path = filePath.substr(0, filePath.rfind('/'));
    char resolvedPath[PATH_MAX + 1] = { 0 };
    if (path.length() > PATH_MAX || realpath(path.c_str(), resolvedPath) == nullptr ||
        std::string::npos != path.find("../") || std::string::npos != path.find("/..")) {
        DOWNLOAD_HILOGE("filePath is invalid, filePath = %s", filePath.c_str());
        return false;
    }
    return true;
}

bool AndroidDownloadAdp::IsDirectory(const std::string &path)
{
    DOWNLOAD_HILOGI("AndroidDownloadAdpImpl IsDirectory called, get file path: %{private}s", path.c_str());
    if (!IsPathValid(path)) {
        return false;
    }
    struct stat statBuffer;
    if (access(path.c_str(), 0) != FILE_NOT_EXIST_NUM &&
        stat(path.c_str(), &statBuffer) != FILE_NOT_EXIST_NUM &&
        S_ISDIR(statBuffer.st_mode)) {
        DOWNLOAD_HILOGI("path: %{private}s is exist and valid", path.c_str());
        return true;
    }

    DOWNLOAD_HILOGE("path: %{private}s is not exist or invalid", path.c_str());
    return false;
}

AndroidDownloadAdpImpl::AndroidDownloadAdpImpl()
{
    DOWNLOAD_HILOGD("AndroidDownloadAdpImpl init");
}

AndroidDownloadAdpImpl::~AndroidDownloadAdpImpl()
{
    DOWNLOAD_HILOGD("AndroidDownloadAdpImpl free");
}

void AndroidDownloadAdpImpl::Download(const DownloadConfig &config, void *downloadProgress)
{
    DOWNLOAD_HILOGI("AndroidDownloadAdpImpl Download called");
    CHECK_NULL_VOID(downloadProgress);
    PluginUtilsInner::RunTaskOnPlatform([&, downloadProgress]() {
        DownloadManagerJni::Download(config, downloadProgress);
    });
}

void AndroidDownloadAdpImpl::Suspend()
{
    DOWNLOAD_HILOGI("AndroidDownloadAdpImpl Suspend called");
    PluginUtilsInner::RunTaskOnPlatform([]() { DownloadManagerJni::PauseDownload(); });
}

bool AndroidDownloadAdpImpl::Restore()
{
    DOWNLOAD_HILOGI("AndroidDownloadAdpImpl Restore called");
    return DownloadManagerJni::ResumeDownload();
}

void AndroidDownloadAdpImpl::Remove()
{
    DOWNLOAD_HILOGI("AndroidDownloadAdpImpl Remove called");
    PluginUtilsInner::RunTaskOnPlatform([]() { DownloadManagerJni::RemoveDownload(); });
}

void AndroidDownloadAdpImpl::GetNetworkType(void *network)
{
    DOWNLOAD_HILOGI("AndroidDownloadAdpImpl GetNetworkType called");
    std::chrono::milliseconds span(WAIT_TIME);
    PluginUtilsInner::RunSyncTaskOnLocal([network]() { DownloadManagerJni::GetNetworkType(network); }, span);
}
} // namespace OHOS::Plugin::Request::Download
