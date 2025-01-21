/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "web_message_port.h"

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "napi_web_message_ext.h"

namespace OHOS::Plugin {
namespace {
const std::string WEB_MESSAGE_PORT_ONE = "port1";
const std::string WEB_MESSAGE_PORT_TWO = "port2";
}

thread_local std::map<int32_t, std::map<std::string, const WebMessagePort*>> WebMessagePort::webMessagePortArray_;

void WebMessagePort::InsertPort(const WebMessagePort* webMessagePort)
{
    CHECK_NULL_VOID(webMessagePort);
    auto webId = webMessagePort->GetWebId();
    auto portHandle = webMessagePort->GetPortHandle();
    if (portHandle != WEB_MESSAGE_PORT_ONE && portHandle != WEB_MESSAGE_PORT_TWO) {
        return;
    }
    auto iter = webMessagePortArray_.find(webId);
    if (iter == webMessagePortArray_.end()) {
        std::map<std::string, const WebMessagePort*> port = {{ portHandle, webMessagePort }};
        webMessagePortArray_[webId] = port;
    } else {
        (iter->second)[portHandle] = webMessagePort;
    }
}

void WebMessagePort::ErasePort(const WebMessagePort* webMessagePort)
{
    CHECK_NULL_VOID(webMessagePort);
    auto webId = webMessagePort->GetWebId();
    auto portHandle = webMessagePort->GetPortHandle();
    if (portHandle != WEB_MESSAGE_PORT_ONE && portHandle != WEB_MESSAGE_PORT_TWO) {
        return;
    }
    auto iter = webMessagePortArray_.find(webId);
    if (iter != webMessagePortArray_.end()) {
        for (auto it = iter->second.cbegin(); it != iter->second.cend(); it++) {
            if (it->second == webMessagePort) {
                iter->second.erase(it);
                if (iter->second.empty()) {
                    webMessagePortArray_.erase(iter);
                }
                return;
            }
        }
    }
}

void WebMessagePort::UvJsCallbackThreadWorker(uv_work_t* work, int status)
{
    LOGI("WebMessagePort UvJsCallbackThreadWorker called");
    if (work == nullptr) {
        return;
    }
    NapiJsCallBackParm* param = reinterpret_cast<NapiJsCallBackParm*>(work->data);
    if (param == nullptr || !(param->env)) {
        delete work;
        work = nullptr;
        return;
    }

    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(param->env, &scope);
    if (scope == nullptr) {
        delete work;
        work = nullptr;
        return;
    }

    napi_value args[INTEGER_ONE] = { 0 };
    if (param->result_.empty()) {
        napi_get_null(param->env, &args[INTEGER_ZERO]);
    } else {
        napi_value jsResult = nullptr;
        LOGI("param result: %{public}s", param->result_.c_str());
        napi_create_string_utf8(param->env, param->result_.c_str(), param->result_.length(), &jsResult);
        args[INTEGER_ZERO] = jsResult;
        napi_value callback = nullptr;
        napi_value callbackResult = nullptr;
        napi_get_reference_value(param->env, param->callback, &callback);
        napi_call_function(param->env, nullptr, callback, sizeof(args) / sizeof(args[0]), args, &callbackResult);
    }

    napi_close_handle_scope(param->env, scope);
    delete work;
}

void WebMessagePort::HandleArrayType(NapiJsCallBackParmExt* param, WebMessageExt* webMessageExt)
{
    switch (param->result_->GetArrayType()) {
        case WebValue::Type::STRINGARRAY:
            webMessageExt->SetStringArray(param->result_->GetStringArray());
            break;
        case WebValue::Type::DOUBLEARRAY:
            webMessageExt->SetDoubleArray(param->result_->GetDoubleArray());
            break;
        case WebValue::Type::INT64ARRAY:
            webMessageExt->SetInt64Array(param->result_->GetInt64Array());
            break;
        case WebValue::Type::BOOLEANARRAY:
            webMessageExt->SetBooleanArray(param->result_->GetBooleanArray());
            break;
        default:
            break;
    }
}

void WebMessagePort::UvJsExtCallbackThreadWorker(uv_work_t* work, int status)
{
    if (work == nullptr) {
        return;
    }
    NapiJsCallBackParmExt* param = reinterpret_cast<NapiJsCallBackParmExt*>(work->data);
    if (param == nullptr || !(param->env)) {
        delete work;
        work = nullptr;
        return;
    }

    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(param->env, &scope);
    if (scope == nullptr) {
        delete work;
        work = nullptr;
        return;
    }

    napi_value args[INTEGER_ONE] = { 0 };

    if (param->result_ == nullptr) {
        napi_get_null(param->env, &args[INTEGER_ZERO]);
    } else {
        napi_value jsResult = nullptr;
        napi_create_string_utf8(param->env, param->result_->GetString().c_str(), param->result_->GetString().length(), &jsResult);
        args[INTEGER_ZERO] = jsResult;

        napi_value webMsgExt = nullptr;
        napi_status status = napi_get_reference_value(param->env, NapiWebMessageExt::g_webMsgExtClassRef, &webMsgExt);
        if (status != napi_status::napi_ok) {
            LOGE("napi_get_reference_value failed.");
            return ;
        }
        status = napi_new_instance(param->env, webMsgExt, 0, NULL, &args[INTEGER_ZERO]);
        if (status != napi_status::napi_ok) {
            LOGE("napi_new_instance failed.");
            return ;
        }
        auto webMsg = std::make_shared<OHOS::Plugin::WebMessage>(WebValue::Type::NONE);
        WebMessageExt* webMessageExt = new (std::nothrow) WebMessageExt(webMsg);
        if (webMessageExt == nullptr) {
            LOGE("new WebMessageExt failed.");
            return;
        }
        switch (param->result_->GetType()) {
            case static_cast<int>(WebMessageType::STRING):
                webMessageExt->SetString(param->result_->GetString());
                break;
            case static_cast<int>(WebMessageType::NUMBER):
                webMessageExt->SetNumber(param->result_->GetNumber());
                break;
            case static_cast<int>(WebMessageType::BOOLEAN):
                webMessageExt->SetBoolean(param->result_->GetBoolean());
                break;
            case static_cast<int>(WebMessageType::ARRAYBUFFER): {
                    auto arrayBuffer = param->result_->GetArrayBuffer();
                    webMessageExt->SetArrayBuffer(arrayBuffer);
                }
                break;
            case static_cast<int>(WebMessageType::ARRAY):
                HandleArrayType(param, webMessageExt);
                break;
            default:
                LOGI("Unsupported WebValue type.");
                break;
        }

        status = napi_wrap(param->env, args[INTEGER_ZERO], webMessageExt,
        [](napi_env env, void* data, void* hint) {
            WebMessageExt* webMessageExt = static_cast<WebMessageExt*>(data);
            delete webMessageExt;
            webMessageExt = nullptr;
        },
        nullptr, nullptr);
        if (status != napi_status::napi_ok) {
            LOGE("napi_wrap failed %{public}d",status);
            return;
        }
        napi_value callback = nullptr;
        napi_value callbackResult = nullptr;
        napi_get_reference_value(param->env, param->callback, &callback);
        napi_call_function(param->env, nullptr, callback, sizeof(args) / sizeof(args[0]), args, &callbackResult);
    }

    napi_close_handle_scope(param->env, scope);
    delete work;
}

void WebMessagePort::OnMessage(int32_t webId, const std::string& portHandle, const std::string& result)
{
    LOGI("WebMessagePort OnMessage webId: %{public}d, portHandle: %{public}s, result: %{public}s.",
        webId, portHandle.c_str(), result.c_str());
    if (result.empty()) {
        return;
    }
    auto iter = webMessagePortArray_.find(webId);
    if (iter != webMessagePortArray_.end()) {
        auto it = iter->second.find(portHandle);
        if (it == iter->second.end()) {
            return;
        }
        auto targetPort = it->second;
        CHECK_NULL_VOID(targetPort);
        auto param = targetPort->GetWebMessageCallback();
        CHECK_NULL_VOID(param);
        CHECK_NULL_VOID(param->env);

        param->result_ = result;
        uv_loop_s* loop = nullptr;
        uv_work_t* work = nullptr;
        napi_get_uv_event_loop(param->env, &loop);
        if (loop == nullptr) {
            LOGE("get uv event loop failed");
            return;
        }
        work = new (std::nothrow) uv_work_t;
        if (work == nullptr) {
            LOGE("new uv work failed");
            return;
        }

        work->data = reinterpret_cast<void*>(param.get());
        int ret = uv_queue_work_with_qos(loop, work, [](uv_work_t* work) {}, UvJsCallbackThreadWorker,
            uv_qos_user_initiated);
        if (ret != 0) {
            if (work != nullptr) {
                delete work;
                work = nullptr;
            }
        }
    }
}

void WebMessagePort::OnMessageExt(
    int32_t webId, const std::string& portHandle, const std::shared_ptr<WebMessageExt>& webMessage)
{
    if (!webMessage) {
        LOGE("WebMessagePort OnMessageExt webMessage is null.");
        return;
    }
    auto iter = webMessagePortArray_.find(webId);
    if (iter != webMessagePortArray_.end()) {
        auto it = iter->second.find(portHandle);
        if (it == iter->second.end()) {
            return;
        }
        auto targetPort = it->second;
        CHECK_NULL_VOID(targetPort);
        auto param = targetPort->GetWebMessageExtCallback();
        CHECK_NULL_VOID(param);
        CHECK_NULL_VOID(param->env);

        param->result_ = std::make_shared<WebMessageExt>(*webMessage);
        CHECK_NULL_VOID(param->result_);
        uv_loop_s* loop = nullptr;
        uv_work_t* work = nullptr;
        napi_get_uv_event_loop(param->env, &loop);
        if (loop == nullptr) {
            LOGE("get uv event loop failed");
            return;
        }
        work = new (std::nothrow) uv_work_t;
        if (work == nullptr) {
            LOGE("new uv work failed");
            return;
        }

        work->data = reinterpret_cast<void*>(param.get());
        int ret = uv_queue_work_with_qos(
            loop, work, [](uv_work_t* work) {}, UvJsExtCallbackThreadWorker, uv_qos_user_initiated);
        if (ret != 0) {
            if (work != nullptr) {
                delete work;
                work = nullptr;
            }
        }
    }
}
} // namespace OHOS::Plugin
