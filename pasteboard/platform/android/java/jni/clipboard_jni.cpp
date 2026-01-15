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

#include "clipboard_jni.h"

#include <cstddef>
#include <regex>

#include "clipboard_proxy.h"
#include "errors.h"
#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "pasteboard_error.h"
#include "plugin_utils.h"
#include "want_params_wrapper.h"

#define CHECK_NULL_VOID_RELEASE_CLAZZ(ptr, env, clazz)                      \
    do {                                                                    \
        if (!(ptr)) {                                                       \
            LOGW(#ptr " is null, release clazz and return on line %{public}d", __LINE__); \
            if ((env) != nullptr && (clazz) != nullptr) {                   \
                (env)->DeleteLocalRef((clazz));                             \
            }                                                               \
            return;                                                         \
        }                                                                   \
    } while (0)

using namespace OHOS::MiscServices;
namespace OHOS::Plugin {
namespace {
static const char CLIPBOARD_PLUGIN_CLASS_NAME[] = "ohos/ace/plugin/clipboard/ClipboardAosp";
static const char PASTE_DATA_DTO_CLASS_NAME[] = "ohos/ace/plugin/clipboard/PasteDataDTO";
static const char PASTE_DATA_RECORD_DTO_CLASS_NAME[] = "ohos/ace/plugin/clipboard/DataRecordDTO";
static const char PASTE_DATA_PROPERTY_DTO_CLASS_NAME[] = "ohos/ace/plugin/clipboard/PropertyDTO";

static const JNINativeMethod METHODS[] = {
    { .name = "nativeInit", .signature = "()V", .fnPtr = reinterpret_cast<void*>(ClipboardJni::NativeInit) },
    { .name = "onPasteboardChanged",
        .signature = "()V",
        .fnPtr = reinterpret_cast<void*>(ClipboardJni::OnPasteboardChanged) },
};

static const char METHOD_SET_DATA[] = "setData";
static const char METHOD_GET_DATA[] = "getData";
static const char METHOD_HAS_DATA[] = "hasData";
static const char METHOD_CLEAR[] = "clear";
static const char METHOD_HAS_DATA_TYPE[] = "hasDataType";
static const char METHOD_SUBSCRIBE_PASTEBOARD_CHANGE[] = "subscribePasteboardChange";
static const char METHOD_UNSUBSCRIBE_PASTEBOARD_CHANGE[] = "unsubscribePasteboardChange";
static const char METHOD_DETECT_PATTERNS[] = "detectPatterns";

static const char METHOD_SET_PROPERTY_DTO[] = "setPropertyDTO";
static const char METHOD_SET_RECORDS[] = "setRecords";
static const char METHOD_GET_PROPERTY_DTO[] = "getPropertyDTO";
static const char METHOD_GET_RECORDS[] = "getRecords";

static const char METHOD_SET_MIMETYPE[] = "setMimeType";
static const char METHOD_SET_HTMLTEXT[] = "setHtmlText";
static const char METHOD_SET_JSON_WANT[] = "setJsonWant";
static const char METHOD_SET_PLAINTEXT[] = "setPlainText";
static const char METHOD_SET_URI[] = "setUri";
static const char METHOD_GET_MIMETYPE[] = "getMimeType";
static const char METHOD_GET_HTMLTEXT[] = "getHtmlText";
static const char METHOD_GET_JSON_WANT[] = "getJsonWant";
static const char METHOD_GET_PLAINTEXT[] = "getPlainText";
static const char METHOD_GET_URI[] = "getUri";

static const char METHOD_SET_MIMETYPES[] = "setMimeTypes";
static const char METHOD_SET_TAG[] = "setTag";
static const char METHOD_SET_TIMESTAMP[] = "setTimestamp";
static const char METHOD_SET_JSON_ADDITIONS[] = "setJsonAdditions";
static const char METHOD_GET_MIMETYPES[] = "getMimeTypes";
static const char METHOD_GET_TAG[] = "getTag";
static const char METHOD_GET_TIMESTAMP[] = "getTimestamp";
static const char METHOD_GET_JSON_ADDITIONS[] = "getJsonAdditions";

static const char SIGNATURE_SET_DATA[] = "(Lohos/ace/plugin/clipboard/PasteDataDTO;)I";
static const char SIGNATURE_GET_DATA[] = "()Lohos/ace/plugin/clipboard/PasteDataDTO;";
static const char SIGNATURE_HAS_DATA[] = "()Z";
static const char SIGNATURE_HAS_DATA_TYPE[] = "(Ljava/lang/String;)Z";
static const char SIGNATURE_CLEAR[] = "()Z";
static const char SIGNATURE_SUBSCRIBE_PASTEBOARD_CHANGE[] = "()Z";
static const char SIGNATURE_UNSUBSCRIBE_PASTEBOARD_CHANGE[] = "()Z";
static const char SIGNATURE_DETECT_PATTERNS[] = "(I)I";

static const char SIGNATURE_SET_PROPERTY_DTO[] = "(Lohos/ace/plugin/clipboard/PropertyDTO;)V";
static const char SIGNATURE_SET_RECORDS[] = "([Lohos/ace/plugin/clipboard/DataRecordDTO;)V";
static const char SIGNATURE_GET_PROPERTY_DTO[] = "()Lohos/ace/plugin/clipboard/PropertyDTO;";
static const char SIGNATURE_GET_RECORDS[] = "()[Lohos/ace/plugin/clipboard/DataRecordDTO;";

static const char SIGNATURE_SET_MIMETYPE[] = "(Ljava/lang/String;)V";
static const char SIGNATURE_SET_HTMLTEXT[] = "(Ljava/lang/String;)V";
static const char SIGNATURE_SET_JSON_WANT[] = "(Ljava/lang/String;)V";
static const char SIGNATURE_SET_PLAINTEXT[] = "(Ljava/lang/String;)V";
static const char SIGNATURE_SET_URI[] = "(Ljava/lang/String;)V";
static const char SIGNATURE_GET_MIMETYPE[] = "()Ljava/lang/String;";
static const char SIGNATURE_GET_HTMLTEXT[] = "()Ljava/lang/String;";
static const char SIGNATURE_GET_JSON_WANT[] = "()Ljava/lang/String;";
static const char SIGNATURE_GET_PLAINTEXT[] = "()Ljava/lang/String;";
static const char SIGNATURE_GET_URI[] = "()Ljava/lang/String;";

static const char SIGNATURE_SET_MIMETYPES[] = "([Ljava/lang/String;)V";
static const char SIGNATURE_SET_TAG[] = "(Ljava/lang/String;)V";
static const char SIGNATURE_SET_TIMESTAMP[] = "(J)V";
static const char SIGNATURE_SET_JSON_ADDITIONS[] = "(Ljava/lang/String;)V";
static const char SIGNATURE_GET_MIMETYPES[] = "()[Ljava/lang/String;";
static const char SIGNATURE_GET_TAG[] = "()Ljava/lang/String;";
static const char SIGNATURE_GET_TIMESTAMP[] = "()J";
static const char SIGNATURE_GET_JSON_ADDITIONS[] = "()Ljava/lang/String;";

struct {
    jmethodID setData;
    jmethodID getData;
    jmethodID hasData;
    jmethodID clear;
    jmethodID subscribePasteboardChange;
    jmethodID unsubscribePasteboardChange;
    jmethodID detectPatterns;
    jmethodID hasDataType;
    jmethodID getMimeTypes;
    jobject globalRef;
} g_clipboard;

struct {
    jmethodID constructor;
    jmethodID getPropertyDTO;
    jmethodID getRecords;
    jmethodID setPropertyDTO;
    jmethodID setRecords;
    jclass clazz;
} g_pasteDataDTO;

struct PropertyDTOCache {
    jmethodID constructor;
    jmethodID setMimeTypes;
    jmethodID setTag;
    jmethodID setTimestamp;
    jmethodID setJsonAdditions;
    jmethodID getMimeTypes;
    jmethodID getTag;
    jmethodID getTimestamp;
    jmethodID getJsonAdditions;
    jclass clazz;
} g_propertyDTO;

struct DataRecordDTOCache {
    jmethodID constructor;
    jmethodID setMimeType;
    jmethodID setHtmlText;
    jmethodID setJsonWant;
    jmethodID setPlainText;
    jmethodID setUri;
    jmethodID getMimeType;
    jmethodID getHtmlText;
    jmethodID getJsonWant;
    jmethodID getPlainText;
    jmethodID getUri;
    jclass clazz;
} g_dataRecordDTO;

static bool SetJavaStringField(JNIEnv* env, jobject jObj, jmethodID setter, const std::string& value)
{
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(jObj, false);
    CHECK_NULL_RETURN(setter, false);

    jstring jStr = env->NewStringUTF(value.c_str());
    CHECK_NULL_RETURN(jStr, false);
    env->CallVoidMethod(jObj, setter, jStr);
    bool hasException = env->ExceptionCheck();
    if (hasException) {
        LOGE("SetJavaStringField: call setter failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    env->DeleteLocalRef(jStr);
    return !hasException;
}

std::shared_ptr<std::string> GetPlainTextFromRecord(const std::shared_ptr<OHOS::MiscServices::PasteDataRecord>& record)
{
    CHECK_NULL_RETURN(record, nullptr);
    auto plainText = record->GetPlainText();
    if (plainText) {
        return plainText;
    }
    auto type = MiscServices::CommonUtils::Convert(record->GetUDType(), record->GetMimeType());
    if (type == UDMF::HTML) {
        std::string res;
        auto entry = record->GetEntryByMimeType(MIMETYPE_TEXT_HTML);
        if (entry == nullptr) {
            LOGE("GetPlainTextFromRecord: get entry failed");
            return nullptr;
        }
        auto value = entry->GetValue();
        if (!std::holds_alternative<std::shared_ptr<Object>>(value)) {
            LOGE("GetPlainTextFromRecord: value error, no Object");
            return nullptr;
        }
        auto object = std::get<std::shared_ptr<Object>>(value);
        if (object == nullptr) {
            LOGE("GetPlainTextFromRecord: can not find object in value");
            return nullptr;
        }
        if (!object->GetValue(UDMF::PLAIN_CONTENT, res)) {
            LOGE("GetPlainTextFromRecord: get PLAIN_CONTENT failed");
            return nullptr;
        }
        return std::make_shared<std::string>(res);
    }
    return nullptr;
}

bool SetWantToJavaDataRecordDTO(
    JNIEnv* env, jobject jRecord, const std::shared_ptr<OHOS::MiscServices::PasteDataRecord> record)
{
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(jRecord, false);
    CHECK_NULL_RETURN(record, false);
    CHECK_NULL_RETURN(g_dataRecordDTO.setJsonWant, false);
    std::shared_ptr<OHOS::AAFwk::Want> wantPtr = record->GetWant();
    CHECK_NULL_RETURN(wantPtr, true);

    std::string jsonWantStr = wantPtr->ToJson();
    nlohmann::json jsonObject = nlohmann::json::parse(jsonWantStr.c_str(), nullptr, false);
    if (jsonObject.is_discarded()) {
        LOGE("jsonObject is discarded. value = %{public}s", jsonWantStr.c_str());
        return false;
    }
    if (!wantPtr->GetBundleName().empty()) {
        jsonObject["bundleName"] = wantPtr->GetBundleName();
    }
    if (!wantPtr->GetModuleName().empty()) {
        jsonObject["moduleName"] = wantPtr->GetModuleName();
    }
    if (!wantPtr->GetAbilityName().empty()) {
        jsonObject["abilityName"] = wantPtr->GetAbilityName();
    }
    if (!wantPtr->GetType().empty()) {
        jsonObject["type"] = wantPtr->GetType();
    }

    std::string jsonWant = jsonObject.dump();
    jstring jJsonWant = env->NewStringUTF(jsonWant.c_str());
    CHECK_NULL_RETURN(jJsonWant, false);

    env->CallVoidMethod(jRecord, g_dataRecordDTO.setJsonWant, jJsonWant);
    env->DeleteLocalRef(jJsonWant);
    if (env->ExceptionCheck()) {
        LOGE("SetWantToJavaDataRecordDTO: call setJsonWant failed");
        env->ExceptionDescribe();
        env->ExceptionClear();    
        return false;
    }
    return true;
}

jobject ToJavaDataRecordDTO(JNIEnv* env, const std::shared_ptr<OHOS::MiscServices::PasteDataRecord>& record)
{
    CHECK_NULL_RETURN(env, nullptr);
    CHECK_NULL_RETURN(record, nullptr);
    CHECK_NULL_RETURN(g_dataRecordDTO.constructor, nullptr);
    CHECK_NULL_RETURN(g_dataRecordDTO.clazz, nullptr);

    jobject jRecord = env->NewObject(g_dataRecordDTO.clazz, g_dataRecordDTO.constructor);
    if (!jRecord || env->ExceptionCheck()) {
        LOGE("ToJavaDataRecordDTO: create DataRecordDTO failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }

    std::string mimeType = record->GetMimeType();
    if (!SetJavaStringField(env, jRecord, g_dataRecordDTO.setMimeType, mimeType)) {
        return nullptr;
    }
    std::shared_ptr<std::string> htmlTextPtr = record->GetHtmlText();
    if (htmlTextPtr != nullptr) {
        if (!SetJavaStringField(env, jRecord, g_dataRecordDTO.setHtmlText, *htmlTextPtr)) {
            return nullptr;
        }
    }
    if (!SetWantToJavaDataRecordDTO(env, jRecord, record)) {
        return nullptr;
    }
    std::shared_ptr<std::string> plainTextPtr = GetPlainTextFromRecord(record);
    if (plainTextPtr != nullptr) {
        if (!SetJavaStringField(env, jRecord, g_dataRecordDTO.setPlainText, *plainTextPtr)) {
            return nullptr;
        }
    }
    std::shared_ptr<OHOS::Uri> uri = record->GetUri();
    if (uri != nullptr) {
        std::string uriText = uri->ToString();
        if (!SetJavaStringField(env, jRecord, g_dataRecordDTO.setUri, uriText)) {
            return nullptr;
        }
    }

    return jRecord;
}

jobject ToJavaDataRecordDTOArray(
    JNIEnv* env, const std::vector<std::shared_ptr<OHOS::MiscServices::PasteDataRecord>>& records)
{
    CHECK_NULL_RETURN(env, nullptr);
    if (records.empty()) {
        LOGE("ToJavaDataRecordDTOArray: empty records");
        return nullptr;
    }

    jsize arrayLength = static_cast<jsize>(records.size());
    if (!g_pasteDataDTO.clazz) {
        LOGE("ToJavaDataRecordDTOArray: null pasteDataRecordData class");
        return nullptr;
    }
    jobjectArray jRecordArray = env->NewObjectArray(arrayLength, g_dataRecordDTO.clazz, nullptr);
    if (!jRecordArray) {
        LOGE("ToJavaDataRecordDTOArray: create DataRecordDTO array failed");
        return nullptr;
    }

    for (jsize i = 0; i < arrayLength; ++i) {
        const auto& record = records[i];
        jobject jRecord = ToJavaDataRecordDTO(env, record);
        if (jRecord) {
            env->SetObjectArrayElement(jRecordArray, i, jRecord);
            env->DeleteLocalRef(jRecord);
        } else {
            LOGE("ToJavaDataRecordDTOArray: convert single record failed at index %d", i);
            env->DeleteLocalRef(jRecordArray);
            return nullptr;
        }
    }
    return jRecordArray;
}

AAFwk::WantParams JsonToWantParams(const std::string& jsonParams)
{
    auto jsonObject = nlohmann::json::parse(jsonParams.c_str(), nullptr, false);
    AAFwk::WantParams wantParams;
    if (jsonObject.is_discarded()) {
        LOGE("JsonToWantParams: jsonObject is discarded. value = %{public}s", jsonParams.c_str());
        return wantParams;
    }
    if (jsonObject.contains("params")) {
        AAFwk::WantParamWrapper::ParseWantParams(jsonObject["params"], wantParams);
    }
    return wantParams;
}

std::string WantParamsToJson(AAFwk::WantParams Params)
{
    AAFwk::WantParamWrapper wrapper(Params);
    std::string jsonParams = "{\"" + AAFwk::JSON_WANTPARAMS_PARAM + "\":" + wrapper.ToString() + "}";
    return jsonParams;
}

jobject ToJavaPropertyDTO(JNIEnv* env, const OHOS::MiscServices::PasteDataProperty& property)
{
    CHECK_NULL_RETURN(env, nullptr);
    CHECK_NULL_RETURN(g_propertyDTO.constructor, nullptr);
    CHECK_NULL_RETURN(g_propertyDTO.clazz, nullptr);

    jobject jProperty = env->NewObject(g_propertyDTO.clazz, g_propertyDTO.constructor);
    if (!jProperty || env->ExceptionCheck()) {
        LOGE("ToJavaPropertyDTO: create PropertyDTO failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }
    std::string tag = property.tag;
    SetJavaStringField(env, jProperty, g_propertyDTO.setTag, tag);

    CHECK_NULL_RETURN(g_propertyDTO.clazz, nullptr);
    CHECK_NULL_RETURN(g_propertyDTO.setMimeTypes, nullptr);
    const std::vector<std::string>& mimeTypes = property.mimeTypes;
    if (!mimeTypes.empty()) {
        jclass stringClazz = env->FindClass("java/lang/String");
        jobjectArray jMimeTypes = env->NewObjectArray(mimeTypes.size(), stringClazz, nullptr);
        for (size_t i = 0; i < mimeTypes.size(); ++i) {
            jstring jStr = env->NewStringUTF(mimeTypes[i].c_str());
            env->SetObjectArrayElement(jMimeTypes, i, jStr);
            env->DeleteLocalRef(jStr);
        }
        env->CallVoidMethod(jProperty, g_propertyDTO.setMimeTypes, jMimeTypes);
        if (env->ExceptionCheck()) {
            LOGE("ToJavaPropertyDTO: setMimeTypes failed");
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        env->DeleteLocalRef(jMimeTypes);
        env->DeleteLocalRef(stringClazz);
    }

    AAFwk::WantParams additions = property.additions;
    std::string jsonAdditions = WantParamsToJson(additions);
    SetJavaStringField(env, jProperty, g_propertyDTO.setJsonAdditions, jsonAdditions);
    return jProperty;
}

jobject ToJavaPasteDataDTO(JNIEnv* env, const OHOS::MiscServices::PasteData& pasteData)
{
    CHECK_NULL_RETURN(env, nullptr);
    CHECK_NULL_RETURN(g_pasteDataDTO.clazz, nullptr);
    jobject jPasteData = env->NewObject(g_pasteDataDTO.clazz, g_pasteDataDTO.constructor);
    if (!jPasteData || env->ExceptionCheck()) {
        LOGE("ToJavaPasteDataDTO: create PasteDataDTO failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }
    OHOS::MiscServices::PasteDataProperty property = pasteData.GetProperty();
    std::vector<std::shared_ptr<OHOS::MiscServices::PasteDataRecord>> pasteDataRecords = pasteData.AllRecords();
    jobject jProperty = ToJavaPropertyDTO(env, property);
    jobject jRecords = ToJavaDataRecordDTOArray(env, pasteDataRecords);

    CHECK_NULL_RETURN(jProperty, nullptr);
    if (!g_pasteDataDTO.setPropertyDTO) {
        env->DeleteLocalRef(jProperty);
        return nullptr;
    }
    env->CallVoidMethod(jPasteData, g_pasteDataDTO.setPropertyDTO, jProperty);
    env->DeleteLocalRef(jProperty);

    CHECK_NULL_RETURN(jRecords, nullptr);
    if (!g_pasteDataDTO.setRecords) {
        env->DeleteLocalRef(jRecords);
        return nullptr;
    }
    env->CallVoidMethod(jPasteData, g_pasteDataDTO.setRecords, jRecords);
    env->DeleteLocalRef(jRecords);
    return jPasteData;
}

bool GetJsonWantFromDataRecordDTO(
    JNIEnv* env, const jobject& jrecord, OHOS::MiscServices::PasteDataRecord::Builder& builder,
        std::vector<std::string> mimeTypes)
{
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(jrecord, false);
    CHECK_NULL_RETURN(g_dataRecordDTO.clazz, false);
    CHECK_NULL_RETURN(g_dataRecordDTO.getJsonWant, false);

    jstring jJsonwant = static_cast<jstring>(env->CallObjectMethod(jrecord, g_dataRecordDTO.getJsonWant));
    if (env->ExceptionCheck()) {
        LOGE("GetWantFromRecordDTO: getUri failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    if (jJsonwant) {
        const char* jsonWantStr = env->GetStringUTFChars(jJsonwant, nullptr);
        if (jsonWantStr) {
            auto jsonObject = nlohmann::json::parse(jsonWantStr, nullptr, false);
            if (jsonObject.is_discarded()) {
                LOGE("jsonObject is discarded. value = %{public}s", jsonWantStr);
                std::shared_ptr<OHOS::AAFwk::Want> want = nullptr;
                builder.SetWant(want);
                env->ReleaseStringUTFChars(jJsonwant, jsonWantStr);
                return false;
            }
            std::shared_ptr<OHOS::AAFwk::Want> want = std::make_shared<OHOS::AAFwk::Want>();
            if (jsonObject.contains("bundleName")) {
                want->SetBundleName(jsonObject["bundleName"].get<std::string>());
            }
            if (jsonObject.contains("abilityName")) {
                want->SetAbilityName(jsonObject["abilityName"].get<std::string>());
            }
            if (jsonObject.contains("moduleName")) {
                want->SetModuleName(jsonObject["moduleName"].get<std::string>());
            }
            if (jsonObject.contains("type")) {
                want->SetType(jsonObject["type"].get<std::string>());
            }
            if (jsonObject.contains("params")) {
                std::string wantParamsJson = "{\"params\":" + jsonObject["params"].dump() + "}";
                LOGE("GetJsonWantFromDataRecordDTO: parse parameters: %{public}s", wantParamsJson.c_str());
                want->ParseJson(wantParamsJson);
            }
            builder.SetWant(want);
            mimeTypes.emplace_back(MIMETYPE_TEXT_WANT);
            env->ReleaseStringUTFChars(jJsonwant, jsonWantStr);
        }
        env->DeleteLocalRef(jJsonwant);
    }
    return true;
}

bool GetUriFromDataRecordDTO(JNIEnv* env, const jobject& jrecord, OHOS::MiscServices::PasteDataRecord::Builder& builder,
    std::vector<std::string>& mimeTypes)
{
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(jrecord, false);
    CHECK_NULL_RETURN(g_dataRecordDTO.clazz, false);
    CHECK_NULL_RETURN(g_dataRecordDTO.getUri, false);

    jstring jUri = static_cast<jstring>(env->CallObjectMethod(jrecord, g_dataRecordDTO.getUri));
    if (env->ExceptionCheck()) {
        LOGE("GetUriFromRecordDTO: getUri failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    if (jUri) {
        const char* uriStr = env->GetStringUTFChars(jUri, nullptr);
        if (uriStr) {
            std::shared_ptr<OHOS::Uri> uri = std::make_shared<OHOS::Uri>(uriStr);
            builder.SetUri(uri);
            mimeTypes.emplace_back(MIMETYPE_TEXT_URI);
            env->ReleaseStringUTFChars(jUri, uriStr);
        }
        env->DeleteLocalRef(jUri);
    }
    return true;
}

bool GetPlainTextFromDataRecordDTO(
    JNIEnv* env, const jobject& jrecord, OHOS::MiscServices::PasteDataRecord::Builder& builder, 
        std::vector<std::string>& mimeTypes)
{
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(jrecord, false);
    CHECK_NULL_RETURN(g_dataRecordDTO.clazz, false);
    CHECK_NULL_RETURN(g_dataRecordDTO.getPlainText, false);

    jstring jPlainText = static_cast<jstring>(env->CallObjectMethod(jrecord, g_dataRecordDTO.getPlainText));
    if (env->ExceptionCheck()) {
        LOGE("GetPlainTextFromRecordDTO: getPlainText failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    if (jPlainText) {
        const char* plainStr = env->GetStringUTFChars(jPlainText, nullptr);
        if (plainStr) {
            builder.SetPlainText(std::make_shared<std::string>(plainStr));
            mimeTypes.emplace_back(MIMETYPE_TEXT_PLAIN);
            env->ReleaseStringUTFChars(jPlainText, plainStr);
        }
        env->DeleteLocalRef(jPlainText);
    }
    return true;
}

bool GetHtmlTextFromDataRecordDTO(
    JNIEnv* env, const jobject& jrecord, OHOS::MiscServices::PasteDataRecord::Builder& builder,
        std::vector<std::string>& mimeTypes)
{
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(jrecord, false);
    CHECK_NULL_RETURN(g_dataRecordDTO.clazz, false);
    CHECK_NULL_RETURN(g_dataRecordDTO.getHtmlText, false);

    jstring jHtmlText = static_cast<jstring>(env->CallObjectMethod(jrecord, g_dataRecordDTO.getHtmlText));
    if (env->ExceptionCheck()) {
        LOGE("GetHtmlTextFromRecordDTO: getHtmlText failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    if (jHtmlText) {
        const char* htmlStr = env->GetStringUTFChars(jHtmlText, nullptr);
        if (htmlStr) {
            builder.SetHtmlText(std::make_shared<std::string>(htmlStr));
            mimeTypes.emplace_back(MIMETYPE_TEXT_HTML);
            env->ReleaseStringUTFChars(jHtmlText, htmlStr);
        }
        env->DeleteLocalRef(jHtmlText);
    }
    return true;
}

bool ToDataRecord(JNIEnv* env, const jobject& jrecord, OHOS::MiscServices::PasteData& pasteData,
    const std::string& mimeType)
{
    OHOS::MiscServices::PasteDataRecord::Builder builder("");
    std::vector<std::string> mimeTypes;
    if (!GetHtmlTextFromDataRecordDTO(env, jrecord, builder, mimeTypes)) {
        return false;
    }
    if (!GetPlainTextFromDataRecordDTO(env, jrecord, builder, mimeTypes)) {
        return false;
    }
    if (!GetUriFromDataRecordDTO(env, jrecord, builder, mimeTypes)) {
        return false;
    }
    if (!GetJsonWantFromDataRecordDTO(env, jrecord, builder, mimeTypes)) {
        return false;
    }
    auto it = std::find(mimeTypes.begin(), mimeTypes.end(), mimeType);
    if (it != mimeTypes.end()) {
        builder.SetMimeType(mimeType);
    }
    std::shared_ptr<OHOS::MiscServices::PasteDataRecord> record = builder.Build();
    if (!record) {
        LOGE("ToDataRecord: Builder.Build() failed");
        return false;
    }
    pasteData.AddRecord(record);
    return true;
}

bool ToDataRecordArray(JNIEnv* env, const jobjectArray& jrecords, OHOS::MiscServices::PasteData& pasteData)
{
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(jrecords, false);

    jsize length = env->GetArrayLength(jrecords);
    OHOS::MiscServices::PasteDataProperty property = pasteData.GetProperty();
    std::vector<std::string> mimeTypes = property.mimeTypes;
    int mimeTypeSize = mimeTypes.size();
    property.mimeTypes = {};
    pasteData.SetProperty(property);
    for (jsize i = length - 1; i >= 0; --i) {
        jobject jrecord = env->GetObjectArrayElement(jrecords, i);
        if (!jrecord) {
            LOGE("ToDataRecordArray: null record at index %d", i);
            return false;
        }
        std::string mimeType = "";
        if (i < mimeTypeSize) {
            mimeType = mimeTypes[i];
        }
        if (!ToDataRecord(env, jrecord, pasteData, mimeType)) {
            LOGE("ToDataRecordArray: convert record failed at index %d", i);
            env->DeleteLocalRef(jrecord);
            return false;
        }
        env->DeleteLocalRef(jrecord);
    }
    return true;
}

bool GetTagFromPropertyDTO(JNIEnv* env, const jobject& jproperty, std::string& tag)
{
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_propertyDTO.clazz, false);
    CHECK_NULL_RETURN(g_propertyDTO.getTag, false);

    jstring jTag = static_cast<jstring>(env->CallObjectMethod(jproperty, g_propertyDTO.getTag));
    if (!jTag || env->ExceptionCheck()) {
        LOGE("GetTagFromPropertyDTO: getTag failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    const char* tagStr = env->GetStringUTFChars(jTag, nullptr);
    if (tagStr) {
        tag = tagStr;
        env->ReleaseStringUTFChars(jTag, tagStr);
    }
    env->DeleteLocalRef(jTag);

    return true;
}

bool GetMimeTypesFromPropertyDTO(JNIEnv* env, const jobject& jproperty, std::vector<std::string>& mimeTypes)
{
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_propertyDTO.clazz, false);
    CHECK_NULL_RETURN(g_propertyDTO.getMimeTypes, false);

    jobjectArray jMimeTypes = static_cast<jobjectArray>(env->CallObjectMethod(jproperty, g_propertyDTO.getMimeTypes));
    if (!jMimeTypes || env->ExceptionCheck()) {
        LOGE("GetMimeTypesFromPropertyDTO: getMimeTypes failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    jsize mimeLength = env->GetArrayLength(jMimeTypes);
    for (jsize i = 0; i < mimeLength; ++i) {
        jstring jMime = static_cast<jstring>(env->GetObjectArrayElement(jMimeTypes, i));
        if (jMime) {
            const char* mimeStr = env->GetStringUTFChars(jMime, nullptr);
            if (mimeStr) {
                mimeTypes.emplace_back(mimeStr);
                env->ReleaseStringUTFChars(jMime, mimeStr);
            }
            env->DeleteLocalRef(jMime);
        }
    }
    return true;
}

bool GetTimeStampFromPropertyDTO(JNIEnv* env, const jobject& jproperty, jlong& timestamp)
{
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_propertyDTO.clazz, false);
    CHECK_NULL_RETURN(g_propertyDTO.getTimestamp, false);

    timestamp = static_cast<jlong>(env->CallLongMethod(jproperty, g_propertyDTO.getTimestamp));
    if (env->ExceptionCheck()) {
        LOGE("GetTimeStampFromPropertyDTO: getTimestamp failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}

bool GetAdditionsFromPropertyDTO(JNIEnv* env, const jobject& jproperty, AAFwk::WantParams& additions)
{
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_propertyDTO.clazz, false);
    CHECK_NULL_RETURN(g_propertyDTO.getJsonAdditions, false);

    jstring jAdditions = static_cast<jstring>(env->CallObjectMethod(jproperty, g_propertyDTO.getJsonAdditions));
    if (env->ExceptionCheck()) {
        LOGE("GetAdditionsFromPropertyDTO: getJsonAdditions failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    CHECK_NULL_RETURN(jAdditions, true);
    const char* jsonAdditions = env->GetStringUTFChars(jAdditions, nullptr);
    if (jsonAdditions) {
        additions = JsonToWantParams(jsonAdditions);
        env->ReleaseStringUTFChars(jAdditions, jsonAdditions);
    }
    env->DeleteLocalRef(jAdditions);
    return true;
}

bool GetPropertyFromPropertyDTO(JNIEnv* env, const jobject& jproperty, OHOS::MiscServices::PasteData& pasteData)
{
    std::string tag = "";
    if (!GetTagFromPropertyDTO(env, jproperty, tag)) {
        return false;
    }
    std::vector<std::string> mimeTypes;
    if (!GetMimeTypesFromPropertyDTO(env, jproperty, mimeTypes)) {
        return false;
    }
    jlong timestamp = 0;
    if (!GetTimeStampFromPropertyDTO(env, jproperty, timestamp)) {
        return false;
    }
    AAFwk::WantParams additions;
    if (!GetAdditionsFromPropertyDTO(env, jproperty, additions)) {
        return false;
    }
    OHOS::MiscServices::PasteDataProperty property;
    property.tag = tag;
    property.mimeTypes = mimeTypes;
    property.timestamp = timestamp;
    property.additions = additions;
    pasteData.SetProperty(property);
    return true;
}

bool ToPasteData(JNIEnv* env, const jobject& jPasteData, OHOS::MiscServices::PasteData& pasteData)
{
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_pasteDataDTO.clazz, false);
    CHECK_NULL_RETURN(g_pasteDataDTO.getPropertyDTO, false);
    CHECK_NULL_RETURN(g_pasteDataDTO.getRecords, false);

    jobject jproperty = env->CallObjectMethod(jPasteData, g_pasteDataDTO.getPropertyDTO);
    if (!jproperty || env->ExceptionCheck()) {
        LOGE("ToPasteData: getPropertyDTO failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    if (!GetPropertyFromPropertyDTO(env, jproperty, pasteData)) {
        LOGE("ToPasteData: convert PropertyDTO failed");
        env->DeleteLocalRef(jproperty);
        return false;
    }
    env->DeleteLocalRef(jproperty);

    jobjectArray jrecords = static_cast<jobjectArray>(env->CallObjectMethod(jPasteData, g_pasteDataDTO.getRecords));
    if (!jrecords || env->ExceptionCheck()) {
        LOGE("ToPasteData: getRecords failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    if (!ToDataRecordArray(env, jrecords, pasteData)) {
        LOGE("ToPasteData: convert DataRecordArray failed");
        env->DeleteLocalRef(jrecords);
        return false;
    }
    env->DeleteLocalRef(jrecords);
    return true;
}
} // namespace

bool ClipboardJni::Register(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);

    jclass clazz = jniEnv->FindClass(CLIPBOARD_PLUGIN_CLASS_NAME);
    CHECK_NULL_RETURN(clazz, false);

    bool ret = jniEnv->RegisterNatives(clazz, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(clazz);
    CHECK_NULL_RETURN(ret, false);

    jclass pasteDataClass = jniEnv->FindClass(PASTE_DATA_DTO_CLASS_NAME);
    g_pasteDataDTO.clazz = static_cast<jclass>(jniEnv->NewGlobalRef(pasteDataClass));
    jniEnv->DeleteLocalRef(pasteDataClass);
    CHECK_NULL_RETURN(g_pasteDataDTO.clazz, false);

    jclass dataRecordClass = jniEnv->FindClass(PASTE_DATA_RECORD_DTO_CLASS_NAME);
    g_dataRecordDTO.clazz = static_cast<jclass>(jniEnv->NewGlobalRef(dataRecordClass));
    jniEnv->DeleteLocalRef(dataRecordClass);
    CHECK_NULL_RETURN(g_dataRecordDTO.clazz, false);

    jclass propertyClass = jniEnv->FindClass(PASTE_DATA_PROPERTY_DTO_CLASS_NAME);
    g_propertyDTO.clazz = static_cast<jclass>(jniEnv->NewGlobalRef(propertyClass));
    jniEnv->DeleteLocalRef(propertyClass);
    CHECK_NULL_RETURN(g_propertyDTO.clazz, false);
    return true;
}

void ClipboardJni::NativeInit(JNIEnv* env, jobject object)
{
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(object);
    g_clipboard.globalRef = env->NewGlobalRef(object);
    CHECK_NULL_VOID(g_clipboard.globalRef);
    jclass clazz = env->GetObjectClass(object);
    CHECK_NULL_VOID(clazz);

    g_clipboard.setData = env->GetMethodID(clazz, METHOD_SET_DATA, SIGNATURE_SET_DATA);
    CHECK_NULL_VOID_RELEASE_CLAZZ(g_clipboard.setData, env, clazz);

    g_clipboard.getData = env->GetMethodID(clazz, METHOD_GET_DATA, SIGNATURE_GET_DATA);
    CHECK_NULL_VOID_RELEASE_CLAZZ(g_clipboard.getData, env, clazz);

    g_clipboard.hasData = env->GetMethodID(clazz, METHOD_HAS_DATA, SIGNATURE_HAS_DATA);
    CHECK_NULL_VOID_RELEASE_CLAZZ(g_clipboard.hasData, env, clazz);

    g_clipboard.clear = env->GetMethodID(clazz, METHOD_CLEAR, SIGNATURE_CLEAR);
    CHECK_NULL_VOID_RELEASE_CLAZZ(g_clipboard.clear, env, clazz);

    g_clipboard.subscribePasteboardChange =
        env->GetMethodID(clazz, METHOD_SUBSCRIBE_PASTEBOARD_CHANGE, SIGNATURE_SUBSCRIBE_PASTEBOARD_CHANGE);
    CHECK_NULL_VOID_RELEASE_CLAZZ(g_clipboard.subscribePasteboardChange, env, clazz);

    g_clipboard.unsubscribePasteboardChange =
        env->GetMethodID(clazz, METHOD_UNSUBSCRIBE_PASTEBOARD_CHANGE, SIGNATURE_UNSUBSCRIBE_PASTEBOARD_CHANGE);
    CHECK_NULL_VOID_RELEASE_CLAZZ(g_clipboard.unsubscribePasteboardChange, env, clazz);

    g_clipboard.detectPatterns = env->GetMethodID(clazz, METHOD_DETECT_PATTERNS, SIGNATURE_DETECT_PATTERNS);
    CHECK_NULL_VOID_RELEASE_CLAZZ(g_clipboard.detectPatterns, env, clazz);

    g_clipboard.hasDataType = env->GetMethodID(clazz, METHOD_HAS_DATA_TYPE, SIGNATURE_HAS_DATA_TYPE);
    CHECK_NULL_VOID_RELEASE_CLAZZ(g_clipboard.hasDataType, env, clazz);

    g_clipboard.getMimeTypes = env->GetMethodID(clazz, METHOD_GET_MIMETYPES, SIGNATURE_GET_MIMETYPES);
    CHECK_NULL_VOID_RELEASE_CLAZZ(g_clipboard.getMimeTypes, env, clazz);

    env->DeleteLocalRef(clazz);

    PasteDataDTOInit(env);
    DataRecordDTOInit(env);
    PropertyDTOInit(env);
}

void ClipboardJni::PasteDataDTOInit(JNIEnv* env)
{
    CHECK_NULL_VOID(env);

    g_pasteDataDTO.constructor = env->GetMethodID(g_pasteDataDTO.clazz, "<init>", "()V");
    CHECK_NULL_VOID(g_pasteDataDTO.constructor);

    g_pasteDataDTO.setPropertyDTO =
        env->GetMethodID(g_pasteDataDTO.clazz, METHOD_SET_PROPERTY_DTO, SIGNATURE_SET_PROPERTY_DTO);
    CHECK_NULL_VOID(g_pasteDataDTO.setPropertyDTO);

    g_pasteDataDTO.setRecords = env->GetMethodID(g_pasteDataDTO.clazz, METHOD_SET_RECORDS, SIGNATURE_SET_RECORDS);
    CHECK_NULL_VOID(g_pasteDataDTO.setRecords);

    g_pasteDataDTO.getPropertyDTO =
        env->GetMethodID(g_pasteDataDTO.clazz, METHOD_GET_PROPERTY_DTO, SIGNATURE_GET_PROPERTY_DTO);
    CHECK_NULL_VOID(g_pasteDataDTO.getPropertyDTO);

    g_pasteDataDTO.getRecords = env->GetMethodID(g_pasteDataDTO.clazz, METHOD_GET_RECORDS, SIGNATURE_GET_RECORDS);
    CHECK_NULL_VOID(g_pasteDataDTO.getRecords);
}

void ClipboardJni::DataRecordDTOInit(JNIEnv* env)
{
    CHECK_NULL_VOID(env);

    g_dataRecordDTO.constructor = env->GetMethodID(g_dataRecordDTO.clazz, "<init>", "()V");
    CHECK_NULL_VOID(g_dataRecordDTO.constructor);

    g_dataRecordDTO.setMimeType = env->GetMethodID(g_dataRecordDTO.clazz, METHOD_SET_MIMETYPE, SIGNATURE_SET_MIMETYPE);
    CHECK_NULL_VOID(g_dataRecordDTO.setMimeType);

    g_dataRecordDTO.setHtmlText = env->GetMethodID(g_dataRecordDTO.clazz, METHOD_SET_HTMLTEXT, SIGNATURE_SET_HTMLTEXT);
    CHECK_NULL_VOID(g_dataRecordDTO.setHtmlText);

    g_dataRecordDTO.setJsonWant =
        env->GetMethodID(g_dataRecordDTO.clazz, METHOD_SET_JSON_WANT, SIGNATURE_SET_JSON_WANT);
    CHECK_NULL_VOID(g_dataRecordDTO.setJsonWant);

    g_dataRecordDTO.setPlainText =
        env->GetMethodID(g_dataRecordDTO.clazz, METHOD_SET_PLAINTEXT, SIGNATURE_SET_PLAINTEXT);
    CHECK_NULL_VOID(g_dataRecordDTO.setPlainText);

    g_dataRecordDTO.setUri = env->GetMethodID(g_dataRecordDTO.clazz, METHOD_SET_URI, SIGNATURE_SET_URI);
    CHECK_NULL_VOID(g_dataRecordDTO.setUri);

    g_dataRecordDTO.getMimeType = env->GetMethodID(g_dataRecordDTO.clazz, METHOD_GET_MIMETYPE, SIGNATURE_GET_MIMETYPE);
    CHECK_NULL_VOID(g_dataRecordDTO.getMimeType);

    g_dataRecordDTO.getHtmlText = env->GetMethodID(g_dataRecordDTO.clazz, METHOD_GET_HTMLTEXT, SIGNATURE_GET_HTMLTEXT);
    CHECK_NULL_VOID(g_dataRecordDTO.getHtmlText);

    g_dataRecordDTO.getJsonWant =
        env->GetMethodID(g_dataRecordDTO.clazz, METHOD_GET_JSON_WANT, SIGNATURE_GET_JSON_WANT);
    CHECK_NULL_VOID(g_dataRecordDTO.getJsonWant);

    g_dataRecordDTO.getPlainText =
        env->GetMethodID(g_dataRecordDTO.clazz, METHOD_GET_PLAINTEXT, SIGNATURE_GET_PLAINTEXT);
    CHECK_NULL_VOID(g_dataRecordDTO.getPlainText);

    g_dataRecordDTO.getUri = env->GetMethodID(g_dataRecordDTO.clazz, METHOD_GET_URI, SIGNATURE_GET_URI);
    CHECK_NULL_VOID(g_dataRecordDTO.getUri);
}

void ClipboardJni::PropertyDTOInit(JNIEnv* env)
{
    CHECK_NULL_VOID(env);

    g_propertyDTO.constructor = env->GetMethodID(g_propertyDTO.clazz, "<init>", "()V");
    CHECK_NULL_VOID(g_propertyDTO.constructor);

    g_propertyDTO.setMimeTypes = env->GetMethodID(g_propertyDTO.clazz, METHOD_SET_MIMETYPES, SIGNATURE_SET_MIMETYPES);
    CHECK_NULL_VOID(g_propertyDTO.setMimeTypes);

    g_propertyDTO.setTag = env->GetMethodID(g_propertyDTO.clazz, METHOD_SET_TAG, SIGNATURE_SET_TAG);
    CHECK_NULL_VOID(g_propertyDTO.setTag);

    g_propertyDTO.setTimestamp = env->GetMethodID(g_propertyDTO.clazz, METHOD_SET_TIMESTAMP, SIGNATURE_SET_TIMESTAMP);
    CHECK_NULL_VOID(g_propertyDTO.setTimestamp);

    g_propertyDTO.setJsonAdditions =
        env->GetMethodID(g_propertyDTO.clazz, METHOD_SET_JSON_ADDITIONS, SIGNATURE_SET_JSON_ADDITIONS);
    CHECK_NULL_VOID(g_propertyDTO.setJsonAdditions);

    g_propertyDTO.getMimeTypes = env->GetMethodID(g_propertyDTO.clazz, METHOD_GET_MIMETYPES, SIGNATURE_GET_MIMETYPES);
    CHECK_NULL_VOID(g_propertyDTO.getMimeTypes);

    g_propertyDTO.getTag = env->GetMethodID(g_propertyDTO.clazz, METHOD_GET_TAG, SIGNATURE_GET_TAG);
    CHECK_NULL_VOID(g_propertyDTO.getTag);

    g_propertyDTO.getTimestamp = env->GetMethodID(g_propertyDTO.clazz, METHOD_GET_TIMESTAMP, SIGNATURE_GET_TIMESTAMP);
    CHECK_NULL_VOID(g_propertyDTO.getTimestamp);

    g_propertyDTO.getJsonAdditions =
        env->GetMethodID(g_propertyDTO.clazz, METHOD_GET_JSON_ADDITIONS, SIGNATURE_GET_JSON_ADDITIONS);
    CHECK_NULL_VOID(g_propertyDTO.getJsonAdditions);
}

int32_t ClipboardJni::SetData(const OHOS::MiscServices::PasteData& pasteData)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LOGE("SetData: null env");
        return static_cast<int32_t>(PasteboardError::OTHER_ERROR);
    }
    if (!g_clipboard.globalRef) {
        LOGE("SetData: null g_clipboard.globalRef");
        return static_cast<int32_t>(PasteboardError::OTHER_ERROR);
    }
    if (!g_clipboard.setData) {
        LOGE("SetData: null setData method");
        return static_cast<int32_t>(PasteboardError::OTHER_ERROR);
    }

    jobject jPasteData = ToJavaPasteDataDTO(env, pasteData);
    if (jPasteData == nullptr) {
        LOGE("SetData: jPasteData is null");
        return static_cast<int32_t>(PasteboardError::OTHER_ERROR);
    }

    auto setDataRet = env->CallIntMethod(g_clipboard.globalRef, g_clipboard.setData, jPasteData);
    env->DeleteLocalRef(jPasteData);
    if (env->ExceptionCheck()) {
        LOGE("SetData: call setData has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    int32_t ret = static_cast<int32_t>(setDataRet);
    return ret;
}

int32_t ClipboardJni::GetData(OHOS::MiscServices::PasteData& pasteData)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LOGE("GetData: null env");
        return static_cast<int32_t>(PasteboardError::OTHER_ERROR);
    }
    if (!g_clipboard.globalRef) {
        LOGE("GetData: null g_clipboard.globalRef");
        return static_cast<int32_t>(PasteboardError::OTHER_ERROR);
    }
    if (!g_clipboard.getData) {
        LOGE("GetData: null getData method");
        return static_cast<int32_t>(PasteboardError::OTHER_ERROR);
    }
    jobject jPasteData = env->CallObjectMethod(g_clipboard.globalRef, g_clipboard.getData);
    if (env->ExceptionCheck()) {
        LOGE("GetData: Call getData method failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return static_cast<int32_t>(PasteboardError::OTHER_ERROR);
    }
    if (!jPasteData) {
        return static_cast<int32_t>(PasteboardError::E_OK);
    }
    if (!ToPasteData(env, jPasteData, pasteData)) {
        LOGE("GetData: Convert jPasteData to PasteData failed");
        pasteData = OHOS::MiscServices::PasteData();
        env->DeleteLocalRef(jPasteData);
        return static_cast<int32_t>(PasteboardError::OTHER_ERROR);
    }
    env->DeleteLocalRef(jPasteData);
    return static_cast<int32_t>(PasteboardError::E_OK);
}

bool ClipboardJni::HasData()
{
    return HasPasteData();
}

bool ClipboardJni::HasPasteData()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LOGE("Clipborad JNI: null env");
        return false;
    }
    if (!g_clipboard.globalRef) {
        LOGE("Clipborad JNI: null g_clipboard.globalRef");
        return false;
    }
    if (!g_clipboard.hasData) {
        LOGE("Clipborad JNI: null HasPasteData method");
        return false;
    }

    auto ret = env->CallBooleanMethod(g_clipboard.globalRef, g_clipboard.hasData);
    if (env->ExceptionCheck()) {
        LOGE("Clipborad JNI: call HasPasteData has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return ret;
}

bool ClipboardJni::Clear()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LOGE("Clipborad JNI: null env");
        return false;
    }
    if (!g_clipboard.globalRef) {
        LOGE("Clipborad JNI: null g_clipboard.globalRef");
        return false;
    }
    if (!g_clipboard.clear) {
        LOGE("Clipborad JNI: null Clear method");
        return false;
    }

    auto ret = env->CallBooleanMethod(g_clipboard.globalRef, g_clipboard.clear);
    if (env->ExceptionCheck()) {
        LOGE("Clipborad JNI: call Clear has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return ret;
}

bool ClipboardJni::Subscribe()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LOGE("Clipborad JNI: null env");
        return false;
    }
    if (!g_clipboard.globalRef) {
        LOGE("Clipborad JNI: null g_clipboard.globalRef");
        return false;
    }
    if (!g_clipboard.subscribePasteboardChange) {
        LOGE("Clipborad JNI: null subscribePasteboardChange method");
        return false;
    }

    auto ret = env->CallBooleanMethod(g_clipboard.globalRef, g_clipboard.subscribePasteboardChange);
    if (env->ExceptionCheck()) {
        LOGE("Clipborad JNI: call subscribePasteboardChange has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return ret;
}

bool ClipboardJni::Unsubscribe()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LOGE("Clipborad JNI: null env");
        return false;
    }
    if (!g_clipboard.globalRef) {
        LOGE("Clipborad JNI: null g_clipboard.globalRef");
        return false;
    }
    if (!g_clipboard.unsubscribePasteboardChange) {
        LOGE("Clipborad JNI: null unsubscribePasteboardChange method");
        return false;
    }

    auto ret = env->CallBooleanMethod(g_clipboard.globalRef, g_clipboard.unsubscribePasteboardChange);
    if (env->ExceptionCheck()) {
        LOGE("Clipborad JNI: call unsubscribePasteboardChange has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return ret;
}

void ClipboardJni::OnPasteboardChanged()
{
    auto clipboardProxy = ClipboardProxy::GetInstance();
    if (clipboardProxy == nullptr) {
        LOGE("Clipborad JNI: clipboardProxy is null");
        return;
    }
    clipboardProxy->NotifyObservers();
}

int ClipboardJni::DetectPatterns(
    const std::vector<MiscServices::Pattern>& patternsToCheck, std::vector<MiscServices::Pattern>& funcResult)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LOGE("Clipborad JNI: null env");
        return static_cast<int32_t>(PasteboardError::INVALID_OPERATION_ERROR);
    }
    if (!g_clipboard.globalRef) {
        LOGE("Clipborad JNI: null g_clipboard.globalRef");
        return static_cast<int32_t>(PasteboardError::INVALID_OPERATION_ERROR);
    }
    if (!g_clipboard.detectPatterns) {
        LOGE("Clipborad JNI: null detectPatterns method");
        return static_cast<int32_t>(PasteboardError::INVALID_OPERATION_ERROR);
    }

    int patterns = 0;
    for (auto pattern : patternsToCheck) {
        if (pattern == Pattern::URL) {
            patterns |= 1 << static_cast<uint32_t>(Pattern::URL);
        } else if (pattern == Pattern::EMAIL_ADDRESS) {
            patterns |= 1 << static_cast<uint32_t>(Pattern::EMAIL_ADDRESS);
        } else {
            patterns |= 1 << static_cast<uint32_t>(Pattern::NUMBER);
        }
    }
    auto ret = env->CallIntMethod(g_clipboard.globalRef, g_clipboard.detectPatterns, patterns);
    if (env->ExceptionCheck()) {
        LOGE("Clipborad JNI: call DetectPatterns has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return static_cast<int32_t>(PasteboardError::INVALID_OPERATION_ERROR);
    }
    if (ret & (1 << static_cast<uint32_t>(Pattern::URL))) {
        funcResult.push_back(Pattern::URL);
    }
    if (ret & (1 << static_cast<uint32_t>(Pattern::EMAIL_ADDRESS))) {
        funcResult.push_back(Pattern::EMAIL_ADDRESS);
    }
    if (ret & (1 << static_cast<uint32_t>(Pattern::NUMBER))) {
        funcResult.push_back(Pattern::NUMBER);
    }
    return ERR_OK;
}

bool ClipboardJni::HasDataType(const std::string& mimeType)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LOGE("Clipborad JNI: null env");
        return false;
    }
    if (!g_clipboard.globalRef) {
        LOGE("Clipborad JNI: null g_clipboard.globalRef");
        return false;
    }
    if (!g_clipboard.hasDataType) {
        LOGE("Clipborad JNI: null hasDataType method");
        return false;
    }

    jstring jMimeType = env->NewStringUTF(mimeType.c_str());
    if (!jMimeType) {
        LOGE("Clipborad JNI: failed to create jstring for mimeType");
        return false;
    }

    auto ret = env->CallBooleanMethod(g_clipboard.globalRef, g_clipboard.hasDataType, jMimeType);
    env->DeleteLocalRef(jMimeType);

    if (env->ExceptionCheck()) {
        LOGE("Clipborad JNI: call hasDataType has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return ret;
}

bool ClipboardJni::GetMimeTypes(std::vector<std::string>& funcResult)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        LOGE("Clipborad JNI: null env");
        return false;
    }
    if (!g_clipboard.globalRef) {
        LOGE("Clipborad JNI: null g_clipboard.globalRef");
        return false;
    }
    if (!g_clipboard.getMimeTypes) {
        LOGE("Clipborad JNI: null getMimeTypes method");
        return false;
    }

    jobjectArray jMimeTypes =
        static_cast<jobjectArray>(env->CallObjectMethod(g_clipboard.globalRef, g_clipboard.getMimeTypes));
    if (!jMimeTypes || env->ExceptionCheck()) {
        LOGE("Clipborad: getMimeTypes failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    std::set<std::string> mimeTypes;
    jsize mimeLength = env->GetArrayLength(jMimeTypes);
    for (jsize i = 0; i < mimeLength; ++i) {
        jstring jMime = static_cast<jstring>(env->GetObjectArrayElement(jMimeTypes, i));
        if (jMime) {
            const char* mimeStr = env->GetStringUTFChars(jMime, nullptr);
            if (mimeStr) {
                mimeTypes.insert(mimeStr);
                env->ReleaseStringUTFChars(jMime, mimeStr);
            }
            env->DeleteLocalRef(jMime);
        }
    }
    funcResult = std::vector<std::string>(mimeTypes.begin(), mimeTypes.end());
    return true;
}
} // namespace OHOS::Plugin
