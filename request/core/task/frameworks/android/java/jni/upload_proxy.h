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

#ifndef PLUGINS_REQUEST_UPLOAD_PROXY_H
#define PLUGINS_REQUEST_UPLOAD_PROXY_H

#include <curl/curl.h>
#include <curl/easy.h>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "constant.h"
#include "i_upload_proxy.h"

namespace OHOS::Plugin::Request {
class UploadProxy : public IUploadProxy, public std::enable_shared_from_this<UploadProxy> {
public:
    UploadProxy(int64_t taskId, const Config &config);
    virtual ~UploadProxy();
    bool Start(UploadCallback callback) override;
    bool Remove() override;
    bool Stop() override;
    bool Pause() override;
    bool Resume() override;

private:
    void Exec();
    uint32_t ExecInner();

    void BuildHeaderData(const Config &config);
    void InitTaskInfo(const Config &config, TaskInfo &info);
    void ChangeState(State state);
    void Notify(const std::string &type);

    int32_t UploadOneFile(uint32_t index, const FileSpec &file);
    int32_t ResumeUploadOneFile(uint32_t index);
    int32_t CheckUploadStatus(CURLM *curlMulti);
    void ClearCurlResource(CURLM *curlMulti, CURL *curl, curl_mime *mime);
    void SplitHttpMessage(const std::string &stmp);
    static int ProgressCallback(void *clientp,
        curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
    static size_t OnWritingMemoryBody(const void *data, size_t size, size_t memBytes, void *userData);
    static size_t HeaderCallback(char *buffer, size_t size, size_t nitems, void *userdata);

    static size_t ReadCallback(char *buffer, size_t size, size_t nitems, void *arg);
    void ReportInfo(bool isChangeState);

    void SetSslOpt(CURL *curl, const std::string &url);
    void SetConnectionOpt(CURL *curl);
    void SetNetworkOpt(CURL *curl, const std::string &url);
    void SetCallbackOpt(CURL *curl);
    void SetBehaviorOpt(CURL *curl);
    void SetCurlOpt(CURL *curl, const Config &config);
    void SetHttpPut(CURL *curl, const FileSpec &file, int64_t fileSize);
    void SetMimePost(CURL *curl, curl_mime *mime, const FileSpec &file, int64_t fileSize);
    void ParseHttpHeaders(const std::string& headers);

private:
    int64_t taskId_ = 0;
    CURL* currentCurl_ = nullptr;
    CURLM *curlMulti_ = nullptr;
    curl_mime *currentMime_ = nullptr;
    Config config_;
    Response response_;
    UploadCallback callback_ = nullptr;
    TaskInfo info_ {};
    std::mutex mutex_;
    bool isAbort_ = false;
    bool isPause_ = false;
    bool isStopped_ = false;
    std::string responseHead_ = "";
    struct curl_slist *list_ = nullptr;
    int64_t reportTime_ = 0;
    int64_t currentTime_ = 0;
    std::mutex curlMutex_;
    std::mutex responseMutex_;
    static constexpr int32_t HTTP_SUCCESS = 200;
    static constexpr int32_t HTTP_SUCCESS_MIN = 200;
    static constexpr int32_t HTTP_SUCCESS_MAX  = 299;
    static constexpr int32_t HTTP_PARTIAL_SUCCESS = 206;
    static constexpr int32_t TRANS_TIMEOUT_MS = 30 * 1000;
    static constexpr int32_t READFILE_TIMEOUT_MS = 30 * 1000;
};
} // namespace OHOS::Plugin::Request
#endif // PLUGINS_REQUEST_UPLOAD_PROXY_H