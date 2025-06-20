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

#include "avmetadatahelper_jni.h"

#include <jni.h>
#include <string.h>
#include <locale>
#include <codecvt>
#include <unistd.h>

#include "media_errors.h"
#include "plugin_utils.h"
#include "avdatasrcmemory.h"
#include "avmetadatahelper.h"
#include "mock_avsharedmemory.h"
#include "interfaces/native/log.h"
#include "inner_api/plugin_utils_inner.h"

namespace OHOS::Plugin {
namespace {
const char AVMetadataHelper_CLASS_NAME[] = "ohos/ace/plugin/avmetadatahelperplugin/AVMetadataHelperPlugin";
static const JNINativeMethod METHODS[] = {
    {"nativeInit", "()V", reinterpret_cast<void *>(AVMetadataHelperJni::NativeInit)},
    {"nativeReadAt", "(JJ[BII)I", reinterpret_cast<void *>(AVMetadataHelperJni::NativeReadAt)},
    {"nativeOnStateChanged", "(JI)V", reinterpret_cast<void *>(AVMetadataHelperJni::NativeOnStateChanged)},
};

static const char METHOD_SET_DATA_SOURCE_WITH_FD[] = "setDataSource";
static const char METHOD_SET_DATA_SOURCE_WITH_DATA_SOURCE[] = "setDataSource";
static const char METHOD_EXTRACT_METADATA[] = "extractMetadata";
static const char METHOD_EXTRACT_METADATA_DONE[] = "extractMetadataDone";
static const char METHOD_GET_EMBEDDED_PICTURE[] = "getEmbeddedPicture";
static const char METHOD_GET_EMBEDDED_PICTURE_SIZE[] = "getEmbeddedPictureSize";
static const char METHOD_CREATE_METADATA_RETRIEVER[] = "createMetadataRetriever";
static const char METHOD_RELEASE_METADATA_RETRIEVER[] = "releaseMetadataRetriever";
static const char METHOD_RELEASE[] = "release";

static const char SIGNATURE_SET_DATA_SOURCE_WITH_FD[] = "(JLjava/lang/String;JJ)V";
static const char SIGNATURE_SET_DATA_SOURCE_WITH_DATA_SOURCE[] = "(J)V";
static const char SIGNATURE_EXTRACT_METADATA[] = "(JI)Ljava/lang/String;";
static const char SIGNATURE_EXTRACT_METADATA_DONE[] = "(J)V";
static const char SIGNATURE_GET_EMBEDDED_PICTURE[] = "(J)[B";
static const char SIGNATURE_GET_EMBEDDED_PICTURE_SIZE[] = "(J)I";
static const char SIGNATURE_CREATE_METADATA_RETRIEVER[] = "(J)V";
static const char SIGNATURE_RELEASE_METADATA_RETRIEVER[] = "(J)V";
static const char SIGNATURE_RELEASE[] = "(J)V";

struct {
    jmethodID setDataSourceWithFd;
    jmethodID setDataSourceWithDataSource;
    jmethodID extractMetadata;
    jmethodID extractMetadataDone;
    jmethodID getEmbeddedPicture;
    jmethodID getEmbeddedPictureSize;
    jmethodID createMetadataRetriever;
    jmethodID releaseMetadataRetriever;
    jmethodID release;
    jobject globalRef;
} g_avmetadataHelperPluginClass;

static jstring StringToJavaString(JNIEnv* env, const std::string &string)
{
    std::u16string str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(string);
    return env->NewString(reinterpret_cast<const jchar *>(str.data()), str.length());
}
}  // namespace

std::map<long, std::shared_ptr<Media::IMediaDataSource>> AVMetadataHelperJni::mediaDataSources_;
std::map<long, std::shared_ptr<Media::HelperCallback>> AVMetadataHelperJni::callbacks_;

bool AVMetadataHelperJni::Register(void *env)
{
    LOGD("AVMetadataHelperJni::Register");
    auto *jniEnv = static_cast<JNIEnv *>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(AVMetadataHelper_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("AVMetadataHelperJni JNI: RegisterNatives fail.");
        return false;
    }

    return true;
}

void AVMetadataHelperJni::NativeInit(JNIEnv *env, jobject jobj)
{
    LOGD("AVMetadataHelperJni::NativeInit");
    CHECK_NULL_VOID(env);
    g_avmetadataHelperPluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_avmetadataHelperPluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_avmetadataHelperPluginClass.setDataSourceWithFd = env->GetMethodID(
        cls, METHOD_SET_DATA_SOURCE_WITH_FD, SIGNATURE_SET_DATA_SOURCE_WITH_FD);
    CHECK_NULL_VOID(g_avmetadataHelperPluginClass.setDataSourceWithFd);

    g_avmetadataHelperPluginClass.setDataSourceWithDataSource = env->GetMethodID(
        cls, METHOD_SET_DATA_SOURCE_WITH_DATA_SOURCE, SIGNATURE_SET_DATA_SOURCE_WITH_DATA_SOURCE);
    CHECK_NULL_VOID(g_avmetadataHelperPluginClass.setDataSourceWithDataSource);

    g_avmetadataHelperPluginClass.extractMetadata = env->GetMethodID(
        cls, METHOD_EXTRACT_METADATA, SIGNATURE_EXTRACT_METADATA);
    CHECK_NULL_VOID(g_avmetadataHelperPluginClass.extractMetadata);

    g_avmetadataHelperPluginClass.extractMetadataDone = env->GetMethodID(
        cls, METHOD_EXTRACT_METADATA_DONE, SIGNATURE_EXTRACT_METADATA_DONE);
    CHECK_NULL_VOID(g_avmetadataHelperPluginClass.extractMetadataDone);

    g_avmetadataHelperPluginClass.getEmbeddedPicture = env->GetMethodID(
        cls, METHOD_GET_EMBEDDED_PICTURE, SIGNATURE_GET_EMBEDDED_PICTURE);
    CHECK_NULL_VOID(g_avmetadataHelperPluginClass.getEmbeddedPicture);

    g_avmetadataHelperPluginClass.getEmbeddedPictureSize = env->GetMethodID(
        cls, METHOD_GET_EMBEDDED_PICTURE_SIZE, SIGNATURE_GET_EMBEDDED_PICTURE_SIZE);
    CHECK_NULL_VOID(g_avmetadataHelperPluginClass.getEmbeddedPictureSize);

    g_avmetadataHelperPluginClass.createMetadataRetriever = env->GetMethodID(
        cls, METHOD_CREATE_METADATA_RETRIEVER, SIGNATURE_CREATE_METADATA_RETRIEVER);
    CHECK_NULL_VOID(g_avmetadataHelperPluginClass.createMetadataRetriever);

    g_avmetadataHelperPluginClass.releaseMetadataRetriever = env->GetMethodID(
        cls, METHOD_RELEASE_METADATA_RETRIEVER, SIGNATURE_RELEASE_METADATA_RETRIEVER);
    CHECK_NULL_VOID(g_avmetadataHelperPluginClass.releaseMetadataRetriever);

    g_avmetadataHelperPluginClass.release = env->GetMethodID(cls, METHOD_RELEASE, SIGNATURE_RELEASE);
    CHECK_NULL_VOID(g_avmetadataHelperPluginClass.release);

    env->DeleteLocalRef(cls);
    return;
}

jint AVMetadataHelperJni::NativeReadAt(
        JNIEnv *env, jobject jthiz, jlong key, jlong position, jbyteArray buffer, jint offset, jint size)
{
    CHECK_NULL_RETURN(env, 0);
    auto iter = mediaDataSources_.find((long)key);
    if (iter == mediaDataSources_.end()) {
        LOGD("AVMetadataHelperJni NativeReadAt callback not found");
        return 0;
    }
    std::shared_ptr<Media::MockAVSharedMemory> dataSrc_ptr =
        std::make_shared<Media::MockAVSharedMemory>((int)size, Media::AVSharedMemory::Flags::FLAGS_READ_WRITE);
    (iter->second)->ReadAt(dataSrc_ptr, (uint32_t)size, (int64_t)position);
    jbyte *data = (jbyte *)dataSrc_ptr->GetBase();

    env->SetByteArrayRegion(buffer, 0, (jint)dataSrc_ptr->GetSize(), data);
    return (jint)dataSrc_ptr->GetSize();
}

void AVMetadataHelperJni::CreateMetadataRetriever(long key)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_avmetadataHelperPluginClass.globalRef);
    CHECK_NULL_VOID(g_avmetadataHelperPluginClass.createMetadataRetriever);

    env->CallVoidMethod(
        g_avmetadataHelperPluginClass.globalRef, g_avmetadataHelperPluginClass.createMetadataRetriever, (jlong)key);
    if (env->ExceptionCheck()) {
        LOGE("AVMetadataHelperJni JNI: call CreateMetadataRetriever has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
}

void AVMetadataHelperJni::ReleaseMetadataRetriever(long key)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_avmetadataHelperPluginClass.globalRef);
    CHECK_NULL_VOID(g_avmetadataHelperPluginClass.releaseMetadataRetriever);

    env->CallVoidMethod(
        g_avmetadataHelperPluginClass.globalRef, g_avmetadataHelperPluginClass.releaseMetadataRetriever, (jlong)key);
    if (env->ExceptionCheck()) {
        LOGE("AVMetadataHelperJni JNI: call ReleaseMetadataRetriever has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
    mediaDataSources_.erase(key);
    callbacks_.erase(key);
}

int32_t AVMetadataHelperJni::SetHelperCallback(long key, const std::shared_ptr<Media::HelperCallback> &callback)
{
    auto iter = callbacks_.find(key);
    if (iter != callbacks_.end()) {
        return Media::MSERR_OK;
    }

    callbacks_[key] = callback;
    return Media::MSERR_OK;
}

int32_t AVMetadataHelperJni::SetSource(long key, int32_t fd, int64_t offset, int64_t size, int32_t usage)
{
    LOGD("AVMetadataHelperJni JNI: SetSource in.");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_avmetadataHelperPluginClass.globalRef,  Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_avmetadataHelperPluginClass.setDataSourceWithFd,
         Media::MSERR_UNKNOWN);

    char filePath[PATH_MAX] = {'\0'};
    char buf[128] = {'\0'};

    snprintf(buf, sizeof(buf), "proc/self/fd/%d", fd);

    if (readlink(buf, filePath, PATH_MAX) < 0) {
        return Media::MSERR_SERVICE_DIED;
    }

    LOGD("AVMetadataHelperJni JNI: SetSource %{public}s.", filePath);
    env->CallVoidMethod(g_avmetadataHelperPluginClass.globalRef, g_avmetadataHelperPluginClass.setDataSourceWithFd,
        (jlong)key, StringToJavaString(env, filePath), (jlong)offset, (jlong)size);

    if (env->ExceptionCheck()) {
        LOGE("AVMetadataHelperJni JNI: call setDataSourceWithFd has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

int32_t AVMetadataHelperJni::SetSource(long key, const std::shared_ptr<Media::IMediaDataSource> &dataSrc)
{
    mediaDataSources_[key] = dataSrc;

    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_avmetadataHelperPluginClass.globalRef,  Media::MSERR_UNKNOWN);
    CHECK_NULL_RETURN(g_avmetadataHelperPluginClass.setDataSourceWithDataSource,
        Media::MSERR_UNKNOWN);

    env->CallVoidMethod(
        g_avmetadataHelperPluginClass.globalRef, g_avmetadataHelperPluginClass.setDataSourceWithDataSource,
        (jlong)key);
    if (env->ExceptionCheck()) {
        LOGE("AVMetadataHelperJni JNI: call setDataSourceWithDatasrc has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return Media::MSERR_INVALID_OPERATION;
    }
    return Media::MSERR_OK;
}

std::unordered_map<int32_t, std::string> AVMetadataHelperJni::ResolveMetadata(long key)
{
    LOGD("AVMetadataHelperJni JNI: ResolveMetadata in.");
    std::unordered_map<int32_t, std::string> ret;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, ret);
    CHECK_NULL_RETURN(g_avmetadataHelperPluginClass.globalRef,  ret);
    CHECK_NULL_RETURN(g_avmetadataHelperPluginClass.extractMetadata, ret);
    CHECK_NULL_RETURN(g_avmetadataHelperPluginClass.extractMetadataDone, ret);

    int avKey[] = {
        Media::AV_KEY_ALBUM,
        Media::AV_KEY_ALBUM_ARTIST,
        Media::AV_KEY_ARTIST,
        Media::AV_KEY_AUTHOR,
        Media::AV_KEY_DATE_TIME,
        Media::AV_KEY_DATE_TIME_FORMAT,
        Media::AV_KEY_COMPOSER,
        Media::AV_KEY_DURATION,
        Media::AV_KEY_GENRE,
        Media::AV_KEY_HAS_AUDIO,
        Media::AV_KEY_HAS_VIDEO,
        Media::AV_KEY_MIME_TYPE,
        Media::AV_KEY_NUM_TRACKS,
        Media::AV_KEY_SAMPLE_RATE,
        Media::AV_KEY_TITLE,
        Media::AV_KEY_VIDEO_HEIGHT,
        Media::AV_KEY_VIDEO_WIDTH,
        Media::AV_KEY_VIDEO_ORIENTATION,
    };
    for (int i = 0; i < METADATA_KEY_SUM; i++) {
        int keyCode = ConvertAVKeytoMetadataKey(avKey[i]);
        jstring value = (jstring)env->CallObjectMethod(
            g_avmetadataHelperPluginClass.globalRef, g_avmetadataHelperPluginClass.extractMetadata,
            (jlong)key, (jint)keyCode);
        if (env->ExceptionCheck()) {
            LOGE("AVMetadataHelperJni JNI: call ResolveMetadata has exception");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return ret;
        }
        const char* utf8Str = env->GetStringUTFChars(value, NULL);
        int len = strlen(utf8Str);
        if (len > 0) {
            std::string result(utf8Str, len);
            ret[avKey[i]] = result;
        }
    }

    env->CallVoidMethod(
        g_avmetadataHelperPluginClass.globalRef, g_avmetadataHelperPluginClass.extractMetadataDone, (jlong)key);
    if (env->ExceptionCheck()) {
        LOGE("AVMetadataHelperJni JNI: call ResolveMetadata Done has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return ret;
    }

    return ret;
}

std::shared_ptr<Media::AVSharedMemory> AVMetadataHelperJni::FetchArtPicture(long key)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, nullptr);
    CHECK_NULL_RETURN(g_avmetadataHelperPluginClass.globalRef,  nullptr);
    CHECK_NULL_RETURN(g_avmetadataHelperPluginClass.getEmbeddedPicture, nullptr);
    CHECK_NULL_RETURN(g_avmetadataHelperPluginClass.getEmbeddedPictureSize, nullptr);

    jint picSize = env->CallIntMethod(
        g_avmetadataHelperPluginClass.globalRef, g_avmetadataHelperPluginClass.getEmbeddedPictureSize,
        (jlong)key);
    if (env->ExceptionCheck()) {
        LOGE("AVMetadataHelperJni JNI: call FetchArtPicture has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }

    if ((int) picSize < 0) {
        LOGE("AVMetadataHelperJni JNI: FetchArtPicture no picture");
        return nullptr;
    }

    jbyteArray jarr = env->NewByteArray((jsize)picSize);
    jarr = (jbyteArray)env->CallObjectMethod(
        g_avmetadataHelperPluginClass.globalRef, g_avmetadataHelperPluginClass.getEmbeddedPicture, (jlong)key);
    if (jarr == nullptr) {
        LOGE("AVMetadataHelperJni JNI: jarr is null after CallObjectMethod");
        return nullptr;
    }
    if (env->ExceptionCheck()) {
        LOGE("AVMetadataHelperJni JNI: call FetchArtPicture has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }

    jint len = env->GetArrayLength(jarr);

    std::shared_ptr<Media::MockAVSharedMemory> dataSrc_ptr =
        std::make_shared<Media::MockAVSharedMemory>((int)len, Media::AVSharedMemory::Flags::FLAGS_READ_WRITE);

    env->GetByteArrayRegion(jarr, 0, len, reinterpret_cast<jbyte*>(dataSrc_ptr->GetBase()));
    env->DeleteLocalRef(jarr);
    return dataSrc_ptr;
}

void AVMetadataHelperJni::Release(long key)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_avmetadataHelperPluginClass.globalRef);
    CHECK_NULL_VOID(g_avmetadataHelperPluginClass.release);

    env->CallVoidMethod(
        g_avmetadataHelperPluginClass.globalRef, g_avmetadataHelperPluginClass.release, (jlong)key);
    if (env->ExceptionCheck()) {
        LOGE("AVMetadataHelperJni JNI: call Release has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

int32_t AVMetadataHelperJni::ConvertAVKeytoMetadataKey(int32_t key)
{
    switch(key) {
        case Media::AV_KEY_ALBUM:
            return METADATA_KEY_ALBUM;
        case Media::AV_KEY_ALBUM_ARTIST:
            return METADATA_KEY_ALBUMARTIST;
        case Media::AV_KEY_ARTIST:
            return METADATA_KEY_ARTIST;
        case Media::AV_KEY_AUTHOR:
            return METADATA_KEY_AUTHOR;
        case Media::AV_KEY_DATE_TIME:
            return METADATA_KEY_YEAR;
        case Media::AV_KEY_DATE_TIME_FORMAT:
            return METADATA_KEY_DATE;
        case Media::AV_KEY_COMPOSER:
            return METADATA_KEY_COMPOSER;
        case Media::AV_KEY_DURATION:
            return METADATA_KEY_DURATION;
        case Media::AV_KEY_GENRE:
            return METADATA_KEY_GENRE;
        case Media::AV_KEY_HAS_AUDIO:
            return METADATA_KEY_HAS_AUDIO;
        case Media::AV_KEY_HAS_VIDEO:
            return METADATA_KEY_HAS_VIDEO;
        case Media::AV_KEY_MIME_TYPE:
            return METADATA_KEY_MIMETYPE;
        case Media::AV_KEY_NUM_TRACKS:
            return METADATA_KEY_NUM_TRACKS;
        case Media::AV_KEY_SAMPLE_RATE:
            return METADATA_KEY_SAMPLERATE;
        case Media::AV_KEY_TITLE:
            return METADATA_KEY_TITLE;
        case Media::AV_KEY_VIDEO_HEIGHT:
            return METADATA_KEY_VIDEO_HEIGHT;
        case Media::AV_KEY_VIDEO_WIDTH:
            return METADATA_KEY_VIDEO_WIDTH;
        case Media::AV_KEY_VIDEO_ORIENTATION:
            return METADATA_KEY_VIDEO_ROTATION;
        default:
            return -1;
    }
}

void AVMetadataHelperJni::NativeOnStateChanged(JNIEnv *env, jobject jthiz, jlong key, jint state)
{
    auto iter = callbacks_.find((long)key);
    if (iter == callbacks_.end()) {
        LOGD("AVMetadataHelperJni NativeOnStateChanged callback not found");
        return;
    }
    Media::Format infoBody;
    (iter->second)->OnInfo(Media::HelperOnInfoType::HELPER_INFO_TYPE_STATE_CHANGE, (int32_t)state, infoBody);
}
} // namespace OHOS::Plugin
