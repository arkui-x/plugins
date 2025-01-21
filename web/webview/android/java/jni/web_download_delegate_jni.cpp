/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "web_download_delegate_jni.h"

#include <jni.h>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"
#include "plugins/web/webview/web_message_port.h"

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
namespace {
const char WEB_WEBVIEW_CLASS_NAME[] = "ohos/ace/adapter/capability/web/AceWebPluginBase";

static const JNINativeMethod METHODS[] = {
    {      
        "onBeforeDownloadObject",
        "(JLjava/lang/Object;)V",
        reinterpret_cast<void*>(WebDownloadDelegateJni::OnBeforeDownloadObject)
    },
    {      
        "onDownloadUpdatedObject",
        "(JLjava/lang/Object;)V",
        reinterpret_cast<void*>(WebDownloadDelegateJni::OnDownloadUpdatedObject)
    },
    {      
        "onDownloadFailedObject",
        "(JLjava/lang/Object;)V",
        reinterpret_cast<void*>(WebDownloadDelegateJni::OnDownloadFailedObject)
    },
    {      
        "onDownloadFinishObject",
        "(JLjava/lang/Object;)V",
        reinterpret_cast<void*>(WebDownloadDelegateJni::OnDownloadFinishObject)
    },
};
}

bool WebDownloadDelegateJni::Register(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(WEB_WEBVIEW_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    return ret;
}

void WebDownloadDelegateJni::OnBeforeDownloadObject(JNIEnv* env, jobject clazz, jlong id, jobject object)
{
    if (env != nullptr && object != nullptr) {
        WebDownloadManager::OnBeforeDownload(id, (void*)&object);
    }
}

void WebDownloadDelegateJni::OnDownloadUpdatedObject(JNIEnv* env, jobject clazz, jlong id, jobject object)
{
    if (env != nullptr && object != nullptr) {
        WebDownloadManager::OnUpdatedDownload(id, (void*)&object);
    }
}

void WebDownloadDelegateJni::OnDownloadFailedObject(JNIEnv* env, jobject clazz, jlong id, jobject object)
{
    if (env != nullptr && object != nullptr) {
        WebDownloadManager::OnFailedDownload(id, (void*)&object);
    }
}

void WebDownloadDelegateJni::OnDownloadFinishObject(JNIEnv* env, jobject clazz, jlong id, jobject object)
{
    if (env != nullptr && object != nullptr) {
        WebDownloadManager::OnFinishDownload(id, (void*)&object);
    }
}
}
