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

#include "task_manager_jni.h"

#include <codecvt>
#include <jni.h>
#include <locale>

#include "config_json.h"
#include "constant.h"
#include "filter_json.h"
#include "log.h"
#include "progress_json.h"
#include "request_utils.h"
#include "task_info_json.h"
#include "task_notify_manager.h"
#include "upload_proxy.h"

namespace OHOS::Plugin::Request {
const char TASKMANAGER_PLUGIN_CLASS_NAME[] = "ohos/ace/plugin/taskmanagerplugin/JavaTaskImpl";
static constexpr uint32_t GET_RESULT_TIMEOUT = 3000;
static const JNINativeMethod METHODS[] = {
    {"nativeInit", "()V", reinterpret_cast<void *>(TaskManagerJni::NativeInit)},
    {"onRequestCallback", "(JLjava/lang/String;Ljava/lang/String;)V",
     reinterpret_cast<void *>(TaskManagerJni::OnRequestCallback)},
};

const char METHOD_CREATE_TASK[] = "create";
const char SIGNATURE_CREATE_TASK[] = "(Ljava/lang/String;)J";

const char METHOD_START_TASK[] = "start";
const char SIGNATURE_START_TASK[] = "(J)V";

const char METHOD_PAUSE_TASK[] = "pause";
const char SIGNATURE_PAUSE_TASK[] = "(J)V";

const char METHOD_RESUME_TASK[] = "resume";
const char SIGNATURE_RESUME_TASK[] = "(J)V";

const char METHOD_STOP_TASK[] = "stop";
const char SIGNATURE_STOP_TASK[] = "(J)V";

const char METHOD_REMOVE_TASK[] = "remove";
const char SIGNATURE_REMOVE_TASK[] = "(J)J";

const char METHOD_GET_MIME[] = "getMimeType";
const char SIGNATURE_GET_MIME[] = "(J)Ljava/lang/String;";

const char METHOD_SHOW_TASK[] = "show";
const char SIGNATURE_SHOW_TASK[] = "(J)Ljava/lang/String;";

const char METHOD_TOUCH_TASK[] = "touch";
const char SIGNATURE_TOUCH_TASK[] = "(JLjava/lang/String;)Ljava/lang/String;";

const char METHOD_SEARCH_TASK[] = "search";
const char SIGNATURE_SEARCH_TASK[] = "(Ljava/lang/String;)[J";

const char METHOD_REPORT_INFO[] = "reportTaskInfo";
const char SIGNATURE_REPORT_INFO[] = "(Ljava/lang/String;)V";

const char METHOD_REPORT_STORAGE_PATH[] = "getDefaultStoragePath";
const char SIGNATURE_REPORT_STORAGE_PATH[] = "()Ljava/lang/String;";
struct {
    jmethodID createTask;
    jmethodID startTask;
    jmethodID pauseTask;
    jmethodID resumeTask;
    jmethodID stopTask;
    jmethodID removeTask;
    jmethodID getMime;
    jmethodID showTask;
    jmethodID touchTask;
    jmethodID searchTask;
    jmethodID reportTaskInfo;
    jmethodID getDefaultStoragePath;
    jobject globalRef;
} g_pluginClass;

TaskManagerJni& TaskManagerJni::Get()
{
    static TaskManagerJni instance;
    return instance;
}

TaskManagerJni::TaskManagerJni()
{
}

TaskManagerJni::~TaskManagerJni()
{
    uploadProxyList_.clear();
}

bool TaskManagerJni::Register(void *env)
{
    REQUEST_HILOGI("TaskManagerJni JNI: Register");
    auto *jniEnv = static_cast<JNIEnv *>(env);
    if (!jniEnv) {
        REQUEST_HILOGE("JNIEnv is null when registering TaskManagerJni jni!");
        return false;
    }

    jclass cls = jniEnv->FindClass(TASKMANAGER_PLUGIN_CLASS_NAME);
    if (cls == nullptr) {
        REQUEST_HILOGE("Failed to find TaskManagerJni class: %{public}s", TASKMANAGER_PLUGIN_CLASS_NAME);
        return false;
    }

    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    if (!ret) {
        REQUEST_HILOGE("TaskManagerJni JNI: RegisterNatives fail.");
        return false;
    }

    REQUEST_HILOGI("TaskManagerJni JNI: Register success");
    return true;
}

static std::string UTF16StringToUTF8String(const char16_t* chars, size_t len)
{
    REQUEST_HILOGI("TaskManagerJni JNI: UTF16StringToUTF8String");
    std::u16string u16_string(chars, len);
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.to_bytes(u16_string);
}

static std::string JavaStringToString(JNIEnv* env, const jstring &str)
{
    if (env == nullptr || str == nullptr) {
        return "";
    }
    const jchar *chars = env->GetStringChars(str, NULL);
    if (chars == nullptr) {
        return "";
    }

    auto u8_string = UTF16StringToUTF8String(reinterpret_cast<const char16_t *>(chars), env->GetStringLength(str));
    env->ReleaseStringChars(str, chars);
    return u8_string;
}

static jstring StringToJavaString(JNIEnv *env, const std::string &string)
{
    std::u16string str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(string);
    return env->NewString(reinterpret_cast<const jchar *>(str.data()), str.length());
}

void TaskManagerJni::NativeInit(JNIEnv *env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    g_pluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_pluginClass.globalRef);

    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_pluginClass.createTask = env->GetMethodID(cls, METHOD_CREATE_TASK, SIGNATURE_CREATE_TASK);
    CHECK_NULL_VOID(g_pluginClass.createTask);

    g_pluginClass.startTask = env->GetMethodID(cls, METHOD_START_TASK, SIGNATURE_START_TASK);
    CHECK_NULL_VOID(g_pluginClass.startTask);

    g_pluginClass.pauseTask = env->GetMethodID(cls, METHOD_PAUSE_TASK, SIGNATURE_PAUSE_TASK);
    CHECK_NULL_VOID(g_pluginClass.pauseTask);

    g_pluginClass.resumeTask = env->GetMethodID(cls, METHOD_RESUME_TASK, SIGNATURE_RESUME_TASK);
    CHECK_NULL_VOID(g_pluginClass.resumeTask);

    g_pluginClass.stopTask = env->GetMethodID(cls, METHOD_STOP_TASK, SIGNATURE_STOP_TASK);
    CHECK_NULL_VOID(g_pluginClass.stopTask);

    g_pluginClass.removeTask = env->GetMethodID(cls, METHOD_REMOVE_TASK, SIGNATURE_REMOVE_TASK);
    CHECK_NULL_VOID(g_pluginClass.removeTask);

    g_pluginClass.getMime = env->GetMethodID(cls, METHOD_GET_MIME, SIGNATURE_GET_MIME);
    CHECK_NULL_VOID(g_pluginClass.getMime);

    g_pluginClass.showTask = env->GetMethodID(cls, METHOD_SHOW_TASK, SIGNATURE_SHOW_TASK);
    CHECK_NULL_VOID(g_pluginClass.showTask);

    g_pluginClass.touchTask = env->GetMethodID(cls, METHOD_TOUCH_TASK, SIGNATURE_TOUCH_TASK);
    CHECK_NULL_VOID(g_pluginClass.touchTask);

    g_pluginClass.searchTask = env->GetMethodID(cls, METHOD_SEARCH_TASK, SIGNATURE_SEARCH_TASK);
    CHECK_NULL_VOID(g_pluginClass.searchTask);

    g_pluginClass.reportTaskInfo = env->GetMethodID(cls, METHOD_REPORT_INFO, SIGNATURE_REPORT_INFO);
    CHECK_NULL_VOID(g_pluginClass.reportTaskInfo);

    g_pluginClass.getDefaultStoragePath = env->GetMethodID(cls, METHOD_REPORT_STORAGE_PATH, SIGNATURE_REPORT_STORAGE_PATH);
    CHECK_NULL_VOID(g_pluginClass.getDefaultStoragePath);
    env->DeleteLocalRef(cls);
}

void TaskManagerJni::OnRequestCallback(JNIEnv *env, jobject obj, jlong taskId, jstring type, jstring info)
{
    REQUEST_HILOGI("TaskManagerJni JNI: OnRequestDataCallback");
    auto eventType = JavaStringToString(env, type);
    auto infoParam = JavaStringToString(env, info);
    if (eventType == EVENT_COMPLETED) {
        if (!HandleComplete(infoParam)) {
            eventType = EVENT_FAILED;
        }
    }
    TaskNotifyManager::Get().SendNotify(RequestUtils::GetEventType(taskId, eventType), infoParam);
    REQUEST_HILOGI("TaskManagerJni JNI: OnRequestDataCallback end");
}

int64_t TaskManagerJni::Create(const Config &config)
{
    REQUEST_HILOGI("TaskManagerJni JNI: Create Task");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.createTask, E_SERVICE_ERROR);

    Config newConfig = config;
    if (config.action == Action::DOWNLOAD) {
        newConfig.saveas = GetRealPath(config);
    }
    Json json = newConfig;
    auto jConfig = StringToJavaString(env, json.dump());
    auto taskId = env->CallLongMethod(g_pluginClass.globalRef, g_pluginClass.createTask, jConfig);
    if (env->ExceptionCheck()) {
        REQUEST_HILOGE("create task has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INVALID_TASK_ID;
    }
    REQUEST_HILOGI("TaskManagerJni JNI: execute Create success.");
    std::unique_lock<std::mutex> lock(mutex_);
    if (config.action == Action::UPLOAD) {
        auto uploadProxy = std::make_shared<UploadProxy>(taskId, config);
        if (uploadProxy == nullptr) {
            REQUEST_HILOGE("create upload proxy  has exception");
            return INVALID_TASK_ID;
        }
        uploadProxyList_.emplace(taskId, uploadProxy);
    }
    return taskId;
}

int32_t TaskManagerJni::Start(int64_t taskId)
{
    REQUEST_HILOGI("TaskManagerJni JNI: start to execute Task");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.startTask, E_SERVICE_ERROR);

    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.startTask, taskId);
    if (env->ExceptionCheck()) {
        REQUEST_HILOGE("start task has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return E_SERVICE_ERROR;
    }
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = uploadProxyList_.find(taskId);
    if (it != uploadProxyList_.end() && it->second != nullptr) {
        it->second->Start(TaskManagerJni::UploadCb);
    }
    REQUEST_HILOGI("TaskManagerJni JNI: execute start success.");
    return E_OK;
}

int32_t TaskManagerJni::Pause(int64_t taskId)
{
    REQUEST_HILOGI("TaskManagerJni JNI: Pause Task");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.pauseTask, E_SERVICE_ERROR);

    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.pauseTask, taskId);
    if (env->ExceptionCheck()) {
        REQUEST_HILOGE("pause task has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return E_SERVICE_ERROR;
    }
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = uploadProxyList_.find(taskId);
    if (it != uploadProxyList_.end() && it->second != nullptr) {
        it->second->Pause();
    }
    REQUEST_HILOGI("TaskManagerJni JNI: execute pause success.");
    return E_OK;
}

int32_t TaskManagerJni::Resume(int64_t taskId)
{
    REQUEST_HILOGI("TaskManagerJni JNI: Resume Task");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.resumeTask, E_SERVICE_ERROR);

    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.resumeTask, taskId);
    if (env->ExceptionCheck()) {
        REQUEST_HILOGE("resume task has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return E_SERVICE_ERROR;
    }
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = uploadProxyList_.find(taskId);
    if (it != uploadProxyList_.end() && it->second != nullptr) {
        it->second->Resume();
    }
    REQUEST_HILOGI("TaskManagerJni JNI: execute resume success.");
    return E_OK;
}

int32_t TaskManagerJni::Stop(int64_t taskId)
{
    REQUEST_HILOGI("TaskManagerJni JNI: Stop Task");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.stopTask, E_SERVICE_ERROR);

    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.stopTask, taskId);
    if (env->ExceptionCheck()) {
        REQUEST_HILOGE("stop task has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return E_SERVICE_ERROR;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    auto it = uploadProxyList_.find(taskId);
    if (it != uploadProxyList_.end() && it->second != nullptr) {
        it->second->Stop();
    }
    
    REQUEST_HILOGI("TaskManagerJni JNI: execute stop success.");
    return E_OK;
}

int32_t TaskManagerJni::Remove(int64_t taskId)
{
    REQUEST_HILOGI("TaskManagerJni JNI: Remove Task");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.removeTask, E_SERVICE_ERROR);
    if (env->CallLongMethod(g_pluginClass.globalRef, g_pluginClass.removeTask, taskId) != 0) {  
        if (env->ExceptionCheck()) {
            REQUEST_HILOGE("remove task has exception");
            env->ExceptionDescribe();
            env->ExceptionClear();
            return E_SERVICE_ERROR;
        }
        else{
            return E_TASK_NOT_FOUND;
        }
    }

    std::unique_lock<std::mutex> lock(mutex_);
    auto it = uploadProxyList_.find(taskId);
    if (it != uploadProxyList_.end() && it->second != nullptr) {
        it->second->Remove();
        uploadProxyList_.erase(it);
    }

    REQUEST_HILOGI("TaskManagerJni JNI: execute remove success.");
    return E_OK;
}

int32_t TaskManagerJni::GetMimeType(int64_t taskId, std::string &mimeType)
{
    REQUEST_HILOGI("TaskManagerJni JNI: Get Mime");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.getMime, E_SERVICE_ERROR);

    auto mime = static_cast<jstring>(env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getMime, taskId));
    if (env->ExceptionCheck()) {
        REQUEST_HILOGE("get mime has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return E_SERVICE_ERROR;
    }
    mimeType = JavaStringToString(env, mime);
    REQUEST_HILOGI("TaskManagerJni JNI: execute get mime success.");
    return E_OK;
}

int32_t TaskManagerJni::Show(int64_t taskId, TaskInfo &info)
{
    REQUEST_HILOGI("TaskManagerJni JNI: Show Task");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.showTask, E_SERVICE_ERROR);

    jstring taskInfo =
        static_cast<jstring>(env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.showTask, taskId));
    if (env->ExceptionCheck()) {
        REQUEST_HILOGE("show task has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return E_SERVICE_ERROR;
    }
    auto infoJson = Json::parse(JavaStringToString(env, taskInfo).c_str(), nullptr, false);
    if (infoJson.is_null() || infoJson.is_discarded()) {
        REQUEST_HILOGE("invalid json of task info");
        return E_TASK_NOT_FOUND;
    }
    info = infoJson.get<TaskInfo>();
    REQUEST_HILOGI("TaskManagerJni JNI: execute show task success.");
    return E_OK;
}

int32_t TaskManagerJni::Touch(int64_t taskId, const std::string &token, TaskInfo &info)
{
    REQUEST_HILOGI("TaskManagerJni JNI: Touch Task");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.touchTask, E_SERVICE_ERROR);

    auto jToken = StringToJavaString(env, token);
    jstring taskInfo =
        static_cast<jstring>(env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.touchTask, taskId, jToken));
    if (env->ExceptionCheck()) {
        REQUEST_HILOGE("touch task has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return E_SERVICE_ERROR;
    }
    auto infoJson = Json::parse(JavaStringToString(env, taskInfo).c_str(), nullptr, false);
    if (infoJson.is_null() || infoJson.is_discarded()) {
        REQUEST_HILOGE("invalid json of task info");
        return E_TASK_NOT_FOUND;
    }
    info = infoJson.get<TaskInfo>();
    REQUEST_HILOGI("TaskManagerJni JNI: execute touch task success.");
    return E_OK;
}

int32_t TaskManagerJni::Search(const Filter &filter, std::vector<std::string> &taskList)
{
    REQUEST_HILOGI("TaskManagerJni JNI: Search Task");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.searchTask, E_SERVICE_ERROR);

    Json json = filter;
    auto jFilter = StringToJavaString(env, json.dump());
    jlongArray resultList =
        static_cast<jlongArray>(env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.searchTask, jFilter));
    if (env->ExceptionCheck()) {
        REQUEST_HILOGE("touch search has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return E_SERVICE_ERROR;
    }
    jsize size = env->GetArrayLength(resultList);
    jlong *dataElements = env->GetLongArrayElements(resultList, nullptr);
    taskList.clear();
    for (jsize index = 0; index < size; index++) {
        taskList.emplace_back(std::to_string(dataElements[index]));
    }
    REQUEST_HILOGI("TaskManagerJni JNI: execute search task success.");
    return E_OK;
}

int32_t TaskManagerJni::ReportTaskInfo(const TaskInfo &info)
{
    REQUEST_HILOGI("TaskManagerJni JNI: Report Task Info");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.reportTaskInfo, E_SERVICE_ERROR);

    Json json = info;
    auto jInfo = StringToJavaString(env, json.dump());
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.reportTaskInfo, jInfo);
    if (env->ExceptionCheck()) {
        REQUEST_HILOGE("touch search has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return E_SERVICE_ERROR;
    }
    REQUEST_HILOGI("TaskManagerJni JNI: ReportTaskInfo success.");
    return E_OK;
}

int32_t TaskManagerJni::GetDefaultStoragePath(std::string& path)
{
    REQUEST_HILOGI("TaskManagerJni JNI: Report default storage path start");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, E_SERVICE_ERROR);
    CHECK_NULL_RETURN(g_pluginClass.getDefaultStoragePath, E_SERVICE_ERROR);

    jstring jpath =
    static_cast<jstring>(env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getDefaultStoragePath));
    if (env->ExceptionCheck()) {
        REQUEST_HILOGE("report default storage has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return E_SERVICE_ERROR;
    }
    path = JavaStringToString(env, jpath);

    REQUEST_HILOGI("TaskManagerJni JNI: ReportDefaultStoragePath end. path: %{public}s", path.c_str());
    return E_OK;
}

void TaskManagerJni::UploadCb(int64_t taskId, const std::string &type, const std::string &params)
{
    REQUEST_HILOGI("TaskManagerJni JNI: UploadCb params: %{public}s", params.c_str());
    TaskNotifyManager::Get().SendNotify(RequestUtils::GetEventType(taskId, type), params);
}

std::string TaskManagerJni::GetRealPath(const Config &config)
{
    auto filePath = config.saveas;
    const std::string prefix = "/data/user/";
    if (filePath.find(prefix) == 0) {
        std::string userId = GetUserId(filePath, prefix);
        std::string packageName = GetPackageName(filePath, prefix);
        if (userId.empty() || packageName.empty()) {
            REQUEST_HILOGE("fail to extra user id or packagename");
            return filePath;
        }

        std::string fileName = filePath.substr(filePath.rfind('/') + 1);
        filePath = "/storage/emulated/" + userId + "/Android/data/" + packageName + "/files/" + fileName;
    }
    return filePath;
}

std::string TaskManagerJni::GetUserId(const std::string &sandBoxPath, const std::string &prefix)
{
    std::string userId("");
    if (sandBoxPath.find(prefix) != std::string::npos) {
        std::string temp = sandBoxPath.substr(prefix.length());
        size_t pos = temp.find('/');
        if (pos != std::string::npos) {
            userId = temp.substr(0, pos);
        }
    }
    return userId;
}

std::string TaskManagerJni::GetPackageName(const std::string &sandBoxPath, const std::string &prefix)
{
    std::string packageName("");
    std::string userId = GetUserId(sandBoxPath, prefix);
    if (!userId.empty()) {
        size_t pos = sandBoxPath.find(userId);
        if (pos != std::string::npos) {
            std::string temp = sandBoxPath.substr(pos + userId.length() + 1);
            pos = temp.find('/');
            if (pos != std::string::npos) {
                packageName = temp.substr(0, pos);
            }
        }
    }
    return packageName;
}
const std::string JSON_SAVEAS = "saveas";
const std::string JSON_FILES = "files";

bool TaskManagerJni::HandleComplete(const std::string &params)
{
    auto infoJson = nlohmann::json::parse(params.c_str(), nullptr, false);
    if (infoJson.is_null() || infoJson.is_discarded()) {
        REQUEST_HILOGE("invalid json of task info");
        return false;
    }
    std::string saveas = "";
    if (infoJson.find(JSON_SAVEAS) != infoJson.end() && infoJson[JSON_SAVEAS].is_string()) {
        infoJson.at(JSON_SAVEAS).get_to(saveas);
    }
    auto info = infoJson.get<TaskInfo>();
    if (!info.files.empty() && saveas != info.files[0].uri) {
        std::string copyFile = "cp " + saveas + " " + info.files[0].uri;
        std::string removeFile = "rm -rf " + saveas;
        REQUEST_HILOGI("copyFile = %{public}s", copyFile.c_str());
        if (system(copyFile.c_str()) != 0) {
            REQUEST_HILOGE("%s failed", copyFile.c_str());
            system(removeFile.c_str());
            return false;
        }
        system(removeFile.c_str());
    }
    return true;
}
} // namespace OHOS::Plugin::Request