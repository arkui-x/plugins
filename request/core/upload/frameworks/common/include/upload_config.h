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

#ifndef PLUGINS_REQUEST_UPLOAD_UPLOAD_CONFIG
#define PLUGINS_REQUEST_UPLOAD_UPLOAD_CONFIG

#include <curl/curl.h>
#include <string>
#include <vector>
#include <map>

namespace  OHOS::Plugin::Request::Upload {
struct File {
    std::string filename;
    std::string name;
    std::string uri;
    std::string type;
};

struct RequestData {
    std::string name;
    std::string value;
};

struct UploadResponse {
    int32_t code;
    std::string data;
    std::string headers;
};

struct UploadConfig {
    std::string url;
    std::map<std::string, std::string> header;
    std::string method;
    std::vector<File> files;
    std::vector<RequestData> data;
    std::function<void(UploadResponse &data)> fsuccess;
    std::function<void(std::string &data, int32_t &code)> ffail;
    std::function<void()> fcomplete;
    std::string protocolVersion;
};
class CUrlAdp;
struct FileData {
    FILE *fp;
    std::string filename;
    std::string name;
    std::string type;
    std::shared_ptr<CUrlAdp> adp;
    int64_t upsize;
    int64_t totalsize;
    uint32_t fileIndex;
    struct curl_slist *list;
    std::vector<std::string> responseHead;
    int32_t headSendFlag;
    int32_t httpCode;
    uint32_t result;
};
} // namespace OHOS::Plugin::Request::Upload
#endif