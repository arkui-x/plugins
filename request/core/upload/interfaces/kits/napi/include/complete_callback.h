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

#ifndef PLUGINS_REQUEST_UPLOAD_COMPLETE_CALLBACK_H
#define PLUGINS_REQUEST_UPLOAD_COMPLETE_CALLBACK_H

#include <uv.h>
#include <map>
#include <vector>

#include "i_complete_callback.h"
#include "i_callbackable_judger.h"
#include "js_util.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "upload_common.h"

namespace OHOS::Plugin::Request::Upload {
class CompleteCallback : public ICompleteCallback {
public:
    CompleteCallback(ICallbackAbleJudger *judger, napi_env env, napi_value callback);
    virtual ~CompleteCallback();
    void Complete(const std::vector<TaskState> &taskStates) override;
    napi_ref GetCallback() override;
private:
    struct CompleteWorker {
        ICallbackAbleJudger *judger;
        const CompleteCallback *callback = nullptr;
        const std::vector<TaskState> taskStates;
        CompleteWorker(ICallbackAbleJudger *judger, const CompleteCallback *callbackIn,
            const std::vector<TaskState> &taskStatesIn)
            : judger(judger), callback(callbackIn), taskStates(taskStatesIn) {}
    };

    ICallbackAbleJudger *judger_ = nullptr;
    napi_ref callback_ = nullptr;
    napi_env env_;
    uv_loop_s *loop_ = nullptr;
};
} // end of  OHOS::Plugin::Request::Upload
#endif