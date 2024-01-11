/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_REQUEST_JS_TASK_H
#define PLUGINS_REQUEST_JS_TASK_H
#include <map>
#include <string>
#include <vector>
#include <unordered_set>
#include "async_call.h"
#include "constant.h"
#include "i_task.h"
#include "js_notify.h"
#include "napi/native_api.h"
#include "napi_utils.h"

namespace OHOS::Plugin::Request {
class JsTask final {
public:
    JsTask() = default;
    ~JsTask() = default;
    JsTask(JsTask const &) = delete;
    void operator=(JsTask const &) = delete;
    JsTask(JsTask &&) = delete;
    JsTask &operator=(JsTask &&) = delete;

    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value Pause(napi_env env, napi_callback_info info);
    static napi_value QueryMimeType(napi_env env, napi_callback_info info);
    static napi_value Query(napi_env env, napi_callback_info info);
    static napi_value Remove(napi_env env, napi_callback_info info);
    static napi_value Resume(napi_env env, napi_callback_info info);
    static napi_value Start(napi_env env, napi_callback_info info);
    static napi_value Stop(napi_env env, napi_callback_info info);
    static std::map<Reason, DownloadErrorCode> failMap_;

private:
    struct JsParam {
        std::string type = "";
        napi_value callback = nullptr;
        ITask *task = nullptr;
    };
    enum { BOOL_RES, STR_RES, INFO_RES };
    struct ExecContext : public AsyncCall::Context {
        ITask *task = nullptr;
        bool boolRes = false;
        std::string strRes = "";
        DownloadInfo infoRes {};
    };

    using Event = std::function<int32_t(const std::shared_ptr<ExecContext> &)>;
    static napi_value Exec(napi_env env, napi_callback_info info, const std::string &execType);

    static int32_t StartExec(const std::shared_ptr<ExecContext> &context);
    static int32_t StopExec(const std::shared_ptr<ExecContext> &context);
    static int32_t PauseExec(const std::shared_ptr<ExecContext> &context);
    static int32_t QueryMimeTypeExec(const std::shared_ptr<ExecContext> &context);
    static int32_t QueryExec(const std::shared_ptr<ExecContext> &context);
    static int32_t RemoveExec(const std::shared_ptr<ExecContext> &context);
    static int32_t ResumeExec(const std::shared_ptr<ExecContext> &context);

    static napi_status ParseInputParameters(napi_env env, size_t argc, napi_value self,
        const std::shared_ptr<ExecContext> &context);
    static ExceptionError ParseOnOffParameters(napi_env env, napi_callback_info info, bool IsRequiredParam,
        JsParam &jsParam);
    static napi_status GetResult(napi_env env, const std::shared_ptr<ExecContext> &context,
        const std::string &execType, napi_value &result);
    static void GetDownloadInfo(const TaskInfo &infoRes, DownloadInfo &info);
    static bool IsSupportType(const std::string &type, Version version);
    static void UnifiedType(std::string &type);
    static std::shared_ptr<JsNotify> CreateNotify(napi_env env,
        const std::string &type, napi_value callback, Version version, Action action);

private:
    static std::unordered_set<std::string> supportEventsV10_;
    static std::unordered_set<std::string> supportEventsV9_;
    static std::map<std::string, Event> functionMap_;
    static std::map<std::string, uint32_t> resMap_;
    static std::map<State, DownloadStatus> stateMap_;
    static std::mutex listenerMutex_;
    static std::map<std::string, std::vector<std::shared_ptr<JsNotify>>> listenerMap_;
};
} // namespace OHOS::Plugin::Request

#endif // PLUGINS_REQUEST_JS_TASK_H