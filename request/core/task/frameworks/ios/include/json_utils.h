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

#ifndef PLUGINS_REQUEST_IOS_JSON_UTILS_H
#define PLUGINS_REQUEST_IOS_JSON_UTILS_H

#import <Foundation/Foundation.h>
#include "constant.h"

namespace OHOS::Plugin::Request {
class JsonUtils {
public:
    JsonUtils() = default;
    virtual ~JsonUtils() = default;

    static NSString *CStringToNSString(const std::string &str);
    static std::string TaskInfoToJsonString(const TaskInfo &info);
    static std::string ConfigToJsonString(const Config &config);
    static std::string ProgressToJsonString(const Progress &progress);
    static std::string TaskStatesToJsonStirng(const std::vector<TaskState> &taskStates);
    static std::string FilesToJsonStirng(const std::vector<FileSpec> &files);

    static void JsonStringToProgress(const std::string &jsonProgress, Progress &progress);
    static void JsonStringToFiles(const std::string &jsonFiles, std::vector<FileSpec> &files);
    static void JsonStringToForms(const std::string &jsonForms, std::vector<FormItem> &forms);
    static void JsonStringToExtras(const std::string &jsonExtras, std::map<std::string, std::string> &extras);
    static void JsonStringToTaskStates(const std::string &jsonTaskStates, std::vector<TaskState> &taskStates);
};
} // namespace OHOS::Plugin::Request
#endif // PLUGINS_REQUEST_IOS_JSON_UTILS_H