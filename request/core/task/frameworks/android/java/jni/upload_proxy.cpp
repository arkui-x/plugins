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
#include <sstream>

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
    auto uploadThread = std::make_unique<std::thread>(&UploadProxy::Exec, shared_from_this());
    uploadThread->detach();
    return true;
}

bool UploadProxy::Remove()
{
    REQUEST_HILOGI("upload remove");
    std::lock_guard<std::mutex> guard(mutex_);
    ChangeState(State::REMOVED);
    isAbort_ = true;
    Notify(EVENT_REMOVE);
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

bool UploadProxy::Pause()
{
    REQUEST_HILOGI("upload Pause");
    std::lock_guard<std::mutex> guard(mutex_);
    if (info_.progress.state == State::RUNNING) {
        REQUEST_HILOGI("upload Pause currentCurl_:%0xp", currentCurl_);
        CURLcode pauseRes = curl_easy_pause(currentCurl_, CURLPAUSE_RECV | CURLPAUSE_SEND);
        if (pauseRes != CURLE_OK) {
            REQUEST_HILOGI("Pause curl_easy_strerror handle: %{public}s", curl_easy_strerror(pauseRes));
        } else {
            REQUEST_HILOGI("upload Paused at position: %lld", info_.progress.processed);
            ChangeState(State::PAUSED);
            isPause_ = true;
            Notify(EVENT_PAUSE);
            return true;
        }
    } else {
        REQUEST_HILOGI("Upload pause failed.");
    }

    return false;
}

bool UploadProxy::Resume()
{
    REQUEST_HILOGI("upload Resume");
    std::lock_guard<std::mutex> guard(mutex_);
    if (info_.progress.state == State::PAUSED) {
        REQUEST_HILOGI("upload Pause currentCurl_:%0xp", currentCurl_);
        if (currentCurl_ != nullptr) {
            CURLcode resumeRes = curl_easy_pause(currentCurl_, CURLPAUSE_CONT);
            if (resumeRes != CURLE_OK) {
                REQUEST_HILOGI("Resume curl_easy_strerror, handle: %{public}s", curl_easy_strerror(resumeRes));
            } else {
                REQUEST_HILOGI("upload Resume at position: %lld", info_.progress.processed);
                ChangeState(State::RUNNING);
                Notify(EVENT_RESUME);
                auto uploadThread = std::make_unique<std::thread>(&UploadProxy::Exec, shared_from_this());
                uploadThread->detach();
                return true;
            }
        }
    } else {
        REQUEST_HILOGI("Upload Resume failed.");
    }

    return false;
}

void UploadProxy::Exec()
{
    uint32_t ret = ExecInner();
    REQUEST_HILOGI("ExecInner: %{public}d", ret);
    auto newState = ret == E_OK ? State::COMPLETED :  State::FAILED;
    if (info_.progress.state != State::STOPPED && info_.progress.state != State::PAUSED) {
        ChangeState(newState);
    }
}

uint32_t UploadProxy::ExecInner()
{
    ChangeState(State::RUNNING);
    REQUEST_HILOGI("ExecInner isPause_: %{public}d", isPause_);
    if (!isPause_) {
        for (size_t i = 0; i < config_.files.size(); i++) {
            info_.progress.extras.clear();
            info_.progress.bodyBytes.clear();
            info_.progress.processed = 0;
            info_.progress.index = i;
            if (UploadOneFile(info_.progress.index, config_.files[i]) != E_OK) {
                return E_SERVICE_ERROR;
            }
            ReportInfo(false);
            if (isPause_) {
                return E_OK;
            }
        }
    } else {
        if (ResumeUploadOneFile(info_.progress.index) != E_OK) {
            return E_SERVICE_ERROR;
        }
        ReportInfo(false);
        if (isPause_) {
            return E_OK;
        }
        if (info_.progress.index < config_.files.size() - 1) {
            for (size_t i = info_.progress.index + 1; i < config_.files.size(); i++) {
                info_.progress.extras.clear();
                info_.progress.bodyBytes.clear();
                info_.progress.processed = 0;
                info_.progress.index = i;
                if (UploadOneFile(info_.progress.index, config_.files[i]) != E_OK) {
                    return E_SERVICE_ERROR;
                }
                ReportInfo(false);
                if (isPause_) {
                    return E_OK;
                }
            }
        }
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
        if (file.filename.empty()) {
            state.responseCode = E_FILE_IO;
        }
        if (file.fd > 0) {
            const int64_t fileTotalSize = lseek(file.fd, 0, SEEK_END);
            int64_t fileSize = fileTotalSize;
            int64_t offset = 0;
            if (i == config.index) {
                int64_t beginPos = config.begins;
                int64_t endPos = config.ends;
                if (config.begins >= fileTotalSize) {
                    beginPos = 0;
                }
                if (config.ends >= fileTotalSize) {
                    endPos = -1;
                }
                if (config.begins > config.ends && config.ends >= 0) {
                    beginPos = 0;
                    endPos = -1;
                }
                if (endPos == -1) {
                    fileSize = fileTotalSize - beginPos;
                    offset = beginPos;
                } else {
                    fileSize = endPos - beginPos + 1;
                    offset = beginPos;
                }
            } else {
                REQUEST_HILOGI("Full upload[%d]: size=%lld", i, fileSize);
            }
            info.progress.sizes.emplace_back(fileSize);
            lseek(file.fd, offset, SEEK_SET);
        } else {
            info.progress.sizes.emplace_back(0);
            if (state.responseCode == E_OK) {
                state.responseCode = E_FILE_IO;
            }
        }
        state.message = GetCodeMessage(state.responseCode);
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
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWritingMemoryBody);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
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
    std::string proxy = config.proxy;
    if (!proxy.empty()) {
        if (curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str()) != CURLE_OK) {
            REQUEST_HILOGE("Failed to set proxy: %s", proxy.c_str());
        }
    }
    SetNetworkOpt(curl, config.url);
    SetConnectionOpt(curl);
    SetSslOpt(curl, config.url);
    SetBehaviorOpt(curl);
    SetCallbackOpt(curl);
}

void UploadProxy::SetMimePost(CURL *curl, curl_mime *mime, const FileSpec &file, int64_t fileSize)
{
    for (const auto& form : config_.forms) {
        curl_mimepart *formPart = curl_mime_addpart(mime);
        curl_mime_name(formPart, form.name.c_str());
        curl_mime_data(formPart, form.value.c_str(), form.value.size());
        REQUEST_HILOGI("Added form field: %{public}s=%{public}s",
            form.name.c_str(), form.value.c_str());
    }

    curl_mimepart *filePart = curl_mime_addpart(mime);
    curl_mime_name(filePart, file.name.empty() ? "file" : file.name.c_str());
    curl_mime_type(filePart, file.type.c_str());
    curl_mime_filename(filePart, file.filename.c_str());
    curl_mime_data_cb(filePart, fileSize, ReadCallback, nullptr, nullptr, this);

    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    REQUEST_HILOGI("Added file: %{public}s, type: %{public}s, size: %{public}lld)",
        file.filename.c_str(), file.type.c_str(), fileSize);
}

void UploadProxy::SetHttpPut(CURL *curl, const FileSpec &file, int64_t fileSize)
{
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, ReadCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, this);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE, fileSize);
}


int32_t UploadProxy::ResumeUploadOneFile(uint32_t index)
{
    std::lock_guard<std::mutex> guard(curlMutex_);
    int isRuning = 0;
    int32_t errorCode = ExceptionErrorCode::E_OK;
    do {
        if (curlMulti_ == nullptr || currentCurl_ == nullptr) {
            errorCode = ExceptionErrorCode::E_SERVICE_ERROR;
            break;
        }
        isPause_ = false;
        curl_multi_perform(curlMulti_, &isRuning);
        do {
            int numfds = 0;
            if (curl_multi_wait(curlMulti_, nullptr, 0, TRANS_TIMEOUT_MS, &numfds) != CURLM_OK) {
                errorCode = ExceptionErrorCode::E_SERVICE_ERROR;
                break;
            }
            curl_multi_perform(curlMulti_, &isRuning);
        } while (isRuning && !isPause_ && !isAbort_);
        if (!isRuning) {
            errorCode = CheckUploadStatus(curlMulti_);
            info_.taskStates[index].responseCode = errorCode;
            info_.taskStates[index].message = GetCodeMessage(errorCode);
        }
    } while (0);

    if (!isRuning) {
        ClearCurlResource(curlMulti_, currentCurl_, currentMime_);
        currentCurl_ = nullptr;
        currentMime_ = nullptr;
        curlMulti_ = nullptr;
    }

    return errorCode;
}


int32_t UploadProxy::UploadOneFile(uint32_t index, const FileSpec &file)
{
    int32_t errorCode = ExceptionErrorCode::E_OK;
    std::lock_guard<std::mutex> guard(curlMutex_);

    CURLM *curlMulti = curl_multi_init();
    CURL *curl = curl_easy_init();
    curl_mime *mime = nullptr;
    currentCurl_ = curl;
    currentMime_ = mime;
    curlMulti_ = curlMulti;
    int isRuning = 0;
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
            currentMime_ = mime;
        }
        curl_multi_add_handle(curlMulti, curl);
        curl_multi_perform(curlMulti, &isRuning);
        do {
            int numfds = 0;
            if (curl_multi_wait(curlMulti, nullptr, 0, TRANS_TIMEOUT_MS, &numfds) != CURLM_OK) {
                errorCode = ExceptionErrorCode::E_SERVICE_ERROR;
                break;
            }
            curl_multi_perform(curlMulti, &isRuning);
        } while (isRuning && !isPause_ && !isAbort_);
        REQUEST_HILOGI("isRuning = %{public}d", isRuning);
        if (!isRuning) {
            errorCode = CheckUploadStatus(curlMulti);
            info_.taskStates[index].responseCode = errorCode;
            info_.taskStates[index].message = GetCodeMessage(errorCode);
        }
    } while (0);
    if (!isRuning) {
        ClearCurlResource(curlMulti, curl, mime);
        currentCurl_ = nullptr;
        currentMime_ = nullptr;
        curlMulti_ = nullptr;
    }

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
        if (respCode < HTTP_SUCCESS_MIN || respCode > HTTP_SUCCESS_MAX) {
            REQUEST_HILOGE("upload fail http error %{public}d", respCode);
            info_.progress.processed = 0;
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
    uint32_t index = thiz->info_.progress.index;
    double progress = static_cast<double>(thiz->info_.progress.processed) / thiz->info_.progress.sizes[index] * 100;
    if (now - thiz->currentTime_ >= REPORT_INFO_INTERVAL) {
        REQUEST_HILOGI("Upload Progress: %.2f%% (%lld/%lld bytes)",
            progress, thiz->info_.progress.processed, thiz->info_.progress.sizes[index]);
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

void UploadProxy::ParseHttpHeaders(const std::string& headers)
{
    std::istringstream stream(headers);
    std::string line;

    if (std::getline(stream, line)) {
        std::istringstream statusLine(line);
        statusLine >> response_.version >> response_.statusCode;
        std::getline(statusLine >> std::ws, response_.reason);
        if (!response_.reason.empty() && response_.reason.back() == '\r') {
            response_.reason.pop_back();
        }
    }

    while (std::getline(stream, line) && !line.empty()) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        const size_t colon = line.find(':');
        if (colon == std::string::npos) {
            continue;
        }
        auto key = line.substr(0, colon);
        auto value = line.substr(colon + 1);

        key.erase(0, key.find_first_not_of(" "));
        key.erase(key.find_last_not_of(" ") + 1);
        value.erase(0, value.find_first_not_of(" "));
        value.erase(value.find_last_not_of(" ") + 1);

        response_.headers[key].push_back(value);
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
        std::string headers = thiz->responseHead_.substr(0, pos);
        if (!headers.empty()) {
            thiz->ParseHttpHeaders(headers);
            thiz->info_.response = thiz->response_;
            thiz->Notify(EVENT_RESPONSE);
        } else {
            REQUEST_HILOGE("Empty header received");
        }
        thiz->responseHead_.erase(0, pos + headEndFlag.length());
        REQUEST_HILOGI("Remaining buffer: %zu bytes", thiz->responseHead_.size());
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

size_t UploadProxy::OnWritingMemoryBody(const void *data, size_t size, size_t memBytes, void *userData)
{
    auto thiz = static_cast<UploadProxy*>(userData);
    if (thiz == nullptr || thiz->isAbort_ || data == nullptr) {
        REQUEST_HILOGE("OnWritingMemoryBody null");
        return CURL_READFUNC_ABORT;
    }
    std::string tempString;
    tempString.append(reinterpret_cast<const char *>(data), size * memBytes);

    thiz->info_.progress.bodyBytes.clear();
    thiz->info_.progress.bodyBytes.resize(tempString.length());
    std::copy(tempString.begin(), tempString.end(), thiz->info_.progress.bodyBytes.begin());

    thiz->Notify(EVENT_HEADERRECEIVE);
    return size * memBytes;
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
