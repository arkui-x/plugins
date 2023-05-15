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

#ifndef HEADER_RECEIVE_CALLBACK
#define HEADER_RECEIVE_CALLBACK

#include <uv.h>
#include "i_header_receive_callback.h"
#include "js_util.h"
#include "napi/native_common.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS::Plugin::Request::Upload {
class HeaderReceiveCallback : public IHeaderReceiveCallback,
                              public std::enable_shared_from_this<HeaderReceiveCallback> {
public:
    HeaderReceiveCallback(napi_env env, napi_value callback);
    virtual ~HeaderReceiveCallback();
    virtual void HeaderReceive(const std::string &header) override;
    napi_ref GetCallback() override;
    std::string GetHeader();
    napi_env GetEnv();
    std::mutex mutex_;

private:
    std::string header_;
    napi_ref callback_ = nullptr;
    napi_env env_;
    uv_loop_s *loop_ = nullptr;
};

struct HeaderReceiveWorker {
    std::shared_ptr<HeaderReceiveCallback> observer = nullptr;
};
} // namespace OHOS::Plugin::Request::Upload
#endif