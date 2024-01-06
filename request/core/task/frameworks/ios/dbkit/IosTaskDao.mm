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
    REQUEST_HILOGI("CreateTask enter");

    if (![[DBManager shareManager] initDb]) {
        REQUEST_HILOGE("Failed to init database");
        return INVALID_TASK_ID;
    }

    std::string strConfig = JsonUtils::ConfigToJsonString(config);
    NSString *jsonConfig = [NSString stringWithUTF8String:strConfig.c_str()];
    REQUEST_HILOGI("jsonConfig:%{public}s", strConfig.c_str());
    IosTaskConfig *taskConfig = [IosTaskConfig initWithJsonString:jsonConfig];
    if (taskConfig == nil) {
        REQUEST_HILOGI("Failed to create task, config is nil");
        return INVALID_TASK_ID;
    }

    return [[DBManager shareManager] insert:[taskConfig getTaskInfo]];
}

int32_t IosTaskDao::RemoveTask(int64_t taskId)
{
    REQUEST_HILOGE("RemoveTask start taskid: %{public}lld", taskId);
    if (![[DBManager shareManager] remove:taskId]) {
       REQUEST_HILOGE("RemoveTask error");     
        return E_SERVICE_ERROR;
    }
    REQUEST_HILOGE("RemoveTask end");
    return E_OK;
}

int32_t IosTaskDao::QueryTaskInfo(int64_t taskId, const std::string &token, TaskInfo &info)
{
    REQUEST_HILOGI("QueryTaskInfo, taskId:%{public}lld, token:%{public}s", taskId, token.c_str());
    if (taskId < 1) {
        return E_SERVICE_ERROR;
    }
    IosTaskInfo *taskInfo = nil;
    if (!token.empty()) {
        NSString *strToken = [NSString stringWithUTF8String:token.c_str()];
        taskInfo = [[DBManager shareManager] queryWithToken:strToken taskId:taskId];
    } else {
        taskInfo = [[DBManager shareManager] queryWithTaskId:taskId];
    }
    if (taskInfo != nil) {
        info.tid = std::to_string(taskInfo.tid);
        info.url = [taskInfo.url UTF8String];
        info.data = [taskInfo.data UTF8String];
        info.title = [taskInfo.title UTF8String];
        info.description = [taskInfo.desc UTF8String];
        info.action = static_cast<Action>(taskInfo.action);
        info.mode = static_cast<Mode>(taskInfo.mode);
        info.mimeType = [taskInfo.mimeType UTF8String];
        info.ctime = taskInfo.ctime;
        info.mtime = taskInfo.mtime;
        info.faults = static_cast<Faults>(taskInfo.faults);
        info.reason = [taskInfo.reason UTF8String];
        // info.taskStates = [taskInfo.taskState UTF8String];
        JsonUtils::JsonStringToProgress([taskInfo.progress UTF8String], info.progress);
        return E_OK;
    }

    REQUEST_HILOGE("failed to query task info");
    return E_SERVICE_ERROR;
}

int32_t IosTaskDao::Search(const Filter &filter, std::vector<std::string> &taskIdList)
{
    REQUEST_HILOGI("IosTaskDao::Search enter");
    IosTaskFilter *taskFilter = [[IosTaskFilter alloc] init];
    taskFilter.before = filter.before;
    taskFilter.after = filter.after;
    taskFilter.state = static_cast<int>(filter.state);
    taskFilter.action = static_cast<int>(filter.action);
    taskFilter.mode = static_cast<int>(filter.mode);
    NSArray *taskIds = [[DBManager shareManager] queryWithFilter:taskFilter];
    for (id taskId in taskIds) {
        std::string strTaskId = [[taskId stringValue] UTF8String];
        REQUEST_HILOGI("queryWithFilter, strTaskId:%{public}s", strTaskId.c_str());
        taskIdList.emplace_back(strTaskId);
    }
    return E_OK;
}

int32_t IosTaskDao::UpdateDB(const TaskInfo &info, const Config &config)
{
    REQUEST_HILOGI("IosTaskDao::UpdateDB enter");
    IosTaskInfo *taskInfo = [[IosTaskInfo alloc] init];
    taskInfo.tid = std::stoll(info.tid);
    taskInfo.saveas = [NSString stringWithUTF8String:config.saveas.c_str()];
    taskInfo.url = [NSString stringWithUTF8String:info.url.c_str()];
    taskInfo.data = [NSString stringWithUTF8String:info.data.c_str()];
    taskInfo.title = [NSString stringWithUTF8String:info.title.c_str()];
    taskInfo.desc = [NSString stringWithUTF8String:info.description.c_str()];
    taskInfo.action = static_cast<int>(config.action);
    taskInfo.mode = static_cast<int>(config.mode);
    taskInfo.mimeType = [NSString stringWithUTF8String:info.mimeType.c_str()];
    taskInfo.state = 0;
    taskInfo.progress = [NSString stringWithUTF8String:JsonUtils::ProgressToJsonString(info.progress).c_str()];
    taskInfo.ctime = info.ctime;
    taskInfo.mtime = info.mtime;
    taskInfo.faults = static_cast<int>(info.faults);
    taskInfo.reason = [NSString stringWithUTF8String:info.reason.c_str()];
    taskInfo.taskStates = @"";
    taskInfo.token = [NSString stringWithUTF8String:config.token.c_str()];

    if (![[DBManager shareManager] update:taskInfo]) {
        REQUEST_HILOGE("failed to UpdateDB");
        return E_SERVICE_ERROR;
    }
    return E_OK;
}

}
