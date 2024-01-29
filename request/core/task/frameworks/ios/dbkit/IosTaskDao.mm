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

#include "IosTaskDao.h"
#import "Constants.h"
#import "DBManager.h"
#import "IosTaskConfig.h"
#import "IosTaskInfo.h"
#import "json_utils.h"
#include "log.h"

namespace OHOS::Plugin::Request {

IosTaskDao::~IosTaskDao()
{
}

int64_t IosTaskDao::CreateTask(const Config &config)
{
    NSLog(@"CreateTask enter");
    if (![[DBManager shareManager] initDB]) {
        NSLog(@"failed to init database");
        return INVALID_TASK_ID;
    }

    std::string strConfig = JsonUtils::ConfigToJsonString(config);
    NSString *jsonConfig = JsonUtils::CStringToNSString(strConfig);
    NSLog(@"CreateTask, jsonConfig:%@", jsonConfig);
    IosTaskConfig *taskConfig = [IosTaskConfig initWithJsonString:jsonConfig];
    if (taskConfig == nil) {
        NSLog(@"failed to create task, config is nil");
        return INVALID_TASK_ID;
    }

    return [[DBManager shareManager] insert:[taskConfig getTaskInfo]];
}

int32_t IosTaskDao::RemoveTask(int64_t taskId)
{
    NSLog(@"RemoveTask taskid: %lld", taskId);
    if (![[DBManager shareManager] remove:taskId]) {
       NSLog(@"RemoveTask error");     
        return E_SERVICE_ERROR;
    }
    NSLog(@"RemoveTask end");
    return E_OK;
}

int32_t IosTaskDao::QueryTaskInfo(int64_t taskId, const std::string &token, TaskInfo &info)
{
    NSLog(@"QueryTaskInfo, taskId:%lld, token:%s", taskId, token.c_str());
    if (taskId < 1) {
        return E_TASK_NOT_FOUND;
    }
    IosTaskInfo *taskInfo = nil;
    if (!token.empty()) {
        NSString *strToken = JsonUtils::CStringToNSString(token);
        taskInfo = [[DBManager shareManager] queryWithToken:strToken taskId:taskId];
    } else {
        taskInfo = [[DBManager shareManager] queryWithTaskId:taskId];
    }
    if (taskInfo == nil) {
        NSLog(@"failed to query task info");
        return E_TASK_NOT_FOUND;
    }
    info.tid = std::to_string(taskInfo.tid);
    info.url = taskInfo.url.UTF8String;
    info.data = taskInfo.data.UTF8String;
    info.title = taskInfo.title.UTF8String;
    info.description = taskInfo.desc.UTF8String;
    info.action = static_cast<Action>(taskInfo.action);
    info.mode = static_cast<Mode>(taskInfo.mode);
    info.mimeType = taskInfo.mimeType.UTF8String;
    info.ctime = taskInfo.ctime;
    info.mtime = taskInfo.mtime;
    info.faults = static_cast<Faults>(taskInfo.faults);
    info.reason = taskInfo.reason.UTF8String;
    JsonUtils::JsonStringToProgress(taskInfo.progress.UTF8String, info.progress);
    info.version = static_cast<Version>(taskInfo.version);
    JsonUtils::JsonStringToFiles(taskInfo.files.UTF8String, info.files);
    JsonUtils::JsonStringToForms(taskInfo.forms.UTF8String, info.forms);
    info.gauge = taskInfo.gauge;
    info.retry = taskInfo.retry;
    info.code = static_cast<Reason>(taskInfo.code);
    info.withSystem = taskInfo.withSystem;
    JsonUtils::JsonStringToExtras(taskInfo.extras.UTF8String, info.extras);
    JsonUtils::JsonStringToTaskStates(taskInfo.taskStates.UTF8String, info.taskStates);
    NSLog(@"QueryTaskInfo end");
    return E_OK;
}

int32_t IosTaskDao::Search(const Filter &filter, std::vector<std::string> &taskIdList)
{
    NSLog(@"Search enter");
    IosTaskFilter *taskFilter = [[IosTaskFilter alloc] init];
    taskFilter.before = filter.before;
    taskFilter.after = filter.after;
    taskFilter.state = static_cast<int>(filter.state);
    taskFilter.action = static_cast<int>(filter.action);
    taskFilter.mode = static_cast<int>(filter.mode);
    NSArray *taskIds = [[DBManager shareManager] queryWithFilter:taskFilter];
    for (id taskId in taskIds) {
        std::string strTaskId = [taskId stringValue].UTF8String;
        taskIdList.emplace_back(strTaskId);
    }
    return E_OK;
}

int32_t IosTaskDao::UpdateDB(const TaskInfo &info)
{
    NSLog(@"UpdateDB enter");
    @autoreleasepool {
        IosTaskInfo *taskInfo = [[IosTaskInfo alloc] init];
        taskInfo.tid = std::stoll(info.tid);
        taskInfo.url = JsonUtils::CStringToNSString(info.url);
        taskInfo.data = JsonUtils::CStringToNSString(info.data);
        taskInfo.title = JsonUtils::CStringToNSString(info.title);
        taskInfo.desc = JsonUtils::CStringToNSString(info.description);
        taskInfo.action = static_cast<int>(info.action);
        taskInfo.mode = static_cast<int>(info.mode);
        taskInfo.mimeType = JsonUtils::CStringToNSString(info.mimeType);
        taskInfo.state = 0;
        taskInfo.progress = JsonUtils::CStringToNSString(JsonUtils::ProgressToJsonString(info.progress).c_str());
        taskInfo.ctime = info.ctime;
        taskInfo.mtime = info.mtime;
        taskInfo.faults = static_cast<int>(info.faults);
        taskInfo.reason = JsonUtils::CStringToNSString(info.reason);
        taskInfo.taskStates = JsonUtils::CStringToNSString(JsonUtils::TaskStatesToJsonStirng(info.taskStates).c_str());
        taskInfo.downloadId = 0;
        taskInfo.version = static_cast<int>(info.version);
        taskInfo.files = JsonUtils::CStringToNSString(JsonUtils::FilesToJsonStirng(info.files).c_str());
        taskInfo.code = info.code;
        if (![[DBManager shareManager] update:taskInfo]) {
            NSLog(@"failed to UpdateDB");
            return E_SERVICE_ERROR;
        }
    }
    NSLog(@"UpdateDB end");
    return E_OK;
}

}
