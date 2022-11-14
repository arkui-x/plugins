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

#ifndef PLUGINS_REQUEST_UPLOAD_PROGRESS_CALLBACK
#define PLUGINS_REQUEST_UPLOAD_PROGRESS_CALLBACK

#include <uv.h>

#include "i_callbackable_judger.h"
#include "i_progress_callback.h"
#include "js_util.h"
#include "napi/native_common.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS::Plugin::Request::Upload {
using namespace OHOS::Plugin::Request::UploadNapi;
class ProgressCallback : public IProgressCallback {
public:
    ProgressCallback(ICallbackAbleJudger *judger, napi_env env, napi_value callback);
    virtual ~ProgressCallback();
    void Progress(const int64_t uploadedSize, const int64_t totalSize) override;
    napi_ref GetCallback() override;
private:
    struct ProgressWorker {
        ICallbackAbleJudger *judger;
        const ProgressCallback *callback = nullptr;
        const int64_t uploadedSize;
        const int64_t totalSize;
        ProgressWorker(ICallbackAbleJudger *judgerIn, const ProgressCallback *callbackIn,
            int64_t uploadedSizeIn, int64_t totalSizeIn)
            : judger(judgerIn), callback(callbackIn), uploadedSize(uploadedSizeIn), totalSize(totalSizeIn) {}
    };

    ICallbackAbleJudger *judger_ = nullptr;
    napi_ref callback_ = nullptr;
    napi_env env_;
    uv_loop_s *loop_ = nullptr;
};
} // end of  OHOS::Plugin::Request::Upload
#endif