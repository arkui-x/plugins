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

#include "time_jni.h"

#include <algorithm>
#include <cctype>
#include <jni.h>
#include <string>

#include "plugin_utils.h"
#include "time_common.h"
#include "time_hilog.h"
using namespace OHOS::MiscServices;

namespace OHOS::Time {
namespace {
const char TIME_CLASS_NAME[] = "ohos/ace/plugin/timeplugin/TimePlugin";
static const JNINativeMethod METHODS[] = {
    {
        "nativeInit",
        "()V",
        reinterpret_cast<void*>(&TimeJni::NativeInit)
    },
};

static const char GET_TIME_ZONE_METHOD[] = "getTimeZone";
static const char GET_TIME_ZONE_METHOD_PARAM[] = "([Ljava/lang/String;)I";

struct {
    jobject globalRef;
    jmethodID getTimeZone;
} g_timeClass;
}

bool TimeJni::Register(void* env)
{
    TIME_HILOGI(TIME_MODULE_JNI, "TiemJni Register start");
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_AND_RETURN_RET_LOG(TIME_MODULE_JNI, jniEnv, false);
    jclass cls = jniEnv->FindClass(TIME_CLASS_NAME);
    CHECK_AND_RETURN_RET_LOG(TIME_MODULE_JNI, cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        TIME_HILOGE(TIME_MODULE_JNI, "TiemJni JNI: RegisterNatives fail.");
        return false;
    }
    TIME_HILOGI(TIME_MODULE_JNI, "TiemJni Register end");
    return true;
}

void TimeJni::NativeInit(JNIEnv* env, jobject jobj)
{
    TIME_HILOGI(TIME_MODULE_JNI, "TiemJni NativeInit start");
    CHECK_AND_RETURN_LOG(TIME_MODULE_JNI, env);
    g_timeClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_AND_RETURN_LOG(TIME_MODULE_JNI, g_timeClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_AND_RETURN_LOG(TIME_MODULE_JNI, cls);

    g_timeClass.getTimeZone = env->GetMethodID(cls, GET_TIME_ZONE_METHOD, GET_TIME_ZONE_METHOD_PARAM);
    CHECK_AND_RETURN_LOG(TIME_MODULE_JNI, g_timeClass.getTimeZone);

    env->DeleteLocalRef(cls);
    TIME_HILOGI(TIME_MODULE_JNI, "TiemJni NativeInit end");
}

int32_t TimeJni::GetTimeZone(std::string &timezoneId)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_AND_RETURN_RET_LOG(TIME_MODULE_JNI, env, E_TIME_NULLPTR);
    CHECK_AND_RETURN_RET_LOG(TIME_MODULE_JNI, g_timeClass.globalRef, E_TIME_NULLPTR);
    CHECK_AND_RETURN_RET_LOG(TIME_MODULE_JNI, g_timeClass.getTimeZone, E_TIME_NULLPTR);
    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray javaArray = env->NewObjectArray(1, stringClass, nullptr);
    jint retJint = env->CallIntMethod(g_timeClass.globalRef, g_timeClass.getTimeZone, javaArray);
    int ret = static_cast<int>(retJint);
    if (ret == E_TIME_OK) {
        jstring stringElement = (jstring)env->GetObjectArrayElement(javaArray, 0);
        const char* pStringElement = env->GetStringUTFChars(stringElement, nullptr);
        if (pStringElement != nullptr) {
            timezoneId = pStringElement;
            env->ReleaseStringUTFChars(stringElement, pStringElement);
        } else {
            ret = E_TIME_DEAL_FAILED;
        }
        env->DeleteLocalRef(stringElement);
    }
    env->DeleteLocalRef(javaArray);
    return ret;
}
} // OHOS::Time 