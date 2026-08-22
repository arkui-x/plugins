/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_REQUEST_CACHE_DOWNLOAD_IOS_CACHE_HTTP_CLIENT_IOS_H
#define PLUGINS_REQUEST_CACHE_DOWNLOAD_IOS_CACHE_HTTP_CLIENT_IOS_H

#include "cache_http_client.h"

#include <memory>

namespace OHOS::Plugin::CacheDownload {

/**
 * iOS implementation of CacheHttpClient backed by NSURLSession.
 */
class CacheHttpClientIOS : public CacheHttpClient {
public:
    CacheHttpClientIOS();
    ~CacheHttpClientIOS() override;

    std::shared_ptr<CacheHttpRequest> NewRequest(
        const CacheHttpRequestOptions& options,
        const std::shared_ptr<CacheHttpRequestCallback>& callback) override;
};

} // namespace OHOS::Plugin::CacheDownload

#endif // PLUGINS_REQUEST_CACHE_DOWNLOAD_IOS_CACHE_HTTP_CLIENT_IOS_H
