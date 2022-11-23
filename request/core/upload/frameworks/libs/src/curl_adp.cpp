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

#include "curl_adp.h"

#include <cinttypes>
#include <climits>
#include <cstdio>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include "upload_hilog_wrapper.h"
#include "upload_task.h"

namespace OHOS::Plugin::Request::Upload {
const uint32_t HTTP_FORCE_STOP = 1;
CUrlAdp::CUrlAdp(std::vector<FileData> &fileDatas, std::shared_ptr<UploadConfig> &config) : fileDatas_(fileDatas)
{
    config_ = config;
    isCurlGlobalInit_ = false;
    isReadAbort_ = false;
    curlMulti_ = nullptr;
    timerId_ = 0;
}

CUrlAdp::~CUrlAdp()
{
}

uint32_t CUrlAdp::DoUpload(IUploadTask *task)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "upload start");
    uploadTask_ = task;

    uint32_t allFileUploadResult = UPLOAD_OK;
    for (auto &vmem : fileDatas_) {
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "read abort stat: %{public}d file index: %{public}u", IsReadAbort(),
            vmem.fileIndex);
        if (IsReadAbort()) {
            vmem.result = UPLOAD_ERRORCODE_UPLOAD_FAIL;
            allFileUploadResult = UPLOAD_ERRORCODE_UPLOAD_FAIL;
            continue;
        }

        mfileData_ = vmem;
        mfileData_.adp = this;
        vmem.result = static_cast<uint32_t>(UploadOneFile());
        if (vmem.result != UPLOAD_OK) {
            allFileUploadResult = UPLOAD_ERRORCODE_UPLOAD_FAIL;
        }
        mfileData_.responseHead.clear();
        if (mfileData_.list) {
            curl_slist_free_all(mfileData_.list);
            mfileData_.list = nullptr;
        }
        ClearCurlResource();
        usleep(FILE_UPLOAD_INTERVEL);
    }

    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "upload end");
    return allFileUploadResult;
}

bool CUrlAdp::MultiAddHandle(CURLM *curlMulti, std::vector<CURL *> &curlArray)
{
    CURL *curl = curl_easy_init();
    if (curl == nullptr) {
        return false;
    }

    SetCurlOpt(curl);
    curlArray.push_back(curl);
    curl_multi_add_handle(curlMulti, curl);
    return true;
}

void CUrlAdp::SetHeadData(CURL *curl)
{
    bool hasContentType = false;
    for (auto &headerData : config_->header) {
        if (headerData.find("Content-Type:") != std::string::npos) {
            hasContentType = true;
        }
        mfileData_.list = curl_slist_append(mfileData_.list, headerData.c_str());
    }

    if (!hasContentType) {
        std::string str = config_->method == PUT ? "Content-Type:application/octet-stream"
                                                 : "Content-Type:multipart/form-data";
        mfileData_.list = curl_slist_append(mfileData_.list, str.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, mfileData_.list);
}

void CUrlAdp::SetBehaviorOpt(CURL *curl)
{
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
}

void CUrlAdp::SetCallbackOpt(CURL *curl)
{
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &mfileData_);
    if (config_->protocolVersion == "L5") {
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallbackL5);
    } else {
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ProgressCallback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &mfileData_);
    }
}

void CUrlAdp::SetNetworkOpt(CURL *curl)
{
    curl_easy_setopt(curl, CURLOPT_URL, config_->url.c_str());
}

void CUrlAdp::SetConnectionOpt(CURL *curl)
{
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
}

void CUrlAdp::SetSslOpt(CURL *curl)
{
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
}

void CUrlAdp::SetCurlOpt(CURL *curl)
{
    SetHeadData(curl);
    SetNetworkOpt(curl);
    SetConnectionOpt(curl);
    SetSslOpt(curl);
    SetBehaviorOpt(curl);
    SetCallbackOpt(curl);
    if (config_->method == PUT) {
        SetHttpPut(curl);
    } else {
        SetMimePost(curl);
    }
}

void CUrlAdp::SetMimePost(CURL *curl)
{
    curl_mimepart *part;
    if (mime_) {
        curl_mime_free(mime_);
        mime_ = nullptr;
    }
    mime_ = curl_mime_init(curl);
    if (config_->data.size()) {
        for (auto &vdata : config_->data) {
            part = curl_mime_addpart(mime_);
            curl_mime_name(part, vdata.name.c_str());
            curl_mime_data(part, vdata.value.c_str(), vdata.value.size());
        }
    }
    part = curl_mime_addpart(mime_);
    if (mfileData_.name.size()) {
        curl_mime_name(part, mfileData_.name.c_str());
    } else {
        curl_mime_name(part, "file");
    }
    curl_mime_type(part, mfileData_.type.c_str());
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "===> MultiAddHandle mfileData_.type=%{public}s", mfileData_.type.c_str());
    curl_mime_filename(part, mfileData_.filename.c_str());
    curl_mime_data_cb(part, mfileData_.totalsize, ReadCallback, NULL, NULL, &mfileData_);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime_);
}

void CUrlAdp::SetHttpPut(CURL *curl)
{
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, ReadCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &mfileData_);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE, mfileData_.totalsize);
}

int32_t CUrlAdp::UploadOneFile()
{
    std::string traceParam = "name:" + mfileData_.filename + "index" + std::to_string(mfileData_.fileIndex) +
                             "size:" + std::to_string(mfileData_.totalsize);

    CurlGlobalInit();
    curlMulti_ = curl_multi_init();
    if (curlMulti_ == nullptr) {
        CurlGlobalCleanup();
        return UPLOAD_ERRORCODE_UPLOAD_LIB_ERROR;
    }

    bool ret = MultiAddHandle(curlMulti_, curlArray_);
    if (ret == false) {
        return UPLOAD_ERRORCODE_UPLOAD_LIB_ERROR;
    }

    int isRuning = 0;
    curl_multi_perform(curlMulti_, &isRuning);
    UPLOAD_HILOGE(UPLOAD_MODULE_FRAMEWORK, "isRuning = %{public}d === taskid%{public}d",
        isRuning, uploadTask_->taskId_);
    do {
        int numfds = 0;
        int res = curl_multi_wait(curlMulti_, NULL, 0, TRANS_TIMEOUT_MS, &numfds);
        if (res != CURLM_OK) {
            UPLOAD_HILOGE(UPLOAD_MODULE_FRAMEWORK, "isRuning = %{public}d =timeout== taskid%{public}d",
                isRuning, uploadTask_->taskId_);
            return res;
        }
        curl_multi_perform(curlMulti_, &isRuning);
    } while (isRuning && !isReadAbort_);
    UPLOAD_HILOGE(UPLOAD_MODULE_FRAMEWORK, "isRuning = %{public}d =end== taskid%{public}d",
        isRuning, uploadTask_->taskId_);

    return CheckUploadStatus(curlMulti_);
}

void CUrlAdp::CurlGlobalInit()
{
    std::lock_guard<std::mutex> guard(curlMutex_);
    if (!isCurlGlobalInit_) {
        isCurlGlobalInit_ = true;
    }
}

void CUrlAdp::CurlGlobalCleanup()
{
    std::lock_guard<std::mutex> guard(curlMutex_);
    if (isCurlGlobalInit_) {
        isCurlGlobalInit_ = false;
    }
}

int CUrlAdp::CheckUploadStatus(CURLM *curlMulti)
{
    int msgsLeft = 0;
    int returnCode = UPLOAD_ERRORCODE_UPLOAD_FAIL;
    CURLMsg *msg = nullptr;
    while (!isReadAbort_ && (msg = curl_multi_info_read(curlMulti, &msgsLeft))) {
        if (msg->msg != CURLMSG_DONE) {
            continue;
        }
        CURL *eh = nullptr;
        eh = msg->easy_handle;
        returnCode = msg->data.result;
        if (returnCode != CURLE_OK) {
            UPLOAD_HILOGE(UPLOAD_MODULE_FRAMEWORK, "upload fail curl error %{public}d", returnCode);
            return UPLOAD_ERRORCODE_UPLOAD_LIB_ERROR;
        }

        long respCode = 0;
        curl_easy_getinfo(eh, CURLINFO_RESPONSE_CODE, &respCode);
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "upload http code %{public}ld", respCode);
        if (respCode != HTTP_SUCCESS) {
            returnCode = respCode;
            UPLOAD_HILOGE(UPLOAD_MODULE_FRAMEWORK, "upload fail http error %{public}d", returnCode);
            return UPLOAD_ERRORCODE_UPLOAD_FAIL;
        }
        returnCode = UPLOAD_OK;
    }

    UPLOAD_HILOGE(UPLOAD_MODULE_FRAMEWORK, "CheckUploadStatus =end== taskid%{public}d", uploadTask_->taskId_);
    return returnCode;
}

bool CUrlAdp::Remove()
{
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "remove In =isReadAbort_%d = taskId%d",
        isReadAbort_, uploadTask_->taskId_);
    std::lock_guard<std::mutex> guard(readMutex_);
    isReadAbort_ = true;
    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "remove end =isReadAbort_%d = taskId%d",
        isReadAbort_, uploadTask_->taskId_);
    return true;
}

bool CUrlAdp::ClearCurlResource()
{
    std::lock_guard<std::mutex> guard(mutex_);
    for (auto url : curlArray_) {
        curl_multi_remove_handle(curlMulti_, url);
        curl_easy_cleanup(url);
    }
    curlArray_.clear();
    if (curlMulti_) {
        curl_multi_cleanup(curlMulti_);
        curlMulti_ = nullptr;
    }
    if (mime_) {
        curl_mime_free(mime_);
        mime_ = nullptr;
    }
    CurlGlobalCleanup();
    return true;
}

int CUrlAdp::OnDebug(CURL *curl, curl_infotype itype, char *pData, size_t size, void *lpvoid)
{
    if (itype == CURLINFO_TEXT) {
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "===>OnDebug CURLINFO_TEXT is %{public}s", pData);
    } else if (itype == CURLINFO_HEADER_IN) {
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "===>OnDebug CURLINFO_HEADER_IN is %{public}s", pData);
    } else if (itype == CURLINFO_HEADER_OUT) {
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "===>OnDebug CURLINFO_HEADER_OUT is %{public}s", pData);
    } else if (itype == CURLINFO_DATA_IN) {
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "===>OnDebug CURLINFO_DATA_IN is %{public}s", pData);
    } else if (itype == CURLINFO_DATA_OUT) {
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "===>OnDebug CURLINFO_DATA_OUT is %{public}s", pData);
    }
    return (int)itype;
}

int CUrlAdp::ProgressCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow,
    curl_off_t ultotal, curl_off_t ulnow)
{
    FileData *fData = (FileData *) clientp;
    if (fData == nullptr) {
        return HTTP_FORCE_STOP;
    }
    CUrlAdp *url = (CUrlAdp *) fData->adp;
    if (url == nullptr) {
        return HTTP_FORCE_STOP;
    }
    if (ulnow > 0) {
        fData->upsize = fData->totalsize - (ultotal - ulnow);
    } else {
        fData->upsize = ulnow;
    }

    if (url && url->uploadTask_) {
        int64_t totalulnow = 0;
        for (auto &vmem : url->fileDatas_) {
            if (fData->filename == vmem.filename) {
                vmem.upsize = fData->upsize;
            }
            totalulnow += vmem.upsize;
        }
        url->uploadTask_->OnProgress(dltotal, dlnow, ultotal, totalulnow);
    }
    return 0;
}

size_t CUrlAdp::HeaderCallback(char *buffer, size_t size, size_t nitems, void *userdata)
{
    FileData *fData = (FileData *) userdata;
    if (fData == nullptr) {
        return size;
    }
    CUrlAdp *url = (CUrlAdp *) fData->adp;
    if (url == nullptr) {
        return size;
    }
    std::string stmp(buffer, size * nitems);
    const int32_t codeOk = 200;
    const std::string headEndFlag = "\r\n";

    if (std::string::npos != stmp.find("HTTP")) {
        fData->headSendFlag = COLLECT_DO_FLAG;
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "http header begin: %{public}s", stmp.c_str());
        const int codeLen = 3;
        std::string::size_type position = stmp.find_first_of(" ");
        std::string scode(stmp, position + 1, codeLen);
        fData->httpCode = std::stol(scode);
    } else if (stmp == headEndFlag) {
        fData->headSendFlag = COLLECT_END_FLAG;
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "http header end: %{public}s", stmp.c_str());
    }
    if (fData->headSendFlag == COLLECT_DO_FLAG || fData->headSendFlag == COLLECT_END_FLAG) {
        fData->responseHead.push_back(stmp);
    }
    if (url && url->uploadTask_ && fData->headSendFlag == COLLECT_END_FLAG) {
        std::string stoatalHead = "";
        for (auto &smem : fData->responseHead) {
            stoatalHead += smem;
        }
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "report head len: %{public}zu, content: %{public}s",
            stoatalHead.length(), stoatalHead.c_str());

        fData->responseHead.clear();
        fData->httpCode = 0;
    }
    return size * nitems;
}

size_t CUrlAdp::HeaderCallbackL5(char *buffer, size_t size, size_t nitems, void *userdata)
{
    FileData *fData = (FileData *) userdata;
    if (fData == nullptr) {
        return size;
    }
    CUrlAdp *url = (CUrlAdp *) fData->adp;
    if (url == nullptr) {
        return size;
    }
    std::string stmp(buffer, size * nitems);
    const int32_t codeOk = 200;
    UploadResponse resData;
    const std::string headEndFlag = "\r\n";

    if (std::string::npos != stmp.find("HTTP")) {
        fData->headSendFlag = COLLECT_DO_FLAG;
        const int codeLen = 3;
        std::string::size_type position = stmp.find_first_of(" ");
        std::string scode(stmp, position + 1, codeLen);
        fData->httpCode = std::stol(scode);
    } else if (stmp == headEndFlag) {
        fData->headSendFlag = COLLECT_END_FLAG;
    }
    if (COLLECT_DO_FLAG == fData->headSendFlag || COLLECT_END_FLAG == fData->headSendFlag) {
        fData->responseHead.push_back(stmp);
    }
    if (url && url->uploadTask_ && COLLECT_END_FLAG == fData->headSendFlag) {
        std::string stoatalHead = "";
        for (auto &smem : fData->responseHead) {
            stoatalHead += smem;
        }
        if (codeOk == fData->httpCode) {
            if (url->fileDatas_.size() == fData->fileIndex && url->config_->fsuccess != nullptr) {
                resData.headers = stoatalHead;
                resData.code = fData->httpCode;
                UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "===>HeaderCallbackL5 success response head is %{public}s",
                    resData.headers.c_str());
                url->config_->fsuccess(resData);
            }
        } else {
            if (url->config_->ffail) {
                url->config_->ffail(stoatalHead, fData->httpCode);
            }
            UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "===>HeaderCallbackL5 fail Data.code is %{public}d", resData.code);
        }
        fData->responseHead.clear();
        fData->httpCode = 0;
    }
    return size * nitems;
}

size_t CUrlAdp::ReadCallback(char *buffer, size_t size, size_t nitems, void *arg)
{
    FileData *read = (FileData *) arg;
    if (read == nullptr) {
        UPLOAD_HILOGI(UPLOAD_MODULE_FRAMEWORK, "read is null");
        return CURL_READFUNC_ABORT;
    }
    CUrlAdp *adp = (CUrlAdp *) read->adp;
    if (adp == nullptr) {
        UPLOAD_HILOGI(UPLOAD_MODULE_FRAMEWORK, "adp is null");
        return CURL_READFUNC_ABORT;
    }
    std::lock_guard<std::mutex> guard(adp->readMutex_);
    if (ferror(read->fp) || adp->IsReadAbort()) {
        UPLOAD_HILOGI(UPLOAD_MODULE_FRAMEWORK, "read abort or ferror");
        return CURL_READFUNC_ABORT;
    }
    std::condition_variable condition;
    size_t readSize = 0;
    std::mutex mutexlock;
    std::unique_lock<std::mutex> lock(mutexlock);
    auto readTask = [&] {
        std::unique_lock<std::mutex> readlock(mutexlock);
        readSize = fread(buffer, size, nitems, read->fp);
        condition.notify_one();
    };

    std::thread tRead(readTask);

    if (condition.wait_for(lock, std::chrono::milliseconds(READFILE_TIMEOUT_MS)) == std::cv_status::timeout) {
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "OutTime error");
        adp->isReadAbort_ = true;
    }
    tRead.join();

    return readSize;
}

void CUrlAdp::InitTimerInfo()
{
    return;
}

void CUrlAdp::StartTimer()
{
    return;
}

void CUrlAdp::StopTimer()
{
    return;
}
} // namespace  OHOS::Plugin::Request::Upload