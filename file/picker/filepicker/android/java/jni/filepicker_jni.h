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

#ifndef PLUGINS_FILE_PICKER_ANDROID_JAVA_JNI_H
#define PLUGINS_FILE_PICKER_ANDROID_JAVA_JNI_H

#include <jni.h>

#include "../../../native_document_view_picker.h"
#include "inner_api/plugin_utils_inner.h"
#include "plugin_utils.h"

namespace OHOS::Plugin {
class FilePickerJni final {
public:
    FilePickerJni() = delete;
    ~FilePickerJni() = delete;
    static bool Register(void* env);

    static void NativeInit(JNIEnv* env, jobject jobj);
    static void onPickerResult(JNIEnv* env, jobject thiz, jobject rst, jint errCode);

    static void Select(DocumentSelectOptions& options);
    static void Save(DocumentSaveOptions& options);
};
} // namespace OHOS::Plugin

#endif // PLUGINS_FILE_PICKER_ANDROID_JAVA_JNI_H
