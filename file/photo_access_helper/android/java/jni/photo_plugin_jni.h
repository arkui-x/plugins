/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_PHOTO_PLUGIN_ANDROID_JAVA_JNI_PHOTO_PLUGIN_JNI_H
#define PLUGINS_PHOTO_PLUGIN_ANDROID_JAVA_JNI_PHOTO_PLUGIN_JNI_H

#include <jni.h>
#include <memory>
#include <vector>

#include "plugins/file/photo_access_helper/android/album_result_set.h"
#include "plugins/file/photo_access_helper/android/photo_result_set.h"
#include "rdb_predicates.h"

namespace OHOS::Plugin {
class PhotoPluginJni final {
public:
    PhotoPluginJni() = delete;
    ~PhotoPluginJni() = delete;
    static bool Register(void* env);

    // Called by Java
    static void NativeInit(JNIEnv* env, jobject jobj);
    static void onPickerResult(JNIEnv* env, jobject thiz, jobject rst, jint errCode);

    // Called by C++
    static void startPhotoPicker(std::string &type);
    static std::shared_ptr<Media::ResultSet> queryPhoto(const NativeRdb::RdbPredicates &queryArgs,
        std::vector<std::string> &selectionArgs, std::vector<std::string> &projection);
    static std::shared_ptr<Media::ResultSet> queryAlbum(const NativeRdb::RdbPredicates &queryArgs,
        std::vector<std::string> &selectionArgs, std::vector<std::string> &projection);
    static std::string CreatePhoto(int photoType, const std::string &extension, const std::string &title);
    static bool checkPermission();
    static bool CheckWritePermission();
    static std::string GetMimeTypeFromExtension(const std::string &extension);
};
} // namespace OHOS::Plugin
#endif // PLUGINS_PHOTO_PLUGIN_ANDROID_JAVA_JNI_PHOTO_PLUGIN_JNI_H
