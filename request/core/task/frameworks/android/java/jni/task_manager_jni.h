/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_REQUEST_ANDROID_JAVA_JNI_TASK_MANAGER_JNI_H
#define PLUGINS_REQUEST_ANDROID_JAVA_JNI_TASK_MANAGER_JNI_H

#include <condition_variable>
#include <jni.h>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "constant.h"
#include "inner_api/plugin_utils_inner.h"
#include "plugin_utils.h"
#include "i_upload_proxy.h"

namespace OHOS::Plugin::Request {
class TaskManagerJni final {
public:
    static TaskManagerJni& Get();
    static bool Register(void* env);

    static void NativeInit(JNIEnv* env, jobject jobj);
    static void OnRequestCallback(JNIEnv *env, jobject obj, jlong taskId, jstring type, jstring info);
  
    int64_t Create(const Config &config);
    int32_t Start(int64_t taskId);
    int32_t Pause(int64_t taskId);
    int32_t Resume(int64_t taskId);
    int32_t Stop(int64_t taskId);
    int32_t Remove(int64_t taskId);
    int32_t GetMimeType(int64_t taskId, std::string &mimeType);
    int32_t Show(int64_t taskId, TaskInfo &info);
    int32_t Touch(int64_t taskId, const std::string &token, TaskInfo &info);
    int32_t Search(const Filter &filter, std::vector<std::string> &taskList);
    int32_t ReportTaskInfo(const TaskInfo &info);
    int32_t GetDefaultStoragePath(std::string& path);

private:
    TaskManagerJni();
    ~TaskManagerJni();
    static void UploadCb(int64_t taskId, const std::string &type, const std::string &params);
    std::string GetRealPath(const Config &config);
    std::string GetPackageName(const std::string &sandBoxPath, const std::string &prefix);
    std::string GetUserId(const std::string &sandBoxPath, const std::string &prefix);
    static bool HandleComplete(const std::string &params);

private:
    std::mutex mutex_;
    std::map<int64_t, std::shared_ptr<IUploadProxy>> uploadProxyList_ {};
};
} // namespace OHOS::Plugin::Request
#endif // PLUGINS_REQUEST_ANDROID_JAVA_JNI_TASK_MANAGER_JNI_H