/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PLUGIN_WEB_DOWNLOAD_DELEGATE_H
#define PLUGIN_WEB_DOWNLOAD_DELEGATE_H

#include <string>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "web_download_item.h"

namespace OHOS {
namespace Plugin {
class WebDownloadDelegate {
public:
    explicit WebDownloadDelegate(napi_env env);
    virtual ~WebDownloadDelegate();

    virtual void DownloadBeforeStart(WebDownloadItem* webDownloadItem);
    virtual void DownloadDidUpdate(WebDownloadItem* webDownloadItem);
    virtual void DownloadDidFail(WebDownloadItem* webDownloadItem);
    virtual void DownloadDidFinish(WebDownloadItem* webDownloadItem);

    virtual void PutDownloadBeforeStart(napi_env, napi_value callback);
    virtual void PutDownloadDidUpdate(napi_env, napi_value callback);
    virtual void PutDownloadDidFinish(napi_env, napi_value callback);
    virtual void PutDownloadDidFail(napi_env, napi_value callback);

    int32_t GetNWebId() const;
    void SetNWebId(int32_t nwebId);
    napi_env GetEnv();
    void RemoveSelfRef();
    napi_ref delegate_;

private:
    int32_t nwebId_ = -1;
    napi_ref download_before_start_callback_;
    napi_ref download_did_update_callback_;
    napi_ref download_did_finish_callback_;
    napi_ref download_did_fail_callback_;
    napi_env env_;
};
} // namespace Plugin
} // namespace OHOS
#endif // PLUGIN_WEB_DOWNLOAD_DELEGATE_H
