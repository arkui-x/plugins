/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "native_module_ohos_media.h"
#include "media_log.h"
#ifdef ANDROID_PLATFORM
#include "player_jni.h"
#include "recorder_jni.h"
#include "avmetadatahelper_jni.h"
#include "plugin_utils.h"
#endif

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN_PLAYER, "JSMediaModule"};
}

/*
 * Function registering all props and functions of ohos.media module
 * which involves player and the recorder
 */
static napi_value Export(napi_env env, napi_value exports)
{
    MEDIA_LOGD("Export() is called");

    OHOS::Media::MediaEnumNapi::Init(env, exports);

#ifdef SUPPORT_METADATA
    OHOS::Media::AVMetadataExtractorNapi::Init(env, exports);
#endif
#ifdef SUPPORT_PLAYER_API9
    OHOS::Media::AVPlayerNapi::Init(env, exports);
#endif
#ifdef SUPPORT_RECORDER_API9
    OHOS::Media::AVRecorderNapi::Init(env, exports);
#endif

    return exports;
}

/*
 * module define
 */
static napi_module g_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Export,
    .nm_modname = "multimedia.media",
    .nm_priv = (reinterpret_cast<void*>(0)),
    .reserved = {0}
};

#ifdef ANDROID_PLATFORM
static void MediaJniRegister()
{
    const char player_className[] = "ohos.ace.plugin.playerplugin.PlayerPlugin";
    const char recorder_className[] = "ohos.ace.plugin.recorderplugin.RecorderPlugin";
    const char helper_className[] = "ohos.ace.plugin.avmetadatahelperplugin.AVMetadataHelperPlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&OHOS::Plugin::PlayerJni::Register, player_className);
    ARKUI_X_Plugin_RegisterJavaPlugin(&OHOS::Plugin::RecorderJni::Register, recorder_className);
    ARKUI_X_Plugin_RegisterJavaPlugin(&OHOS::Plugin::AVMetadataHelperJni::Register, helper_className);
}
#endif

/*
 * module register
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
#ifdef ANDROID_PLATFORM
    MediaJniRegister();
#endif
    MEDIA_LOGD("RegisterModule() is called");
    napi_module_register(&g_module);
}
