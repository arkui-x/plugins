/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "upload_proxy.h"
#include <sys/stat.h>
#include <unistd.h>
#include <thread>
#include <utility>

#include "constant.h"
#include "log.h"
#include "request_utils.h"
#include "task_info_json.h"
#include "task_manager_jni.h"

namespace OHOS::Plugin::Request {
static std::string GetCodeMessage(uint32_t code)
{
    std::vector<std::pair<ExceptionErrorCode, std::string>> codeMap = {
        {E_OK, "file uploaded successfully"},
        {E_FILE_PATH, "file path error"},
        {E_FILE_IO, "failed to get file"},
        {E_SERVICE_ERROR,  "upload failed"},
    };

    for (const auto &it : codeMap) {
        if (static_cast<uint32_t>(it.first) == code) {
            return it.second;
        }
    }
    return "unknown";
}

UploadProxy::UploadProxy(int64_t taskId, const Config &config) : taskId_(taskId), config_(config)
{
    BuildHeaderData(config_);
    InitTaskInfo(config_, info_);
}

UploadProxy::~UploadProxy()
{
    if (list_ != nullptr) {
        curl_slist_free_all(list_);
        list_ = nullptr;
    }
}

bool UploadProxy::Start(UploadCallback callback)
{
    REQUEST_HILOGI("upload start");
    std::lock_guard<std::mutex> guard(mutex_);
    callback_ = callback;
    auto uploadThread = std::make_unique<std::thread>(&UploadProxy::Exec, this);
    uploadThread->detach();
    return true;
}

bool UploadProxy::Remove()
{
    REQUEST_HILOGI("upload remove");
    std::lock_guard<std::mutex> guard(mutex_);
    ChangeState(State::REMOVED);
    isAbort_ = true;
    return true;
}

bool UploadProxy::Stop()
{
    REQUEST_HILOGI("upload stop");
    std::lock_guard<std::mutex> guard(mutex_);
    ChangeState(State::STOPPED);
    isAbort_ = true;
    isStopped_ = true;
    return true;
}

void UploadProxy::Exec()
{
    uint32_t ret = ExecInner();
    REQUEST_HILOGI("ExecInner: %{public}d", ret);
    auto newState = ret == E_OK ? State::COMPLETED :  State::FAILED;
    if (info_.progress.state != State::STOPPED) {
        ChangeState(newState);
    }
}

uint32_t UploadProxy::ExecInner()
{
    ChangeState(State::RUNNING);
    for (size_t i = 0; i < config_.files.size(); i++) {
        REQUEST_HILOGI("upload file: %{public}s", config_.files[i].filename.c_str());
        info_.progress.extras.clear();
        info_.progress.bodyBytes.clear();
        info_.progress.processed = 0;
        info_.progress.index = i;
        if (UploadOneFile(info_.progress.index, config_.files[i]) != E_OK) {
            return E_SERVICE_ERROR;
        }
        ReportInfo(false);
    }
    REQUEST_HILOGI("upload end");
    return E_OK;
}

void UploadProxy::BuildHeaderData(const Config &config)
{
    if (config.headers.empty()) {
        REQUEST_HILOGI("no header data for upload");
        return;
    }
    std::vector<std::string> headVec;
    std::for_each(config.headers.begin(), config.headers.end(),
        [&headVec](const std::pair<std::string, std::string> &header) {
            headVec.emplace_back(header.first + ":" + header.second);
        });
    bool hasContentType = false;
    for (auto &headerData : headVec) {
        if (headerData.find("Content-Type:") != std::string::npos) {
            hasContentType = true;
        }
        list_ = curl_slist_append(list_, headerData.c_str());
    }

    if (!hasContentType) {
        std::string str = config.method == METHOD_PUT ? "Content-Type:application/octet-stream"
                                                      : "Content-Type:multipart/form-data";
        list_ = curl_slist_append(list_, str.c_str());
    }
}

void UploadProxy::InitTaskInfo(const Config &config, TaskInfo &info)
{
    info.version = config.version;
    info.url = config.url;
    info.data = config.data;
    info.files = config.files;
    info.forms = config.forms;
    info.tid = std::to_string(taskId_);
    info.title = config.title;
    info.description = config.description;
    info.action = config.action;
    info.mode = config.mode;
    info.mimeType = "";
    info.gauge = config.gauge;
    info.ctime = RequestUtils::GetTimeNow();
    info.mtime = RequestUtils::GetTimeNow();
    info.retry = false;
    info.tries = 0;
    info.faults = Faults::OTHERS;
    info.code = Reason::REASON_OK;
    info.reason = "";
    info.withSystem = false;
    info.priority = 0;
    info.extras = config.extras;
    info.progress.index = config.index;
    info.progress.processed = 0;
    info.progress.totalProcessed = 0;
    for(auto i = 0; i < config.files.size(); i++){
        auto file = config.files[i];
        TaskState state;
        state.path = file.filename;
        state.responseCode = E_OK;
        if (file.uri.empty()) {
            state.responseCode = E_FILE_PATH;
        }
        if (file.filename.empty() || file.fd < 0) {
            state.responseCode = E_FILE_IO;
        }
        state.message = GetCodeMessage(state.responseCode);
        if (file.fd > 0) {
            int64_t fileSize = lseek(file.fd, 0, SEEK_END);
            if (i == config.index && config.begins >= 0 && config.ends >= 0) {
                REQUEST_HILOGI("partial upload: %{public}u, begins %{public}lld, ends %{public}lld",
                config.index, config.begins, config.ends);
                fileSize = config.ends - config.begins + 1;
            }
            info.progress.sizes.emplace_back(fileSize);
            int64_t offset = 0;
            if (config.begins > 0) {
                offset = config.begins;
            }
            lseek(file.fd, offset, SEEK_SET);
        }
        info.taskStates.emplace_back(state);
    }
    ChangeState(State::INITIALIZED);
}

void UploadProxy::ChangeState(State state)
{
    REQUEST_HILOGI("current state: %{public}d, isStopped_: %{public}d", static_cast<int32_t>(state), isStopped_);
    info_.progress.state = state;
    info_.mtime = RequestUtils::GetTimeNow();
    ReportInfo(true);
    if (state == State::FAILED) {
        if (!isStopped_) {
            Notify(EVENT_FAILED);
        } 
    } else if (state == State::COMPLETED) {
        Notify(EVENT_PROGRESS);
        Notify(EVENT_COMPLETED);
    }
}

void UploadProxy::Notify(const std::string &type)
{   
    if (callback_ != nullptr) {
        Json infoJson = info_;
        callback_(taskId_, type, infoJson.dump());
    }
}

void UploadProxy::SetBehaviorOpt(CURL *curl)
{
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
}

void UploadProxy::SetCallbackOpt(CURL *curl)
{
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
}

void UploadProxy::SetNetworkOpt(CURL *curl, const std::string &url)
{
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
}

void UploadProxy::SetConnectionOpt(CURL *curl)
{
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
}

void UploadProxy::SetSslOpt(CURL *curl, const std::string &url)
{
    if (url.compare("https") != 0) {
        REQUEST_HILOGI("it is http upload");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        return;
    }

    REQUEST_HILOGI("it is https upload");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_CAINFO, HTTP_DEFAULT_CA_PATH);
}

void UploadProxy::SetCurlOpt(CURL *curl, const Config &config)
{
    if (list_ != nullptr) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list_);
    }
    SetNetworkOpt(curl, config.url);
    SetConnectionOpt(curl);
    SetSslOpt(curl, config.url);
    SetBehaviorOpt(curl);
    SetCallbackOpt(curl);
}

void UploadProxy::SetMimePost(CURL *curl, curl_mime *mime, const FileSpec &file, int64_t fileSize)
{
    curl_mimepart *part = curl_mime_addpart(mime);
    if (!config_.forms.empty()) {
        REQUEST_HILOGI("the size of config_.forms: %{public}zu", config_.forms.size());
        curl_mime_name(part, config_.forms[0].name.c_str());
        curl_mime_data(part, config_.forms[0].value.c_str(), config_.forms[0].value.size());
        config_.forms.erase(config_.forms.begin());
    } else {
        curl_mime_name(part, "file");
    }
    curl_mime_type(part, file.type.c_str());
    curl_mime_filename(part, file.filename.c_str());
    curl_mime_data_cb(part, fileSize, ReadCallback, nullptr, nullptr, this);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
}

void UploadProxy::SetHttpPut(CURL *curl, const FileSpec &file, int64_t fileSize)
{
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, ReadCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, this);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE, fileSize);
}

int32_t UploadProxy::UploadOneFile(uint32_t index, const FileSpec &file)
{
    int32_t errorCode = ExceptionErrorCode::E_OK;
    std::lock_guard<std::mutex> guard(curlMutex_);

    CURLM *curlMulti = curl_multi_init();
    CURL *curl = curl_easy_init();
    curl_mime *mime = nullptr;
    do {
        if (curlMulti == nullptr || curl == nullptr) {
            errorCode = ExceptionErrorCode::E_SERVICE_ERROR;
            break;
        }
        SetCurlOpt(curl, config_);
        if (config_.method == METHOD_PUT) {
            SetHttpPut(curl, file, info_.progress.sizes[index]);
        } else {
            mime = curl_mime_init(curl);
            SetMimePost(curl, mime, file, info_.progress.sizes[index]);
        }
        curl_multi_add_handle(curlMulti, curl);
        int isRuning = 0;
        curl_multi_perform(curlMulti, &isRuning);
        do {
            int numfds = 0;
            if (curl_multi_wait(curlMulti, nullptr, 0, TRANS_TIMEOUT_MS, &numfds) != CURLM_OK) {
                errorCode = ExceptionErrorCode::E_SERVICE_ERROR;
                break;
            }
            curl_multi_perform(curlMulti, &isRuning);
        } while (isRuning);
        REQUEST_HILOGI("isRuning = %{public}d", isRuning);
        if (!isRuning) {
            errorCode = CheckUploadStatus(curlMulti);
            info_.taskStates[index].responseCode = errorCode;
            info_.taskStates[index].message = GetCodeMessage(errorCode);
        }
    } while (0);
    ClearCurlResource(curlMulti, curl, mime);
    return errorCode;
}

int32_t UploadProxy::CheckUploadStatus(CURLM *curlMulti)
{
    int msgsLeft = 0;
    CURLMsg *msg = curl_multi_info_read(curlMulti, &msgsLeft);
    while (msg != nullptr) {
        std::lock_guard<std::mutex> guard(responseMutex_);
        if (msg->msg != CURLMSG_DONE) {
            continue;
        }
        if (msg->data.result != CURLE_OK) {
            REQUEST_HILOGE("upload fail curl error %{public}d", msg->data.result);
            info_.progress.processed = 0;
            return E_SERVICE_ERROR;
        }

        int32_t respCode = 0;
        curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &respCode);
        REQUEST_HILOGI("upload http code: %{public}d", respCode);
        if (respCode != HTTP_SUCCESS) {
            REQUEST_HILOGE("upload fail http error %{public}d", respCode);
             if (respCode != HTTP_PARTIAL_SUCCESS) {
                info_.progress.processed = 0;
            }
            return E_SERVICE_ERROR;
        }
        msg = curl_multi_info_read(curlMulti, &msgsLeft);
    }
    return E_OK;
}

void UploadProxy::ClearCurlResource(CURLM *curlMulti, CURL *curl, curl_mime *mime)
{
    if (mime != nullptr) {
        curl_mime_free(mime);
        mime = nullptr;
    }

    if (curl != nullptr) {
        curl_multi_remove_handle(curlMulti, curl);
        curl_easy_cleanup(curl);
        curl = nullptr;
    }

    if (curlMulti != nullptr) {
        curl_multi_cleanup(curlMulti);
        curlMulti = nullptr;
    }
}

int32_t UploadProxy::ProgressCallback(void *client, curl_off_t dltotal, curl_off_t dlnow,
    curl_off_t ultotal, curl_off_t ulnow)
{
    auto thiz = static_cast<UploadProxy*>(client);
    if (thiz == nullptr) {
        return 0;
    }
    if (thiz->isAbort_) {
        REQUEST_HILOGI("upload task has been removed");
        return HTTP_FORCE_STOP;
    }   
    thiz->ReportInfo(false);
    int64_t now = RequestUtils::GetTimeNow();
    if (now - thiz->currentTime_ >= REPORT_INFO_INTERVAL) {
        thiz->Notify(EVENT_PROGRESS);
        thiz->currentTime_ = now;
    }    
    return 0;
}

void UploadProxy::SplitHttpMessage(const std::string &header)
{
    std::string lineEndFlag = HTTP_LINE_SEPARATOR;
    auto temp = header.substr(header.find(lineEndFlag) + lineEndFlag.size());
    while (temp != "") {
        auto pos = temp.find(lineEndFlag);
        auto line = temp.substr(0, pos);
        REQUEST_HILOGI("line = %{public}s", line.c_str());
        auto keyPos = line.find(HTTP_HEADER_SEPARATOR);
        if (keyPos != std::string::npos) {
            info_.progress.extras.emplace(line.substr(0, keyPos), line.substr(keyPos + 2));
        }
        temp = temp.substr(temp.find(lineEndFlag) + lineEndFlag.size());
    }
}

size_t UploadProxy::HeaderCallback(char *buffer, size_t size, size_t nitems, void *userdata)
{
    auto *thiz = static_cast<UploadProxy *>(userdata);
    if (thiz == nullptr || thiz->isAbort_) {
        return CURLE_WRITE_ERROR;
    }

    thiz->responseHead_ += std::string(buffer, size * nitems);
    const std::string headEndFlag = "\r\n\r\n";
    auto pos = thiz->responseHead_.find(headEndFlag);
    if (pos != std::string::npos) {
        thiz->SplitHttpMessage(thiz->responseHead_);
        thiz->Notify(EVENT_HEADERRECEIVE);
        thiz->responseHead_ = "";
    }
    return size * nitems;
}

size_t UploadProxy::ReadCallback(char *buffer, size_t size, size_t nitems, void *arg)
{
    auto thiz = static_cast<UploadProxy*>(arg);
    if (thiz == nullptr || thiz->isAbort_) {
        return CURL_READFUNC_ABORT;
    }

    uint32_t index = thiz->info_.progress.index;
    FileSpec file = thiz->config_.files[index];
    std::condition_variable condition;
    size_t readSize = 0;
    std::mutex mutexlock;
    std::unique_lock<std::mutex> lock(mutexlock);
    int64_t maxReadSize = thiz->info_.progress.sizes[index] - thiz->info_.progress.processed;
    if (maxReadSize > size * nitems) {
        maxReadSize = size * nitems;
    }
    auto readTask = [&] {
        std::unique_lock<std::mutex> readlock(mutexlock);
        readSize = read(file.fd, buffer, maxReadSize);
        thiz->info_.progress.processed += readSize;
        thiz->info_.progress.totalProcessed += readSize;
        condition.notify_one();
    };

    std::thread tRead(readTask);

    if (condition.wait_for(lock, std::chrono::milliseconds(READFILE_TIMEOUT_MS)) == std::cv_status::timeout) {
        REQUEST_HILOGI("Timeout error");
        thiz->isAbort_ = true;
    }
    tRead.join();

    return readSize;
}

void UploadProxy::ReportInfo(bool isChangeState)
{
    bool ignore = !isChangeState;
    int64_t now = RequestUtils::GetTimeNow();
    if (now - reportTime_ >= REPORT_INFO_INTERVAL) {
        ignore = false;
        reportTime_ = now;
    }
    if (!ignore) {
        TaskManagerJni::Get().ReportTaskInfo(info_);
    }
}
} // namespace  OHOS::Plugin::Request
