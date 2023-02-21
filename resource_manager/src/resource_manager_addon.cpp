/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "resource_manager_addon.h"

#include <fstream>
#include <memory>
#include <mutex>
#include <vector>

#if defined(__ARKUI_CROSS__)
#include "plugins/interfaces/native/log.h"
#else
#include "hilog/log_c.h"
#include "hilog/log_cpp.h"
#include "hitrace_meter.h"
#include "hisysevent_adapter.h"
#endif

#include "js_native_api.h"
#include "napi/native_common.h"
#include "napi/native_api.h"
#include "node_api.h"
#include "utils/utils.h"

namespace OHOS {
namespace Plugin {
namespace Resource {
#define GET_PARAMS(env, info, num)    \
    size_t argc = num;                \
    napi_value argv[num] = {nullptr}; \
    napi_value thisVar = nullptr;     \
    void *data = nullptr;             \
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data)

#if !defined(__ARKUI_CROSS__)
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, 0xD001E00, "ResourceManagerJs" };
using namespace OHOS::HiviewDFX;
#endif

static thread_local napi_ref* g_constructor = nullptr;

std::map<std::string, std::shared_ptr<ResourceManager>> g_resourceMgr;
std::mutex g_resMapLock;
#if defined(__ARKUI_CROSS__)
napi_value ResourceManagerAddon::Create(
    napi_env env, const std::string& bundleName, const std::shared_ptr<ResourceManager>& resMgr)
{
        std::shared_ptr<ResourceManagerAddon> addon = std::make_shared<ResourceManagerAddon>(bundleName, resMgr);
#else
napi_value ResourceManagerAddon::Create(
    napi_env env, const std::string& bundleName, const std::shared_ptr<ResourceManager>& resMgr,
    std::shared_ptr<AbilityRuntime::Context> context)
{
    std::shared_ptr<ResourceManagerAddon> addon = std::make_shared<ResourceManagerAddon>(bundleName, resMgr, context);
#endif
    if (!Init(env)) {
        LOGE("Failed to init resource manager addon");
        return nullptr;
    }

    napi_value constructor;
    napi_status status = napi_get_reference_value(env, *g_constructor, &constructor);
    if (status != napi_ok) {
        return nullptr;
    }
    napi_value result;
    status = napi_new_instance(env, constructor, 0, nullptr, &result);
    if (status != napi_ok) {
        return nullptr;
    }

    auto addonPtr = std::make_unique<std::shared_ptr<ResourceManagerAddon>>(addon);
    status = napi_wrap(env, result, reinterpret_cast<void *>(addonPtr.get()), ResourceManagerAddon::Destructor,
        nullptr, nullptr);
    if (status != napi_ok) {
        return nullptr;
    }
    addonPtr.release();
    return result;
}

#if defined(__ARKUI_CROSS__)
ResourceManagerAddon::ResourceManagerAddon(
    const std::string& bundleName, const std::shared_ptr<ResourceManager>& resMgr)
    : bundleName_(bundleName), resMgr_(resMgr)
#else
ResourceManagerAddon::ResourceManagerAddon(
    const std::string& bundleName, const std::shared_ptr<ResourceManager>& resMgr,
    const std::shared_ptr<AbilityRuntime::Context>& context)
    : bundleName_(bundleName), resMgr_(resMgr), context_(context)
#endif
{}

ResourceManagerAddon::~ResourceManagerAddon()
{
    LOGI("~ResourceManagerAddon %{public}s", bundleName_.c_str());
}

void ResourceManagerAddon::Destructor(napi_env env, void *nativeObject, void *hint)
{
    std::unique_ptr<std::shared_ptr<ResourceManagerAddon>> addonPtr;
    addonPtr.reset(static_cast<std::shared_ptr<ResourceManagerAddon>*>(nativeObject));
}

void ResMgrAsyncContext::SetErrorMsg(const std::string &msg, bool withResId, int32_t errCode)
{
    errMsg_ = msg;
    success_ = false;
    errCode_ = errCode;
    if (withResId) {
        LOGE("%{public}s id = %{public}d", msg.c_str(), resId_);
    } else {
        LOGE("%{public}s name = %{public}s", msg.c_str(), resName_.c_str());
    }
}

bool ResourceManagerAddon::Init(napi_env env)
{
    if (g_constructor != nullptr) {
        return true;
    }

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getString", GetString),
        DECLARE_NAPI_FUNCTION("getStringByName", GetStringByName),
        DECLARE_NAPI_FUNCTION("getStringArray", GetStringArray),
        DECLARE_NAPI_FUNCTION("getStringArrayByName", GetStringArrayByName),
        DECLARE_NAPI_FUNCTION("getMedia", GetMedia),
        DECLARE_NAPI_FUNCTION("getMediaByName", GetMediaByName),
        DECLARE_NAPI_FUNCTION("getMediaBase64", GetMediaBase64),
        DECLARE_NAPI_FUNCTION("getMediaBase64ByName", GetMediaBase64ByName),
        DECLARE_NAPI_FUNCTION("getConfiguration", GetConfiguration),
        DECLARE_NAPI_FUNCTION("getDeviceCapability", GetDeviceCapability),
        DECLARE_NAPI_FUNCTION("getPluralString", GetPluralString),
        DECLARE_NAPI_FUNCTION("getPluralStringByName", GetPluralStringByName),
        DECLARE_NAPI_FUNCTION("getRawFile", GetRawFile),
        DECLARE_NAPI_FUNCTION("getRawFileDescriptor", GetRawFileDescriptor),
        DECLARE_NAPI_FUNCTION("closeRawFileDescriptor", CloseRawFileDescriptor),
        DECLARE_NAPI_FUNCTION("getStringSync", GetStringSync),
        DECLARE_NAPI_FUNCTION("getStringByNameSync", GetStringByNameSync),
        DECLARE_NAPI_FUNCTION("getBoolean", GetBoolean),
        DECLARE_NAPI_FUNCTION("getNumber", GetNumber),
        DECLARE_NAPI_FUNCTION("getBooleanByName", GetBooleanByName),
        DECLARE_NAPI_FUNCTION("getNumberByName", GetNumberByName),
        DECLARE_NAPI_FUNCTION("release", Release)
    };

    napi_value constructor;
    napi_status status = napi_define_class(env, "ResourceManager", NAPI_AUTO_LENGTH, New, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        LOGE("Failed to define class at Init");
        return false;
    }

    g_constructor = new (std::nothrow) napi_ref;
    if (g_constructor == nullptr) {
        LOGE("Failed to create ref at init");
        return false;
    }
    status = napi_create_reference(env, constructor, 1, g_constructor);
    if (status != napi_ok) {
        LOGE("Failed to create reference at init");
        return false;
    }
    return true;
}

void ResMgrAsyncContext::Complete(napi_env env, napi_status status, void* data)
{
    ResMgrAsyncContext* asyncContext = static_cast<ResMgrAsyncContext*>(data);

    napi_value finalResult = nullptr;
    if (asyncContext->createValueFunc_ != nullptr) {
        finalResult = asyncContext->createValueFunc_(env, *asyncContext);
    }

    napi_value result[] = { nullptr, nullptr };
    if (asyncContext->success_) {
        napi_get_undefined(env, &result[0]);
        result[1] = finalResult;
    } else {
        napi_value message = nullptr;
        napi_create_string_utf8(env, asyncContext->errMsg_.c_str(), NAPI_AUTO_LENGTH, &message);
        napi_create_error(env, nullptr, message, &result[0]);
        napi_get_undefined(env, &result[1]);
    }
    if (asyncContext->deferred_) {
        if (asyncContext->success_) {
            if (napi_resolve_deferred(env, asyncContext->deferred_, result[1]) != napi_ok) {
                LOGE("napi_resolve_deferred failed");
            }
        } else {
            if (napi_reject_deferred(env, asyncContext->deferred_, result[0]) != napi_ok) {
                LOGE("napi_reject_deferred failed");
            }
        }
    } else {
        do {
            napi_value callback = nullptr;
            napi_status status = napi_get_reference_value(env, asyncContext->callbackRef_, &callback);
            if (status != napi_ok) {
                LOGE("napi_get_reference_value failed status=%{public}d", status);
                break;
            }
            napi_value userRet = nullptr;
            status = napi_call_function(env, nullptr, callback, sizeof(result) / sizeof(napi_value), result, &userRet);
            if (status != napi_ok) {
                LOGE("napi_call_function failed status=%{public}d", status);
                break;
            }
            status = napi_delete_reference(env, asyncContext->callbackRef_);
            if (status != napi_ok) {
                LOGE("napi_call_function failed status=%{public}d", status);
                break;
            }
        } while (false);
    }
    napi_delete_async_work(env, asyncContext->work_);
    delete asyncContext;
};

napi_value ResourceManagerAddon::New(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 1);

    napi_value target = nullptr;
    napi_get_new_target(env, info, &target);
    if (target != nullptr) {
        return thisVar;
    }

    return nullptr;
}

int ResourceManagerAddon::GetResId(napi_env env, size_t argc, napi_value *argv)
{
    if (argc == 0 || argv == nullptr) {
        return 0;
    }

    napi_valuetype valuetype;
    napi_status status = napi_typeof(env, argv[0], &valuetype);
    if (status != napi_ok) {
        LOGE("Failed to get value type");
        return 0;
    }
    if (valuetype != napi_number) {
        LOGE("Invalid param, not number");
        return 0;
    }
    int resId = 0;
    status = napi_get_value_int32(env, argv[0], &resId);
    if (status != napi_ok) {
        LOGE("Failed to get id number");
        return 0;
    }

    return resId;
}

std::string ResourceManagerAddon::GetResNameOrPath(napi_env env, size_t argc, napi_value *argv)
{
    if (argc == 0 || argv == nullptr) {
        return "";
    }

    napi_valuetype valuetype;
    napi_typeof(env, argv[0], &valuetype);
    if (valuetype != napi_string) {
        LOGE("Invalid param, not string");
        return "";
    }
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        LOGE("Failed to get resName or rawfile path length");
        return "";
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("Failed to get resName or raw file path");
        return "";
    }
    return buf.data();
}

napi_value ResourceManagerAddon::ProcessOnlyIdParam(napi_env env, napi_callback_info info, const std::string &name,
    napi_async_execute_callback execute)
{
    GET_PARAMS(env, info, 2);

    std::unique_ptr<ResMgrAsyncContext> asyncContext = std::make_unique<ResMgrAsyncContext>();
    std::shared_ptr<ResourceManagerAddon> *addonPtr = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&addonPtr));
    if (status != napi_ok) {
        LOGE("Failed to unwrap ProcessOnlyIdParam %{public}s", name.c_str());
        return nullptr;
    }
    asyncContext->addon_ = *addonPtr;
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType;
        napi_typeof(env, argv[i], &valueType);
        if (i == 0 && valueType == napi_number) {
            asyncContext->resId_ = GetResId(env, argc, argv);
        } else if (i == 1 && valueType == napi_function) {
            napi_create_reference(env, argv[i], 1, &asyncContext->callbackRef_);
            break;
        } else if (i == 0 && valueType == napi_string) {
            if (name.find("Name") != std::string::npos) {
                asyncContext->resName_ = GetResNameOrPath(env, argc, argv);
            } else {
                asyncContext->path_ = GetResNameOrPath(env, argc, argv);
            }
        } else if (i == 0 && valueType == napi_object) {
            std::shared_ptr<ResourceManager::Resource> resourcePtr = std::make_shared<ResourceManager::Resource>();
            bool ret = GetResourceObject(env, resourcePtr, argv[0]);
            if (!ret) {
                LOGE("Failed to get native Resource object");
                return nullptr;
            }
            asyncContext->resource_ = resourcePtr;
        } else {
            // self resourcemanager with promise
        }
    }
    napi_value result = ResMgrAsyncContext::getResult(env, asyncContext, name, execute);
    return result;
}

auto getStringByNameFunc = [](napi_env env, void* data) {
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    RState state = asyncContext->addon_->GetResMgr()->GetStringByName(asyncContext->resName_.c_str(),
        asyncContext->value_);
    if (state != RState::SUCCESS) {
        asyncContext->SetErrorMsg("GetStringByName failed state", true);
#if !defined(__ARKUI_CROSS__)
        ReportGetResourceByNameFail(asyncContext->resName_, asyncContext->value_,
            "failed in getStringByNameFunc");
#endif
        return;
    }
    asyncContext->createValueFunc_ = [](napi_env env, ResMgrAsyncContext& context) {
        napi_value jsValue = nullptr;
        if (napi_create_string_utf8(env, context.value_.c_str(), NAPI_AUTO_LENGTH, &jsValue) != napi_ok) {
            context.SetErrorMsg("GetStringByName failed to create result");
            return jsValue;
        }
        return jsValue;
    };
};

napi_value ResourceManagerAddon::GetStringByName(napi_env env, napi_callback_info info)
{
    return ProcessOnlyIdParam(env, info, "getStringByName", getStringByNameFunc);
}

auto getStringFunc = [](napi_env env, void* data) {
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    std::shared_ptr<ResourceManager> resMgr = nullptr;
    int32_t resId = 0;

    bool ret = ResMgrAsyncContext::GetHapResourceManager(asyncContext, resMgr, resId);
    if (!ret) {
        LOGE("Failed to GetHapResourceManager in getStringFunc");
        return;
    }
    RState state = resMgr->GetStringById(resId, asyncContext->value_);
#if !defined(__ARKUI_CROSS__)
    if (state != RState::SUCCESS) {
        ReportGetResourceByIdFail(resId, asyncContext->value_, "failed in getStringFunc");
        return;
    }
#endif
    if (state != RState::SUCCESS) {
        asyncContext->SetErrorMsg("GetString failed state", true);
        return;
    }
    asyncContext->createValueFunc_ = [](napi_env env, ResMgrAsyncContext& context) {
        napi_value jsValue = nullptr;
        if (napi_create_string_utf8(env, context.value_.c_str(), NAPI_AUTO_LENGTH, &jsValue) != napi_ok) {
            context.SetErrorMsg("GetString failed to create result");
            return jsValue;
        }
        return jsValue;
    };
};

napi_value ResourceManagerAddon::GetString(napi_env env, napi_callback_info info)
{
    return ProcessOnlyIdParam(env, info, "getString", getStringFunc);
}

auto getStringArrayFunc = [](napi_env env, void* data) {
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    RState state;
    int32_t resId = 0;
    std::shared_ptr<ResourceManager> resMgr = nullptr;
    if (asyncContext->resId_ != 0 || asyncContext->resource_ != nullptr) {
        bool ret = ResMgrAsyncContext::GetHapResourceManager(asyncContext, resMgr, resId);
        if (!ret) {
            LOGE("Failed to GetHapResourceManager in getStringArrayFunc");
            return;
        }
        state = resMgr->GetStringArrayById(resId, asyncContext->arrayValue_);
    } else {
        state = asyncContext->addon_->GetResMgr()->GetStringArrayByName(asyncContext->resName_.c_str(),
            asyncContext->arrayValue_);
    }
    if (state != RState::SUCCESS) {
        asyncContext->SetErrorMsg("GetStringArray failed state", true);
        return;
    }
    asyncContext->createValueFunc_ = [](napi_env env, ResMgrAsyncContext &context) -> napi_value {
        napi_value result;
        napi_status status = napi_create_array_with_length(env, context.arrayValue_.size(), &result);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create array");
            return nullptr;
        }
        for (size_t i = 0; i < context.arrayValue_.size(); i++) {
            napi_value value;
            status = napi_create_string_utf8(env, context.arrayValue_[i].c_str(), NAPI_AUTO_LENGTH, &value);
            if (status != napi_ok) {
                context.SetErrorMsg("Failed to create string item");
                return nullptr;
            }
            status = napi_set_element(env, result, i, value);
            if (status != napi_ok) {
                context.SetErrorMsg("Failed to set array item");
                return nullptr;
            }
        }

        return result;
    };
};

napi_value ResourceManagerAddon::GetStringArrayByName(napi_env env, napi_callback_info info)
{
    return ProcessOnlyIdParam(env, info, "GetStringArrayByName", getStringArrayFunc);
}

napi_value ResourceManagerAddon::GetStringArray(napi_env env, napi_callback_info info)
{
    return ProcessOnlyIdParam(env, info, "getStringArray", getStringArrayFunc);
}

void CreateValue(ResMgrAsyncContext *asyncContext)
{
    asyncContext->createValueFunc_ = [](napi_env env, ResMgrAsyncContext& context) -> napi_value {
        napi_value buffer;
        napi_status status = napi_create_external_arraybuffer(env, context.mediaData.get(), context.len_,
            [](napi_env env, void *data, void *hint) {
                LOGE("Media buffer finalized");
                delete[] static_cast<char*>(data);
            }, nullptr, &buffer);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create media external array buffer");
            return nullptr;
        }

        napi_value result = nullptr;
        status = napi_create_typedarray(env, napi_uint8_array, context.len_, buffer, 0, &result);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create media typed array");
            return nullptr;
        }
        context.mediaData.release();
        return result;
    };
}

auto getMediaByNameFunc = [](napi_env env, void *data) {
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    RState state = asyncContext->addon_->GetResMgr()->GetMediaDataByName(asyncContext->resName_.c_str(),
        asyncContext->len_, asyncContext->mediaData);
    if (state != RState::SUCCESS) {
        asyncContext->SetErrorMsg("Failed to get media data in getMediaByNameFunc", false, state);
        return;
    }
    CreateValue(asyncContext);
    return;
};

napi_value ResourceManagerAddon::GetMediaByName(napi_env env, napi_callback_info info)
{
#if defined(__ARKUI_CROSS__)
    napi_value media = ProcessOnlyIdParam(env, info, "getMediaByName", getMediaByNameFunc);
#else
    std::string traceVal = "ResourceManagerAddon::GetMediaByName";
    StartTrace(HITRACE_TAG_GLOBAL_RESMGR, traceVal);
    napi_value media = ProcessOnlyIdParam(env, info, "getMediaByName", getMediaByNameFunc);
    FinishTrace(HITRACE_TAG_GLOBAL_RESMGR);
#endif
    return media;
}

auto getMediaFunc = [](napi_env env, void *data) {
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    std::string path;
    int32_t resId = 0;
    std::shared_ptr<ResourceManager> resMgr = nullptr;
    bool ret = ResMgrAsyncContext::GetHapResourceManager(asyncContext, resMgr, resId);
    if (!ret) {
        LOGE("Failed to GetHapResourceManager in getMediaFunc");
        return;
    }
    RState state = resMgr->GetMediaDataById(resId, asyncContext->len_, asyncContext->mediaData);
    if (state != SUCCESS) {
        LOGE("Failed to MediaData in getMediaFunc");
        return;
    }
    CreateValue(asyncContext);
};

napi_value ResourceManagerAddon::GetMedia(napi_env env, napi_callback_info info)
{
#if defined(__ARKUI_CROSS__)
    napi_value media = ProcessOnlyIdParam(env, info, "getMedia", getMediaFunc);
#else
    std::string traceVal = "ResourceManagerAddon::GetMedia";
    StartTrace(HITRACE_TAG_GLOBAL_RESMGR, traceVal);
    napi_value media = ProcessOnlyIdParam(env, info, "getMedia", getMediaFunc);
    FinishTrace(HITRACE_TAG_GLOBAL_RESMGR);
#endif
    return media;
}

void CreateStringValue(ResMgrAsyncContext *asyncContext)
{
    asyncContext->createValueFunc_ = [](napi_env env, ResMgrAsyncContext &context) {
        napi_value result;
        if (napi_create_string_utf8(env, context.value_.c_str(), NAPI_AUTO_LENGTH, &result) != napi_ok) {
            context.SetErrorMsg("Failed to create result");
            return result;
        }
        return result;
    };
}

auto getMediaBase64Func = [](napi_env env, void *data) {
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    std::string path;
    RState state;
    if (asyncContext->resId_ != 0 || asyncContext->resource_ != nullptr) {
        int32_t resId = 0;
        std::shared_ptr<ResourceManager> resMgr = nullptr;
        bool ret = ResMgrAsyncContext::GetHapResourceManager(asyncContext, resMgr, resId);
        if (!ret) {
            LOGE("Failed to GetHapResourceManager in getMediaBase64Func");
            return;
        }
        state = resMgr->GetMediaBase64DataById(resId, asyncContext->value_);
        if (state != RState::SUCCESS) {
            asyncContext->SetErrorMsg("GetMedia path failed", true, state);
            return;
        }
        CreateStringValue(asyncContext);
    } else {
        state = asyncContext->addon_->GetResMgr()->GetMediaBase64DataByName(asyncContext->resName_.c_str(),
            asyncContext->value_);
        if (state != RState::SUCCESS) {
            asyncContext->SetErrorMsg("GetMedia path failed", false, state);
            return;
        }
        CreateStringValue(asyncContext);
    }
};

napi_value ResourceManagerAddon::GetMediaBase64(napi_env env, napi_callback_info info)
{
#if defined(__ARKUI_CROSS__)
    napi_value mediaBase64 = ProcessOnlyIdParam(env, info, "GetMediaBase64", getMediaBase64Func);
#else
    std::string traceVal = "ResourceManagerAddon::GetMediaBase64";
    StartTrace(HITRACE_TAG_GLOBAL_RESMGR, traceVal);
    napi_value mediaBase64 = ProcessOnlyIdParam(env, info, "GetMediaBase64", getMediaBase64Func);
    FinishTrace(HITRACE_TAG_GLOBAL_RESMGR);
#endif
    return mediaBase64;
}

napi_value ResourceManagerAddon::GetMediaBase64ByName(napi_env env, napi_callback_info info)
{
#if defined(__ARKUI_CROSS__)
    napi_value mediaBase64 = ProcessOnlyIdParam(env, info, "GetMediaBase64ByName", getMediaBase64Func);
#else
    std::string traceVal = "ResourceManagerAddon::GetMediaBase64ByName";
    StartTrace(HITRACE_TAG_GLOBAL_RESMGR, traceVal);
    napi_value mediaBase64 = ProcessOnlyIdParam(env, info, "GetMediaBase64ByName", getMediaBase64Func);
    FinishTrace(HITRACE_TAG_GLOBAL_RESMGR);
#endif
    return mediaBase64;
}

napi_value ResourceManagerAddon::Release(napi_env env, napi_callback_info info)
{
    napi_value undefined;
    if (napi_get_undefined(env, &undefined) != napi_ok) {
        return nullptr;
    }
    return undefined;
}

std::string ResourceManagerAddon::GetLocale(std::unique_ptr<ResConfig> &cfg)
{
    std::string result;
#ifdef SUPPORT_GRAPHICS
    const icu::Locale *localeInfo = cfg->GetLocaleInfo();
    if (localeInfo == nullptr) {
        return result;
    }
    const char *lang = localeInfo->getLanguage();
    if (lang == nullptr) {
        return result;
    }
    result = lang;

    const char *script = localeInfo->getScript();
    if (script != nullptr) {
        result += std::string("_") + script;
    }

    const char *region = localeInfo->getCountry();
    if (region != nullptr) {
        result += std::string("_") + region;
    }
#endif
    return result;
}

napi_value ResourceManagerAddon::ProcessNoParam(napi_env env, napi_callback_info info, const std::string &name,
    napi_async_execute_callback execute)
{
    GET_PARAMS(env, info, 1);

    std::unique_ptr<ResMgrAsyncContext> asyncContext = std::make_unique<ResMgrAsyncContext>();
    std::shared_ptr<ResourceManagerAddon> *addonPtr = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&addonPtr));
    if (status != napi_ok) {
        LOGE("Failed to unwrap ProcessNoParam");
        return nullptr;
    }
    asyncContext->addon_ = *addonPtr;
    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    if (valueType == napi_function) {
        napi_create_reference(env, argv[0], 1, &asyncContext->callbackRef_);
    }

    napi_value result = ResMgrAsyncContext::getResult(env, asyncContext, name, execute);
    return result;
}

auto getConfigFunc = [](napi_env env, void *data) {
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    asyncContext->createValueFunc_ = [](napi_env env, ResMgrAsyncContext& context) -> napi_value {
        std::unique_ptr<ResConfig> cfg(CreateResConfig());
        context.addon_->GetResMgr()->GetResConfig(*cfg);

        napi_value result;
        napi_status status = napi_create_object(env, &result);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create Configuration object");
            return nullptr;
        }

        napi_value direction;
        status = napi_create_int32(env, static_cast<int>(cfg->GetDirection()), &direction);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create direction");
            return nullptr;
        }
        status = napi_set_named_property(env, result, "direction", direction);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to set direction property");
            return nullptr;
        }

        napi_value locale;
        status = napi_create_string_utf8(env, context.addon_->GetLocale(cfg).c_str(), NAPI_AUTO_LENGTH, &locale);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create locale");
            return nullptr;
        }
        status = napi_set_named_property(env, result, "locale", locale);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to set locale property");
            return nullptr;
        }
        return result;
    };
};

napi_value ResourceManagerAddon::GetConfiguration(napi_env env, napi_callback_info info)
{
    return ProcessNoParam(env, info, "GetConfiguration", getConfigFunc);
}

auto getDeviceCapFunc = [](napi_env env, void *data) {
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    asyncContext->createValueFunc_ = [](napi_env env, ResMgrAsyncContext& context) -> napi_value {
        std::unique_ptr<ResConfig> cfg(CreateResConfig());
        context.addon_->GetResMgr()->GetResConfig(*cfg);

        napi_value result;
        napi_status status = napi_create_object(env, &result);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create GetDeviceCapability object");
            return nullptr;
        }

        napi_value deviceType;
        status = napi_create_int32(env, static_cast<int>(cfg->GetDeviceType()), &deviceType);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create deviceType");
            return nullptr;
        }
        status = napi_set_named_property(env, result, "deviceType", deviceType);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to set deviceType property");
            return nullptr;
        }

        napi_value screenDensity;
        status = napi_create_int32(env, static_cast<int>(cfg->GetScreenDensity()), &screenDensity);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create screenDensity");
            return nullptr;
        }
        status = napi_set_named_property(env, result, "screenDensity", screenDensity);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to set screenDensity property");
            return nullptr;
        }
        return result;
    };
};

napi_value ResourceManagerAddon::GetDeviceCapability(napi_env env, napi_callback_info info)
{
    return ProcessNoParam(env, info, "GetDeviceCapability", getDeviceCapFunc);
}

auto getPluralCapFunc = [](napi_env env, void *data) {
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    RState state;
    if (asyncContext->resId_ != 0 || asyncContext->resource_ != nullptr) {
        int32_t resId = 0;
        std::shared_ptr<ResourceManager> resMgr = nullptr;
        bool ret = ResMgrAsyncContext::GetHapResourceManager(asyncContext, resMgr, resId);
        if (!ret) {
            LOGE("Failed to GetHapResourceManager in getPluralCapFunc");
            return;
        }
        state = resMgr->GetPluralStringByIdFormat(asyncContext->value_,
            resId, asyncContext->param_, asyncContext->param_);
    } else {
        state = asyncContext->addon_->GetResMgr()->GetPluralStringByNameFormat(asyncContext->value_,
            asyncContext->resName_.c_str(), asyncContext->param_, asyncContext->param_);
    }
    if (state != RState::SUCCESS) {
        asyncContext->SetErrorMsg("GetPluralString failed", true);
        return;
    }
    asyncContext->createValueFunc_ = [](napi_env env, ResMgrAsyncContext& context) -> napi_value {
        napi_value result;
        napi_status status = napi_create_string_utf8(env, context.value_.c_str(), NAPI_AUTO_LENGTH, &result);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create plural string");
            return nullptr;
        }
        return result;
    };
};

napi_value ResourceManagerAddon::ProcessIdNameParam(napi_env env, napi_callback_info info, const std::string& name,
    napi_async_execute_callback execute)
{
    GET_PARAMS(env, info, 3);

    std::unique_ptr<ResMgrAsyncContext> asyncContext = std::make_unique<ResMgrAsyncContext>();
    std::shared_ptr<ResourceManagerAddon> *addonPtr = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&addonPtr));
    if (status != napi_ok) {
        LOGE("Failed to unwrap ProcessIdNameParam %{public}s", name.c_str());
        return nullptr;
    }
    asyncContext->addon_ = *addonPtr;
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType;
        napi_typeof(env, argv[i], &valueType);

        if (i == 0 && valueType == napi_number) {
            asyncContext->resId_ = GetResId(env, argc, argv);
        } else if (i == 0 && valueType == napi_string) {
            asyncContext->resName_ = GetResNameOrPath(env, argc, argv);
        } else if (i == 0 && valueType == napi_object) {
            std::shared_ptr<ResourceManager::Resource> resourcePtr = std::make_shared<ResourceManager::Resource>();
            bool ret = GetResourceObject(env, resourcePtr, argv[0]);
            if (!ret) {
                LOGE("Failed to get native Resource object");
                return nullptr;
            }
            asyncContext->resource_ = resourcePtr;
        } else if (i == 1 && valueType == napi_number) {
            napi_get_value_int32(env, argv[i], &asyncContext->param_);
        } else if (i == 2 && valueType == napi_function) { // the third callback param
            napi_create_reference(env, argv[i], 1, &asyncContext->callbackRef_);
            break;
        } else {
            // self resourcemanager with promise
        }
    }
    napi_value result = ResMgrAsyncContext::getResult(env, asyncContext, name, execute);
    return result;
}

napi_value ResourceManagerAddon::GetPluralStringByName(napi_env env, napi_callback_info info)
{
    return ProcessIdNameParam(env, info, "GetPluralStringByName", getPluralCapFunc);
}

napi_value ResourceManagerAddon::GetPluralString(napi_env env, napi_callback_info info)
{
    return ProcessIdNameParam(env, info, "GetPluralString", getPluralCapFunc);
}

auto g_getRawFileFunc = [](napi_env env, void* data) {
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    asyncContext->addon_->GetResMgr()->GetRawFileFromHap(asyncContext->path_,
        asyncContext->len_, asyncContext->mediaData);
    CreateValue(asyncContext);
};

napi_value ResourceManagerAddon::GetRawFile(napi_env env, napi_callback_info info)
{
    return ProcessOnlyIdParam(env, info, "getRawFile", g_getRawFileFunc);
}

auto g_getRawFileDescriptorFunc = [](napi_env env, void* data) {
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    asyncContext->createValueFunc_ = [](napi_env env, ResMgrAsyncContext& context) -> napi_value {
        ResourceManager::RawFileDescriptor descriptor;
        RState state = context.addon_->GetResMgr()->GetRawFileDescriptorFromHap(context.path_, descriptor);
        if (state != RState::SUCCESS) {
            context.SetErrorMsg("GetRawFileDescriptor failed state", true);
            return nullptr;
        }
        napi_value result;
        napi_status status = napi_create_object(env, &result);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create result");
            return result;
        }

        napi_value fd;
        status = napi_create_int32(env, descriptor.fd, &fd);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create fd");
            return result;
        }
        status = napi_set_named_property(env, result, "fd", fd);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to set fd");
            return result;
        }

        napi_value offset;
        status = napi_create_int64(env, descriptor.offset, &offset);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create offset");
            return result;
        }
        status = napi_set_named_property(env, result, "offset", offset);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to set offset");
            return result;
        }

        napi_value length;
        status = napi_create_int64(env, descriptor.length, &length);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to create length");
            return result;
        }
        status = napi_set_named_property(env, result, "length", length);
        if (status != napi_ok) {
            context.SetErrorMsg("Failed to set length");
            return result;
        }
        return result;
    };
};

napi_value ResourceManagerAddon::GetRawFileDescriptor(napi_env env, napi_callback_info info)
{
    return ProcessOnlyIdParam(env, info, "getRawFileDescriptor", g_getRawFileDescriptorFunc);
}

auto closeRawFileDescriptorFunc = [](napi_env env, void* data) {
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    asyncContext->createValueFunc_ = [](napi_env env, ResMgrAsyncContext& context) -> napi_value {
        napi_value undefined;
        if (napi_get_undefined(env, &undefined) != napi_ok) {
            return nullptr;
        }
        RState state = context.addon_->GetResMgr()->CloseRawFileDescriptor(context.path_);
        if (state != RState::SUCCESS) {
            context.SetErrorMsg("CloseRawFileDescriptor failed state", true);
            return nullptr;
        }
        return undefined;
    };
};

napi_value ResourceManagerAddon::CloseRawFileDescriptor(napi_env env, napi_callback_info info)
{
    return ProcessOnlyIdParam(env, info, "closeRawFileDescriptor", closeRawFileDescriptorFunc);
}

std::shared_ptr<ResourceManagerAddon> getResourceManagerAddon(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 2);

    std::shared_ptr<ResourceManagerAddon> *addonPtr = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&addonPtr));
    if (status != napi_ok) {
        LOGE("Failed to unwrap");
        return nullptr;
    }
    return *addonPtr;
}

bool isNapiNumber(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 2);

    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_number) {
        LOGE("Parameter type is not napi_number");
        return false;
    }
    return true;
}

bool isNapiObject(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 2);

    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_object) {
        LOGE("Parameter type is not napi_object");
        return false;
    }
    return true;
}

bool isNapiString(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 2);

    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_string) {
        LOGE("Parameter type is not napi_string");
        return false;
    }
    return true;
}

napi_value ResourceManagerAddon::GetStringSync(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 2);

    std::unique_ptr<ResMgrAsyncContext> asyncContext = std::make_unique<ResMgrAsyncContext>();
    asyncContext->addon_ = getResourceManagerAddon(env, info);
    bool ret;
    if (isNapiNumber(env, info)) {
        asyncContext->resId_ = GetResId(env, argc, argv);
    } else if (isNapiObject(env, info)) {
        std::shared_ptr<ResourceManager::Resource> resourcePtr = std::make_shared<ResourceManager::Resource>();
        ret = GetResourceObject(env, resourcePtr, argv[0]);
        if (!ret) {
            LOGE("GetStringSync failed to get native Resource object");
            return nullptr;
        }
        asyncContext->resource_ = resourcePtr;
    } else {
        LOGE("GetStringSync type is invalid");
        return nullptr;
    }

    std::shared_ptr<ResourceManager> resMgr = nullptr;
    int32_t resId = 0;
    ret = ResMgrAsyncContext::GetHapResourceManager(asyncContext.get(), resMgr, resId);
    if (!ret) {
        LOGE("Failed to GetHapResourceManager in GetStringSync");
        return nullptr;
    }
    RState state = resMgr->GetStringById(resId, asyncContext->value_);
    if (state != RState::SUCCESS) {
        asyncContext->SetErrorMsg("GetStringSync failed state", true);
        return nullptr;
    }

    napi_value jsValue = nullptr;
    if (napi_create_string_utf8(env, asyncContext->value_.c_str(), NAPI_AUTO_LENGTH, &jsValue) != napi_ok) {
        asyncContext->SetErrorMsg("Failed to create jsValue");
    }
    return jsValue;
}

napi_value ResourceManagerAddon::GetStringByNameSync(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 2);

    if (!isNapiString(env, info)) {
        return nullptr;
    }

    std::unique_ptr<ResMgrAsyncContext> asyncContext = std::make_unique<ResMgrAsyncContext>();
    asyncContext->addon_ = getResourceManagerAddon(env, info);
    asyncContext->resName_ = GetResNameOrPath(env, argc, argv);

    RState state = asyncContext->addon_->GetResMgr()->GetStringByName(asyncContext->resName_.c_str(),
        asyncContext->value_);
    if (state != RState::SUCCESS) {
        asyncContext->SetErrorMsg("GetStringByNameSync failed state", true);
        return nullptr;
    }

    napi_value jsValue = nullptr;
    if (napi_create_string_utf8(env, asyncContext->value_.c_str(), NAPI_AUTO_LENGTH, &jsValue) != napi_ok) {
        asyncContext->SetErrorMsg("Failed to create jsValue");
    }
    return jsValue;
}

napi_value ResourceManagerAddon::GetBoolean(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 2);

    std::unique_ptr<ResMgrAsyncContext> asyncContext = std::make_unique<ResMgrAsyncContext>();
    asyncContext->addon_ = getResourceManagerAddon(env, info);
    bool ret;
    if (isNapiNumber(env, info)) {
        asyncContext->resId_ = GetResId(env, argc, argv);
    } else if (isNapiObject(env, info)) {
        std::shared_ptr<ResourceManager::Resource> resourcePtr = std::make_shared<ResourceManager::Resource>();
        ret = GetResourceObject(env, resourcePtr, argv[0]);
        if (!ret) {
            LOGE("GetBoolean failed to get native Resource object");
            return nullptr;
        }
        asyncContext->resource_ = resourcePtr;
    } else {
        LOGE("GetBoolean type is invalid");
        return nullptr;
    }
    std::shared_ptr<ResourceManager> resMgr = nullptr;
    int32_t resId = 0;
    ret = ResMgrAsyncContext::GetHapResourceManager(asyncContext.get(), resMgr, resId);
    if (!ret) {
        LOGE("Failed to GetHapResourceManager in GetBoolean");
        return nullptr;
    }
    RState state = resMgr->GetBooleanById(resId, asyncContext->bValue_);
    if (state != RState::SUCCESS) {
        asyncContext->SetErrorMsg("GetBoolean failed state", true);
        return nullptr;
    }

    napi_value jsValue = nullptr;
    if (napi_get_boolean(env, asyncContext->bValue_, &jsValue) != napi_ok) {
        asyncContext->SetErrorMsg("Failed to create result", true);
    }
    return jsValue;
}

napi_value ResourceManagerAddon::GetBooleanByName(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 2);

    if (!isNapiString(env, info)) {
        return nullptr;
    }

    std::unique_ptr<ResMgrAsyncContext> asyncContext = std::make_unique<ResMgrAsyncContext>();
    asyncContext->addon_ = getResourceManagerAddon(env, info);
    asyncContext->resName_ = GetResNameOrPath(env, argc, argv);

    RState state = asyncContext->addon_->GetResMgr()->GetBooleanByName(asyncContext->resName_.c_str(),
        asyncContext->bValue_);
    if (state != RState::SUCCESS) {
        asyncContext->SetErrorMsg("GetBooleanByName failed state", true);
        return nullptr;
    }

    napi_value jsValue = nullptr;
    if (napi_get_boolean(env, asyncContext->bValue_, &jsValue) != napi_ok) {
        asyncContext->SetErrorMsg("Failed to create result", true);
    }
    return jsValue;
}

napi_value ResourceManagerAddon::GetNumber(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 2);

    std::unique_ptr<ResMgrAsyncContext> asyncContext = std::make_unique<ResMgrAsyncContext>();
    asyncContext->addon_ = getResourceManagerAddon(env, info);
    bool ret;
    if (isNapiNumber(env, info)) {
        asyncContext->resId_ = GetResId(env, argc, argv);
    } else if (isNapiObject(env, info)) {
        std::shared_ptr<ResourceManager::Resource> resourcePtr = std::make_shared<ResourceManager::Resource>();
        ret = GetResourceObject(env, resourcePtr, argv[0]);
        if (!ret) {
            LOGE("GetNumber failed to get native Resource object");
            return nullptr;
        }
        asyncContext->resource_ = resourcePtr;
    } else {
        LOGE("GetNumber type is invalid");
        return nullptr;
    }
    std::shared_ptr<ResourceManager> resMgr = nullptr;
    int32_t resId = 0;
    ret = ResMgrAsyncContext::GetHapResourceManager(asyncContext.get(), resMgr, resId);
    if (!ret) {
        LOGE("Failed to GetHapResourceManager in GetNumber");
        return nullptr;
    }

    RState state = resMgr->GetIntegerById(resId, asyncContext->iValue_);
    napi_value jsValue = nullptr;
    if (state == RState::SUCCESS) {
        if (napi_create_int32(env, asyncContext->iValue_, &jsValue) != napi_ok) {
            asyncContext->SetErrorMsg("GetIntegerById failed to create result", true);
        }
    } else {
        state = asyncContext->addon_->GetResMgr()->GetFloatById(asyncContext->resId_,
        asyncContext->fValue_);
        if (state != RState::SUCCESS) {
            asyncContext->SetErrorMsg("GetFloatById failed state", true);
            return nullptr;
        }
        if (napi_create_double(env, asyncContext->fValue_, &jsValue) != napi_ok) {
            asyncContext->SetErrorMsg("GetFloatById failed to create result", true);
        }
    }
    return jsValue;
}

napi_value ResourceManagerAddon::GetNumberByName(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 2);

    if (!isNapiString(env, info)) {
        return nullptr;
    }

    std::unique_ptr<ResMgrAsyncContext> asyncContext = std::make_unique<ResMgrAsyncContext>();
    asyncContext->addon_ = getResourceManagerAddon(env, info);
    asyncContext->resName_ = GetResNameOrPath(env, argc, argv);

    RState state;
    napi_value jsValue = nullptr;
    state = asyncContext->addon_->GetResMgr()->GetIntegerByName(asyncContext->resName_.c_str(),
        asyncContext->iValue_);
    if (state == RState::SUCCESS) {
        if (napi_create_int32(env, asyncContext->iValue_, &jsValue) != napi_ok) {
            asyncContext->SetErrorMsg("GetIntegerByName failed to create result", true);
        }
    } else {
        state = asyncContext->addon_->GetResMgr()->GetFloatByName(asyncContext->resName_.c_str(),
        asyncContext->fValue_);
        if (state != RState::SUCCESS) {
            asyncContext->SetErrorMsg("GetFloatByName failed state", true);
            return nullptr;
        }
        if (napi_create_double(env, asyncContext->fValue_, &jsValue) != napi_ok) {
            asyncContext->SetErrorMsg("GetFloatByName failed to create result", true);
        }
    }
    return jsValue;
}

napi_valuetype ResourceManagerAddon::GetType(napi_env env, napi_value value)
{
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, value, &valueType);
    return valueType;
}

bool ResourceManagerAddon::GetResourceObjectName(napi_env env, std::shared_ptr<ResourceManager::Resource> &resourcePtr,
    napi_value &value, int32_t type)
{
    std::string typeName("moduleName");
    if (type == 0) {
        typeName = std::string("bundleName");
    }
    napi_value name;
    napi_status status = napi_get_named_property(env, value, typeName.c_str(), &name);
    if (status != napi_ok || name == nullptr) {
        LOGE("Failed to get resource name property");
        return false;
    }
    if (GetType(env, name) != napi_string) {
        LOGE("Failed to get resource name string");
        return false;
    }
    size_t len = 0;
    status = napi_get_value_string_utf8(env, name, nullptr, 0, &len);
    if (status != napi_ok || len <= 0) {
        LOGE("Failed to get resource len");
        return false;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, name, buf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("Failed to get resource name value");
        return false;
    }
    if (type == 0) {
        resourcePtr->bundleName = buf.data();
    } else {
        resourcePtr->moduleName = buf.data();
    }
    return true;
}

bool ResourceManagerAddon::GetResourceObjectId(napi_env env, std::shared_ptr<ResourceManager::Resource> &resourcePtr,
    napi_value &value)
{
    napi_value id;
    napi_status status = napi_get_named_property(env, value, "id", &id);
    if (status != napi_ok || id == nullptr) {
        LOGE("Failed to get resource id property");
        return false;
    }
    if (GetType(env, id) != napi_number) {
        LOGE("Failed to get resource id number");
        return false;
    }
    int32_t resId = 0;
    status = napi_get_value_int32(env, id, &resId);
    if (status != napi_ok) {
        LOGE("Failed to get resource id value");
        return false;
    }
    resourcePtr->id = resId;
    return true;
}

bool ResourceManagerAddon::GetResourceObject(napi_env env, std::shared_ptr<ResourceManager::Resource> &resourcePtr,
    napi_value &value)
{
    if (resourcePtr == nullptr) {
        LOGE("resourcePtr == nullptr");
        return false;
    }
    if (!GetResourceObjectName(env, resourcePtr, value, 0)) {
        LOGE("Failed to get bundleName");
        return false;
    }
    if (!GetResourceObjectName(env, resourcePtr, value, 1)) {
        LOGE("Failed to get moduleName");
        return false;
    }
    if (!GetResourceObjectId(env, resourcePtr, value)) {
        LOGE("Failed to get id");
        return false;
    }
    return true;
}

bool ResMgrAsyncContext::GetHapResourceManager(const ResMgrAsyncContext* asyncContext,
    std::shared_ptr<ResourceManager> &resMgr, int32_t &resId)
{
    std::shared_ptr<ResourceManager::Resource> resource = asyncContext->resource_;
    if (resource == nullptr) {
        resMgr = asyncContext->addon_->GetResMgr();
        resId = asyncContext->resId_;
        return true;
    }

    resId = resource->id;
    std::string key(resource->bundleName + "/" + resource->moduleName);
    std::lock_guard<std::mutex> lock(g_resMapLock);
    auto iter = g_resourceMgr.find(key);
    if (iter != g_resourceMgr.end()) {
        resMgr = g_resourceMgr[key];
        return true;
    }
#if defined(__ARKUI_CROSS__)
    g_resourceMgr[key] = asyncContext->addon_->GetResMgr();
#else
    auto context = asyncContext->addon_->GetContext();
    if (context == nullptr) {
        LOGE("GetHapResourceManager context == nullptr");
        return false;
    }
    auto moduleContext = context->CreateModuleContext(resource->bundleName, resource->moduleName);
    if (moduleContext == nullptr) {
        LOGE("GetHapResourceManager moduleContext == nullptr");
        return false;
    }
    resMgr = moduleContext->GetResourceManager();
    g_resourceMgr[key] = resMgr;
#endif
    return true;
}

napi_value ResMgrAsyncContext::getResult(napi_env env, std::unique_ptr<ResMgrAsyncContext> &asyncContext,
    const std::string &name, napi_async_execute_callback &execute)
{
    napi_value result = nullptr;
    if (asyncContext->callbackRef_ == nullptr) {
        napi_create_promise(env, &asyncContext->deferred_, &result);
    } else {
        napi_get_undefined(env, &result);
    }
    napi_value resource = nullptr;
    napi_create_string_utf8(env, name.c_str(), NAPI_AUTO_LENGTH, &resource);
    if (napi_create_async_work(env, nullptr, resource, execute, ResMgrAsyncContext::Complete,
        static_cast<void*>(asyncContext.get()), &asyncContext->work_) != napi_ok) {
        LOGE("Failed to create async work for %{public}s", name.c_str());
        return result;
    }
    if (napi_queue_async_work(env, asyncContext->work_) != napi_ok) {
        LOGE("Failed to queue async work for %{public}s", name.c_str());
        return result;
    }
    asyncContext.release();
    return result;
}
} // namespace Resource
} // namespace Plugin
} // namespace OHOS