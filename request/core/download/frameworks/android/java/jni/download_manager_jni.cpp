/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "download_manager_jni.h"

#include <jni.h>
#include <codecvt>
#include <locale>

#include "android_download_adp.h"
#include "android_download_adp_impl.h"
#include "android_download_task_impl.h"
#include "constant.h"
#include "download_manager_receiver.h"
#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugins/interfaces/native/log.h"
#include "plugin_utils.h"

namespace OHOS::Plugin::Request::Download {
static constexpr uint32_t DOWNLOAD_RECEIVED_SIZE_ARGC = 0;
static constexpr uint32_t DOWNLOAD_TOTAL_SIZE_ARGC = 1;
// download status and failed reason
static constexpr uint32_t DOWNLOAD_STATUS = 2;
static constexpr uint32_t DOWNLOAD_FAILED_REASON = 3;
// download status code
static constexpr uint32_t DOWNLOAD_PAUSE = 4;
static constexpr uint32_t DOWNLOAD_SUCCESS = 8;
static constexpr uint32_t DOWNLOAD_FAILED = 16;
// DownloadManager error code
static constexpr uint32_t DOWNLOADMANAGER_ERROR_UNKNOWN = 1000;
static constexpr uint32_t DOWNLOADMANAGER_ERROR_FILE_ERROR = 1001;
static constexpr uint32_t DOWNLOADMANAGER_ERROR_UNHANDLED_HTTP_CODE = 1002;
static constexpr uint32_t DOWNLOADMANAGER_ERROR_HTTP_DATA_ERROR = 1004;
static constexpr uint32_t DOWNLOADMANAGER_ERROR_TOO_MANY_REDIRECTS = 1005;
static constexpr uint32_t DOWNLOADMANAGER_ERROR_INSUFFICIENT_SPACE = 1006;
static constexpr uint32_t DOWNLOADMANAGER_ERROR_DEVICE_NOT_FOUND = 1007;
static constexpr uint32_t DOWNLOADMANAGER_ERROR_CANNOT_RESUME = 1008;
static constexpr uint32_t DOWNLOADMANAGER_ERROR_FILE_ALREADY_EXISTS = 1009;
// network type from java
static constexpr uint32_t JAVA_NETWORK_INVALID = 0;
static constexpr uint32_t JAVA_NETWORK_WIFI = 1;
static constexpr uint32_t JAVA_NETWORK_MOBILE = 2;

const char DOWNLOADMANAGER_PLUGIN_CLASS_NAME[] = "ohos/ace/plugin/downloadmanagerplugin/DownloadManagerPlugin";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(DownloadManagerJni::NativeInit) },
    { "onRequestDataCallback", "([IJ)V", reinterpret_cast<void*>(DownloadManagerJni::OnRequestDataCallback) },
    { "onRequestMimeTypeCallback", "(Ljava/lang/String;J)V",
        reinterpret_cast<void*>(DownloadManagerJni::OnRequestMimeTypeCallback) },
    { "onRequestNetworkTypeCallback", "(IJ)V",
        reinterpret_cast<void*>(DownloadManagerJni::OnRequestNetworkTypeCallback) },
};

const char METHOD_START_DOWNLOAD[] = "startDownload";
const char SIGNATURE_START_DOWNLOAD[] = "(J)J";

const char METHOD_REMOVE_DOWNLOAD[] = "removeDownload";
const char SIGNATURE_REMOVE_DOWNLOAD[] = "()V";

const char METHOD_RESUME_DOWNLOAD[] = "resumeDownload";
const char SIGNATURE_RESUME_DOWNLOAD[] = "()Z";

const char METHOD_PAUSE_DOWNLOAD[] = "pauseDownload";
const char SIGNATURE_PAUSE_DOWNLOAD[] = "()V";

const char METHOD_GET_NETWORK_STATE[] = "getNetworkState";
const char SIGNATURE_GET_NETWORK_STATE[] = "(J)I";

const char METHOD_GET_URL[] = "getUrl";
const char SIGNATURE_GET_URL[] = "(Ljava/lang/String;)V";

const char METHOD_GET_HEADER[] = "getHeader";
const char SIGNATURE_GET_HEADER[] = "([Ljava/lang/String;[Ljava/lang/String;)V";

const char METHOD_GET_DESCRIPTION[] = "getDescription";
const char SIGNATURE_GET_DESCRIPTION[] = "(Ljava/lang/String;)V";

const char METHOD_GET_ENABLE_METERED[] = "getEnableMetered";
const char SIGNATURE_GET_ENABLE_METERED[] = "(Z)V";

const char METHOD_GET_ENABLE_ROAMING[] = "getEnableRoaming";
const char SIGNATURE_GET_ENABLE_ROAMING[] = "(Z)V";

const char METHOD_GET_TITLE[] = "getTitle";
const char SIGNATURE_GET_TITLE[] = "(Ljava/lang/String;)V";

const char METHOD_GET_NETWORK_TYPE[] = "getNetworkType";
const char SIGNATURE_GET_NETWORK_TYPE[] = "(I)V";

const char METHOD_GET_FILE_PATH[] = "getFilePath";
const char SIGNATURE_GET_FILE_PATH[] = "(Ljava/lang/String;)V";

const char METHOD_GET_BACKGROUND[] = "getBackground";
const char SIGNATURE_GET_BACKGROUND[] = "(Z)V";

struct {
    jmethodID getFilePath;
    jmethodID getHeader;
    jmethodID getBackground;
    jmethodID getNetworkType;
    jmethodID getTitle;
    jmethodID getDescription;
    jmethodID getEnableRoaming;
    jmethodID getEnableMetered;
    jmethodID getUrl;
    jmethodID getNetworkState;
    jmethodID pauseDownload;
    jmethodID resumeDownload;
    jmethodID removeDownload;
    jmethodID download;
    jobject globalRef;
} g_pluginClass;

bool DownloadManagerJni::Register(void* env)
{
    DOWNLOAD_HILOGI("DownloadManager JNI: Register");
    auto *jniEnv = static_cast<JNIEnv*>(env);
    if (!jniEnv) {
        DOWNLOAD_HILOGE("JNIEnv is null when registering DownloadManager jni!");
        return false;
    }

    jclass cls = jniEnv->FindClass(DOWNLOADMANAGER_PLUGIN_CLASS_NAME);
    if (cls == nullptr) {
        DOWNLOAD_HILOGE("Failed to find DownloadManager class: %{public}s", DOWNLOADMANAGER_PLUGIN_CLASS_NAME);
        return false;
    }

    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    if (!ret) {
        DOWNLOAD_HILOGE("DownloadManager JNI: RegisterNatives fail.");
        return false;
    }

    DOWNLOAD_HILOGI("DownloadManager JNI: Register success");
    return true;
}

static std::string UTF16StringToUTF8String(const char16_t* chars, size_t len)
{
    std::u16string u16_string(chars, len);
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.to_bytes(u16_string);
}

static std::string JavaStringToString(JNIEnv* env, jstring str)
{
    if (env == nullptr || str == nullptr) {
        return "";
    }
    const jchar* chars = env->GetStringChars(str, NULL);
    if (chars == nullptr) {
        return "";
    }

    std::string u8_string =
        UTF16StringToUTF8String(reinterpret_cast<const char16_t *>(chars), env->GetStringLength(str));
    env->ReleaseStringChars(str, chars);
    return u8_string;
}

static jstring StringToJavaString(JNIEnv* env, const std::string &string)
{
    std::u16string str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(string);
    return env->NewString(reinterpret_cast<const jchar *>(str.data()), str.length());
}

void DownloadManagerJni::NativeInit(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    g_pluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_pluginClass.globalRef);

    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_pluginClass.download = env->GetMethodID(cls, METHOD_START_DOWNLOAD, SIGNATURE_START_DOWNLOAD);
    CHECK_NULL_VOID(g_pluginClass.download);

    g_pluginClass.resumeDownload = env->GetMethodID(cls, METHOD_RESUME_DOWNLOAD, SIGNATURE_RESUME_DOWNLOAD);
    CHECK_NULL_VOID(g_pluginClass.resumeDownload);

    g_pluginClass.removeDownload = env->GetMethodID(cls, METHOD_REMOVE_DOWNLOAD, SIGNATURE_REMOVE_DOWNLOAD);
    CHECK_NULL_VOID(g_pluginClass.removeDownload);

    g_pluginClass.pauseDownload = env->GetMethodID(cls, METHOD_PAUSE_DOWNLOAD, SIGNATURE_PAUSE_DOWNLOAD);
    CHECK_NULL_VOID(g_pluginClass.pauseDownload);

    g_pluginClass.getNetworkState = env->GetMethodID(cls, METHOD_GET_NETWORK_STATE, SIGNATURE_GET_NETWORK_STATE);
    CHECK_NULL_VOID(g_pluginClass.getNetworkState);

    g_pluginClass.getUrl = env->GetMethodID(cls, METHOD_GET_URL, SIGNATURE_GET_URL);
    CHECK_NULL_VOID(g_pluginClass.getUrl);

    g_pluginClass.getEnableMetered = env->GetMethodID(cls, METHOD_GET_ENABLE_METERED, SIGNATURE_GET_ENABLE_METERED);
    CHECK_NULL_VOID(g_pluginClass.getEnableMetered);

    g_pluginClass.getEnableRoaming = env->GetMethodID(cls, METHOD_GET_ENABLE_ROAMING, SIGNATURE_GET_ENABLE_ROAMING);
    CHECK_NULL_VOID(g_pluginClass.getEnableRoaming);

    g_pluginClass.getDescription = env->GetMethodID(cls, METHOD_GET_DESCRIPTION, SIGNATURE_GET_DESCRIPTION);
    CHECK_NULL_VOID(g_pluginClass.getDescription);

    g_pluginClass.getTitle = env->GetMethodID(cls, METHOD_GET_TITLE, SIGNATURE_GET_TITLE);
    CHECK_NULL_VOID(g_pluginClass.getTitle);

    g_pluginClass.getNetworkType = env->GetMethodID(cls, METHOD_GET_NETWORK_TYPE, SIGNATURE_GET_NETWORK_TYPE);
    CHECK_NULL_VOID(g_pluginClass.getNetworkType);

    g_pluginClass.getBackground = env->GetMethodID(cls, METHOD_GET_BACKGROUND, SIGNATURE_GET_BACKGROUND);
    CHECK_NULL_VOID(g_pluginClass.getBackground);

    g_pluginClass.getHeader = env->GetMethodID(cls, METHOD_GET_HEADER, SIGNATURE_GET_HEADER);
    CHECK_NULL_VOID(g_pluginClass.getHeader);

    g_pluginClass.getFilePath = env->GetMethodID(cls, METHOD_GET_FILE_PATH, SIGNATURE_GET_FILE_PATH);
    CHECK_NULL_VOID(g_pluginClass.getFilePath);
    env->DeleteLocalRef(cls);
}

static ErrorCode TranslateErrorCode(uint32_t dmFailedReason)
{
    DOWNLOAD_HILOGI("get DownloadManager error code from java, errorCode: %{private}d", dmFailedReason);
    ErrorCode errorCode = ERROR_UNKNOWN;
    switch (dmFailedReason) {
        case DOWNLOADMANAGER_ERROR_UNKNOWN:
            errorCode = ERROR_UNKNOWN;
            break;
        case DOWNLOADMANAGER_ERROR_FILE_ERROR:
            errorCode = ERROR_FILE_ERROR;
            break;
        case DOWNLOADMANAGER_ERROR_UNHANDLED_HTTP_CODE:
            errorCode = ERROR_UNHANDLED_HTTP_CODE;
            break;
        case DOWNLOADMANAGER_ERROR_HTTP_DATA_ERROR:
            errorCode = ERROR_HTTP_DATA_ERROR;
            break;
        case DOWNLOADMANAGER_ERROR_TOO_MANY_REDIRECTS:
            errorCode = ERROR_TOO_MANY_REDIRECTS;
            break;
        case DOWNLOADMANAGER_ERROR_INSUFFICIENT_SPACE:
            errorCode = ERROR_INSUFFICIENT_SPACE;
            break;
        case DOWNLOADMANAGER_ERROR_DEVICE_NOT_FOUND:
            errorCode = ERROR_DEVICE_NOT_FOUND;
            break;
        case DOWNLOADMANAGER_ERROR_CANNOT_RESUME:
            errorCode = ERROR_CANNOT_RESUME;
            break;
        case DOWNLOADMANAGER_ERROR_FILE_ALREADY_EXISTS:
            errorCode = ERROR_FILE_ALREADY_EXISTS;
            break;
        default:
            DOWNLOAD_HILOGE("error code: SESSION_UNKNOWN");
            break;
    }
    DOWNLOAD_HILOGD("translate download manager errorcode success, errorCode: %{private}d", errorCode);
    return errorCode;
}

static NetworkType TranslateNetworkType(int networkType)
{
    DOWNLOAD_HILOGI("get networkType from java, networkType: %{private}d", networkType);
    NetworkType type = NETWORK_INVALID;
    switch (networkType) {
        case JAVA_NETWORK_INVALID:
            type = NETWORK_INVALID;
            break;
        case JAVA_NETWORK_WIFI:
            type = NETWORK_WIFI;
            break;
        case JAVA_NETWORK_MOBILE:
            type = NETWORK_MOBILE;
            break;
        default :
            break;
    }
    DOWNLOAD_HILOGD("translate network type success, networkType: %{private}d", type);
    return type;
}

void DownloadManagerJni::OnRequestDataCallback(JNIEnv* env, jobject obj, jintArray downloadDataArray,
                                               jlong jDownloadProgress)
{
    DOWNLOAD_HILOGI("DownloadManager JNI: OnRequestDataCallback");
    jsize downloadDataArrayLength = env->GetArrayLength(downloadDataArray);

    std::vector<int> downloadDataVector(downloadDataArrayLength);
    jint *downloadDataElements = env->GetIntArrayElements(downloadDataArray, NULL);

    for (int i = 0; i < downloadDataArrayLength; i++) {
        downloadDataVector[i] = downloadDataElements[i];
    }

    int64_t receivedSize = static_cast<int64_t>(downloadDataVector[DOWNLOAD_RECEIVED_SIZE_ARGC]);
    int64_t totalSize = static_cast<int64_t>(downloadDataVector[DOWNLOAD_TOTAL_SIZE_ARGC]);
    uint32_t downloadStatus = downloadDataVector[DOWNLOAD_STATUS];
    uint32_t dmFailedReason = downloadDataVector[DOWNLOAD_FAILED_REASON];
    void *downloadProgress = reinterpret_cast<void *>(jDownloadProgress);
    AndroidDownloadTaskImpl *downloadTaskImpl = reinterpret_cast<AndroidDownloadTaskImpl *>(downloadProgress);
    if (downloadTaskImpl != nullptr) {
        DOWNLOAD_HILOGI("Query receivedSize: %{private}lld, totalSize: %{private}lld, status:%{private}d",
            receivedSize, totalSize, downloadStatus);
        downloadTaskImpl->OnProgress(receivedSize, totalSize);
    }
    else
    {
        DOWNLOAD_HILOGE("downloadTaskImpl is nullptr");
    }
    if (downloadStatus == DOWNLOAD_PAUSE)
    {
        DOWNLOAD_HILOGE("download pause reason: %{private}d", downloadStatus);
        downloadTaskImpl->OnPause();
    }
    else if ((receivedSize == totalSize) && (downloadStatus == DOWNLOAD_SUCCESS)) {
        DOWNLOAD_HILOGI("download completed, receivedSize: %{private}lld, totalSize: %{private}lld",
            receivedSize, totalSize);
        downloadTaskImpl->OnComplete();
    }
    else if (downloadStatus == DOWNLOAD_FAILED) {
        ErrorCode downloadFailedReason = TranslateErrorCode(dmFailedReason);
        DOWNLOAD_HILOGE("download failed, failed reason: %{private}d", downloadFailedReason);
        downloadTaskImpl->OnFail(downloadFailedReason);
    }
    else
    {
        DOWNLOAD_HILOGE("download status: %{private}d", downloadStatus);
    }
    
    env->ReleaseIntArrayElements(downloadDataArray, downloadDataElements, JNI_ABORT);
    DownloadManagerReceiver::OnRequestDataCallback(downloadDataVector);
}

void DownloadManagerJni::OnRequestMimeTypeCallback(JNIEnv* env, jobject obj, jstring jMimeType, jlong jDownloadProgress)
{
    DOWNLOAD_HILOGI("DownloadManager JNI: OnRequestMimeTypeCallback");
    void *downloadProgress = reinterpret_cast<void *>(jDownloadProgress);
    AndroidDownloadTaskImpl *downloadTaskImpl = reinterpret_cast<AndroidDownloadTaskImpl *>(downloadProgress);
    std::string mimeType = JavaStringToString(env, jMimeType);
    downloadTaskImpl->SetMimeType(mimeType);
    DownloadManagerReceiver::OnRequestMimeTypeCallback(mimeType);
}

void DownloadManagerJni::OnRequestNetworkTypeCallback(JNIEnv* env, jobject obj, jint jType, jlong jNetwork)
{
    DOWNLOAD_HILOGI("DownloadManager JNI: OnRequestNetworkTypeCallback");
    void *network = reinterpret_cast<void *>(jNetwork);
    AndroidDownloadTaskImpl *downloadTaskImpl = reinterpret_cast<AndroidDownloadTaskImpl *>(network);
    NetworkType networkType = TranslateNetworkType(jType);
    downloadTaskImpl->SetNetworkType(networkType);
    DownloadManagerReceiver::OnRequestNetworkTypeCallback(networkType);
}

static void GetDownloadHeader(const DownloadConfig &config)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);

    std::map<std::string, std::string> downloadHeader = config.GetHeader();
    std::vector<std::string> headerKeys;
    std::vector<std::string> headerValues;
    for (const auto &header : downloadHeader) {
        std::string key = header.first;
        headerKeys.emplace_back(key);
        std::string value = header.second;
        headerValues.emplace_back(value);
    }

    jstring *jheaderKeys = new jstring[headerKeys.size()];
    jstring *jheaderValues = new jstring[headerValues.size()];
    jobjectArray jKeysArray;
    jobjectArray jValuesArray;
    jclass jcl = env->FindClass("java/lang/String");
    jKeysArray = env->NewObjectArray(headerKeys.size(), jcl, NULL);
    jValuesArray = env->NewObjectArray(headerValues.size(), jcl, NULL);
    for (size_t i = 0; i < headerKeys.size(); i++) {
        jheaderKeys[i] = StringToJavaString(env, headerKeys[i]);
        env->SetObjectArrayElement(jKeysArray, i, jheaderKeys[i]);
    }

    for (size_t j = 0; j < headerValues.size(); j++) {
        jheaderValues[j] = StringToJavaString(env, headerValues[j]);
        env->SetObjectArrayElement(jValuesArray, j, jheaderValues[j]);
    }
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.getHeader, jKeysArray, jValuesArray);
    delete[] jheaderKeys;
    delete[] jheaderValues;
}

static void GetDownloadUrl(const std::string &url)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);

    jstring jUrl = env->NewStringUTF(url.c_str());
    CHECK_NULL_VOID(jUrl);
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.getUrl, jUrl);
    if (env->ExceptionCheck()) {
        DOWNLOAD_HILOGE("GetDownloadUrl has exception");
        env->DeleteLocalRef(jUrl);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    env->DeleteLocalRef(jUrl);
}

static void GetDownloadDesc(const std::string &description)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);

    jstring jDesc = env->NewStringUTF(description.c_str());
    CHECK_NULL_VOID(jDesc);
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.getDescription, jDesc);
    if (env->ExceptionCheck()) {
        DOWNLOAD_HILOGE("GetDownloadDesc has exception");
        env->DeleteLocalRef(jDesc);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    env->DeleteLocalRef(jDesc);
}

static void GetDownloadFilePath(const std::string &filePath)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);

    jstring jPath = env->NewStringUTF(filePath.c_str());
    CHECK_NULL_VOID(jPath);
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.getFilePath, jPath);
    if (env->ExceptionCheck()) {
        DOWNLOAD_HILOGE("GetDownloadFilePath has exception");
        env->DeleteLocalRef(jPath);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    env->DeleteLocalRef(jPath);
}

static void GetDownloadTitle(const std::string &title)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);

    jstring jTitle = env->NewStringUTF(title.c_str());
    CHECK_NULL_VOID(jTitle);
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.getTitle, jTitle);
    if (env->ExceptionCheck()) {
        DOWNLOAD_HILOGE("GetDownloadTitle has exception");
        env->DeleteLocalRef(jTitle);
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    env->DeleteLocalRef(jTitle);
}

static void GetDownloadConfig(const DownloadConfig &config)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);

    GetDownloadUrl(config.GetUrl());
    GetDownloadDesc(config.GetDescription());
    GetDownloadHeader(config);
    GetDownloadFilePath(config.GetFilePath());
    GetDownloadTitle(config.GetTitle());

    bool isEnableMetered = config.IsMetered();
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.getEnableMetered, isEnableMetered);

    bool isEnableRoaming = config.IsRoaming();
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.getEnableRoaming, isEnableRoaming);

    uint32_t downloadNetworkType = config.GetNetworkType();
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.getNetworkType, (jint)downloadNetworkType);

    bool isBackground = config.IsBackground();
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.getBackground, isBackground);

    if (env->ExceptionCheck()) {
        DOWNLOAD_HILOGE("GetDownloadConfig has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
}

void DownloadManagerJni::Download(const DownloadConfig &config, void *downloadProgress)
{
    DOWNLOAD_HILOGI("DownloadManager JNI: start to execute Download");
    CHECK_NULL_VOID(downloadProgress);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    CHECK_NULL_VOID(g_pluginClass.download);

    GetDownloadConfig(config);
    env->CallLongMethod(g_pluginClass.globalRef, g_pluginClass.download, (jlong)downloadProgress);
    if (env->ExceptionCheck()) {
        DOWNLOAD_HILOGE("Download has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    DOWNLOAD_HILOGI("DownloadManager JNI: execute Download success");
}

void DownloadManagerJni::RemoveDownload()
{
    DOWNLOAD_HILOGI("DownloadManager JNI: start to execute RemoveDownload");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    CHECK_NULL_VOID(g_pluginClass.removeDownload);

    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.removeDownload);
    if (env->ExceptionCheck()) {
        DOWNLOAD_HILOGE("RemoveDownload has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    DOWNLOAD_HILOGI("DownloadManager JNI: execute RemoveDownload success");
}

void DownloadManagerJni::PauseDownload()
{
    DOWNLOAD_HILOGI("DownloadManager JNI: start to execute PauseDownload");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    CHECK_NULL_VOID(g_pluginClass.pauseDownload);

    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.pauseDownload);
    if (env->ExceptionCheck()) {
        DOWNLOAD_HILOGE("PauseDownload: has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    DOWNLOAD_HILOGI("DownloadManager JNI: execute PauseDownload success");
}

bool DownloadManagerJni::ResumeDownload()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, false);
    CHECK_NULL_RETURN(g_pluginClass.resumeDownload, false);

    env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.resumeDownload);
    if (env->ExceptionCheck()) {
        DOWNLOAD_HILOGE("ResumeDownload: has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    DOWNLOAD_HILOGI("DownloadManager JNI: execute ResumeDownload success");
    return true;
}

void DownloadManagerJni::GetNetworkType(void *network)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    CHECK_NULL_VOID(g_pluginClass.getNetworkState);

    env->CallIntMethod(g_pluginClass.globalRef, g_pluginClass.getNetworkState, (jlong)network);
    if (env->ExceptionCheck()) {
        DOWNLOAD_HILOGE("GetNetworkType: has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    DOWNLOAD_HILOGI("DownloadManager JNI: execute GetNetworkType success");
}
} // namespace OHOS::Plugin::Request::Download