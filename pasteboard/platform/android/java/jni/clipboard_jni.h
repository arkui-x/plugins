/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
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

#ifndef PLUGINS_PASTEBOARD_PLATFORM_ANDROID_JAVA_JNI_CLIPBOARD_CLIPBOARD_JNI_H
#define PLUGINS_PASTEBOARD_PLATFORM_ANDROID_JAVA_JNI_CLIPBOARD_CLIPBOARD_JNI_H

#include <functional>
#include <jni.h>
#include <memory>
#include <string>
#include <vector>

#include "paste_data.h"
#include "pasteboard_types.h"

namespace OHOS::Plugin {

class ClipboardJni {
public:
    static bool Register(void* env);

    static void NativeInit(JNIEnv* env, jobject object);

    static int32_t SetData(const OHOS::MiscServices::PasteData& pasteData);

    static int32_t GetData(OHOS::MiscServices::PasteData& pasteData);

    static bool HasData();

    static bool HasPasteData();

    static bool Clear();

    static bool Subscribe();

    static bool Unsubscribe();

    static void OnPasteboardChanged();

    static int DetectPatterns(
        const std::vector<MiscServices::Pattern>& patternsToCheck, std::vector<MiscServices::Pattern>& funcResult);

    static bool HasDataType(const std::string& mimeType);

    static bool GetMimeTypes(std::vector<std::string>& funcResult);

    ClipboardJni() = delete;

    ~ClipboardJni() = delete;

private:
    static void PasteDataDTOInit(JNIEnv* env);

    static void DataRecordDTOInit(JNIEnv* env);

    static void PropertyDTOInit(JNIEnv* env);

    ClipboardJni(const ClipboardJni&) = delete;

    ClipboardJni(ClipboardJni&&) = delete;

    ClipboardJni& operator=(const ClipboardJni&) = delete;

    ClipboardJni& operator=(ClipboardJni&&) = delete;
};
} // namespace OHOS::Plugin
#endif // PLUGINS_PASTEBOARD_PLATFORM_ANDROID_JAVA_JNI_CLIPBOARD_CLIPBOARD_JNI_H