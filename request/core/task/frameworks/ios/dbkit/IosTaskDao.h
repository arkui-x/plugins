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
#ifndef PLUGINS_REQUEST_IOS_TASKADAPTER_H
#define PLUGINS_REQUEST_IOS_TASKADAPTER_H

#import <Foundation/Foundation.h>
#include <string>
#include "constant.h"

namespace OHOS::Plugin::Request {
class IosTaskDao {
public:
    IosTaskDao() = default;
    virtual ~IosTaskDao();

    static int64_t CreateTask(const Config &config);
    static int32_t RemoveTask(int64_t taskId);
    static int32_t QueryTaskInfo(int64_t taskId, const std::string &token, TaskInfo &info);
    static int32_t Search(const Filter &filter, std::vector<std::string> &taskIdList);
    static int32_t UpdateDB(const TaskInfo &info);
};
} // namespace OHOS::Plugin::Request
#endif // PLUGINS_REQUEST_IOS_TASKADAPTER_H
