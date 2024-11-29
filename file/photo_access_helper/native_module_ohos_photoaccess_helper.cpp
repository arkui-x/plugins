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

#include "photo_album_napi.h"
#include "fetch_file_result_napi.h"
#include "file_asset_napi.h"
#include "media_library_napi.h"
#include "plugin_utils.h"
#include "photo_picker_napi.h"
#ifdef ANDROID_PLATFORM
#include "android/java/jni/photo_plugin_jni.h"
#endif
#include "log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace Media {
/*
 * Function registering all props and functions of userfilemanager module
 */
static napi_value PhotoAccessHelperExport(napi_env env, napi_value exports)
{
    MediaLibraryNapi::PhotoAccessHelperInit(env, exports);
    PhotoPickerNapi::PhotoPickerInit(env, exports);
    FetchFileResultNapi::PhotoAccessHelperInit(env, exports);
    FileAssetNapi::PhotoAccessHelperInit(env, exports);
    PhotoAlbumNapi::PhotoAccessInit(env, exports);
    return exports;
}

#ifdef ANDROID_PLATFORM
static void PhotoAccessHelperJniRegister()
{
   const char photoPluginName[] = "ohos.ace.plugin.photoaccesshelper.PhotoPlugin";
   ARKUI_X_Plugin_RegisterJavaPlugin(&Plugin::PhotoPluginJni::Register, photoPluginName);
}
#endif

/*
 * module define
 */
static napi_module g_photoAccessHelperModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = PhotoAccessHelperExport,
    .nm_modname = "file.photoAccessHelper",
    .nm_priv = reinterpret_cast<void *>(0),
    .reserved = {0}
};

/*
 * module register
 */
extern "C" __attribute__((constructor)) void RegisterPhotoAccessHelper(void)
{
#ifdef ANDROID_PLATFORM
    PhotoAccessHelperJniRegister();
#endif
    napi_module_register(&g_photoAccessHelperModule);
}
} // namespace Media
} // namespace OHOS