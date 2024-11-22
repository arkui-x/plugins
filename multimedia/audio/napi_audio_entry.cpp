/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_audio_enum.h"
#include "napi_audio_renderer.h"
#include "napi_audio_capturer.h"
#include "napi_audio_manager.h"
#include "napi_audio_routing_manager.h"
#include "napi_audio_stream_manager.h"
#include "napi_audio_volume_group_manager.h"
#include "napi_audio_volume_manager.h"
#include "napi_audio_interrupt_manager.h"
#include "napi_audio_spatialization_manager.h"

#ifdef ANDROID_PLATFORM
#include "audiocapturer/java/jni/audio_capturer_jni.h"
#include "audiorenderer/java/jni/audio_renderer_jni.h"
#include "audiomanager/java/jni/audio_manager_jni.h"
#include "plugin_utils.h"
#endif

namespace OHOS {
namespace AudioStandard {
static napi_value Init(napi_env env, napi_value exports)
{
    /* 各模块新增Init接口添加 */
    NapiAudioEnum::Init(env, exports);
    NapiAudioRenderer::Init(env, exports);
    NapiAudioCapturer::Init(env, exports);
    NapiAudioStreamMgr::Init(env, exports);
    NapiAudioRoutingManager::Init(env, exports);
    NapiAudioVolumeGroupManager::Init(env, exports);
    NapiAudioVolumeManager::Init(env, exports);
    NapiAudioInterruptManager::Init(env, exports);
    NapiAudioSpatializationManager::Init(env, exports);
    NapiAudioManager::Init(env, exports);
    return exports;
}

static napi_module g_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "multimedia.audio",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

#ifdef ANDROID_PLATFORM
static void AudioJniRegister()
{
    const char className[] = "ohos.ace.plugin.audiorenderer.AudioRendererPlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&Plugin::AudioRendererJni::Register, className);
    const char className2[] = "ohos.ace.plugin.audiocapturer.AudioCapturerPlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&Plugin::AudioCapturerJni::Register, className2);
    const char className3[] = "ohos.ace.plugin.audiomanager.AudioManagerPlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&Plugin::AudioManagerJni::Register, className3);
}
#endif

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
#ifdef ANDROID_PLATFORM
    AudioJniRegister();
#endif
    napi_module_register(&g_module);
}
} // namespace AudioStandard
} // namespace OHOS
