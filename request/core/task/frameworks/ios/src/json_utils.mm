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

#include "json_utils.h"

namespace OHOS::Plugin::Request {
using namespace std;

NSString *JsonUtils::CStringToNSString(const string &str)
{
    if (str.empty() || str.c_str() == nullptr) {
        return @"";
    }
    return [NSString stringWithUTF8String:str.c_str()];
}

string JsonUtils::TaskInfoToJsonString(const TaskInfo &info)
{
    @autoreleasepool {
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        [dict setValue:[NSNumber numberWithInteger:(int)info.version] forKey:@"version"];
        [dict setValue:CStringToNSString(info.url) forKey:@"url"];
        [dict setValue:CStringToNSString(info.data) forKey:@"data"];
        // files
        NSMutableArray *filesArr = [NSMutableArray array];
        if (!info.files.empty()) {
            for (auto &file : info.files) {
                NSMutableDictionary *dictFile = [NSMutableDictionary dictionary];
                [dictFile setValue:CStringToNSString(file.name) forKey:@"name"];
                [dictFile setValue:CStringToNSString(file.uri) forKey:@"uri"];
                [dictFile setValue:CStringToNSString(file.filename) forKey:@"filename"];
                [dictFile setValue:CStringToNSString(file.type) forKey:@"type"];
                [dictFile setValue:[NSNumber numberWithInteger:file.fd] forKey:@"fd"];
                [filesArr addObject:dictFile];
            }
        }
        [dict setObject:filesArr forKey:@"files"];
        // forms
        NSMutableArray *formsArr = [NSMutableArray array];
        for (auto &form : info.forms) {
            NSMutableDictionary *dictForm = [NSMutableDictionary dictionary];
            [dictForm setValue:CStringToNSString(form.name) forKey:@"name"];
            [dictForm setValue:CStringToNSString(form.value) forKey:@"value"];
            [formsArr addObject:dictForm];
        }
        [dict setObject:formsArr forKey:@"forms"];
        [dict setValue:CStringToNSString(info.tid) forKey:@"tid"];
        [dict setValue:CStringToNSString(info.title) forKey:@"title"];
        [dict setValue:CStringToNSString(info.description) forKey:@"desc"];
        [dict setValue:[NSNumber numberWithInteger:(int)info.action] forKey:@"action"];
        [dict setValue:[NSNumber numberWithInteger:(int)info.mode] forKey:@"mode"];
        [dict setValue:CStringToNSString(info.mimeType) forKey:@"mimeType"];
        // progress
        NSMutableDictionary *dictProgress = [NSMutableDictionary dictionary];
        [dictProgress setValue:[NSNumber numberWithInteger:(int)info.progress.state] forKey:@"state"];
        [dictProgress setValue:[NSNumber numberWithInteger:(int)info.progress.index] forKey:@"index"];
        [dictProgress setValue:[NSNumber numberWithInteger:(int)info.progress.processed] forKey:@"processed"];
        [dictProgress setValue:[NSNumber numberWithInteger:(int)info.progress.lastProcessed] forKey:@"lastProcessed"];
        [dictProgress setValue:[NSNumber numberWithInteger:(int)info.progress.totalProcessed] forKey:@"totalProcessed"];
        NSMutableArray *sizesArr = [NSMutableArray array];
        for (auto &size : info.progress.sizes) {
            [sizesArr addObject:[NSNumber numberWithInteger:size]];
        }
        [dictProgress setObject:sizesArr forKey:@"sizes"];
        NSMutableDictionary *dictExtras = [NSMutableDictionary dictionary];
        for (auto it = info.progress.extras.begin(); it != info.progress.extras.end(); it++) {
            NSString *key = CStringToNSString(it->first);
            NSString *value = CStringToNSString(it->second);
            [dictExtras setValue:value forKey:key];
        }
        [dictProgress setObject:dictExtras forKey:@"extras"];
        NSMutableArray *bodyBytesArr = [NSMutableArray array];
        for (auto &bodyByte : info.progress.bodyBytes) {
            [bodyBytesArr addObject:[NSNumber numberWithInteger:(int)bodyByte]];
        }
        [dictProgress setObject:bodyBytesArr forKey:@"bodyBytes"];
        [dict setObject:dictProgress forKey:@"progress"];
        // response
        NSMutableDictionary *dictResponse = [NSMutableDictionary dictionary];
            [dictResponse setValue:CStringToNSString(info.response.version) forKey:@"version"];
            [dictResponse setValue:@(info.response.statusCode) forKey:@"statusCode"];
            [dictResponse setValue:CStringToNSString(info.response
                                                     .reason) forKey:@"reason"];
            NSMutableDictionary *dictHeaders = [NSMutableDictionary dictionary];
            for (const auto &header : info.response.headers) {
                NSString *key = CStringToNSString(header.first);
                NSMutableArray *values = [NSMutableArray array];
                for (const auto &val : header.second) {
                    [values addObject:CStringToNSString(val)];
                }
                [dictHeaders setObject:values forKey:key];
            }
        [dictResponse setObject:dictHeaders forKey:@"headers"];
        [dict setObject:dictResponse forKey:@"response"];
        [dict setValue:[NSNumber numberWithInteger:info.gauge] forKey:@"gauge"];
        [dict setValue:[NSNumber numberWithInteger:info.ctime] forKey:@"ctime"];
        [dict setValue:[NSNumber numberWithInteger:info.mtime] forKey:@"mtime"];
        [dict setValue:[NSNumber numberWithInteger:info.retry] forKey:@"retry"];
        [dict setValue:[NSNumber numberWithInteger:info.tries] forKey:@"tries"];
        [dict setValue:[NSNumber numberWithInteger:(int)info.faults] forKey:@"faults"];
        [dict setValue:[NSNumber numberWithInteger:info.code] forKey:@"code"];
        [dict setValue:CStringToNSString(info.reason) forKey:@"reason"];
        [dict setValue:[NSNumber numberWithInteger:info.withSystem] forKey:@"withSystem"];
        NSMutableDictionary *dictInfoExtras = [NSMutableDictionary dictionary];
        for (auto it = info.extras.begin(); it != info.extras.end(); it++) {
            NSString *key = CStringToNSString(it->first);
            NSString *value = CStringToNSString(it->second);
            [dictInfoExtras setValue:value forKey:key];
        }
        [dict setObject:dictInfoExtras forKey:@"extras"];

        NSMutableArray *taskStatesArr = [NSMutableArray array];
        for (auto &state : info.taskStates) {
            NSMutableDictionary *dictState = [NSMutableDictionary dictionary];
            [dictState setValue:CStringToNSString(state.path) forKey:@"path"];
            [dictState setValue:[NSNumber numberWithInteger:state.responseCode] forKey:@"responseCode"];
            [dictState setValue:CStringToNSString(state.message) forKey:@"message"];
            [taskStatesArr addObject:dictState];
        }
        [dict setObject:taskStatesArr forKey:@"taskStates"];

        NSError *error;
        NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dict options:NSJSONWritingPrettyPrinted
            error:&error];
        if (!jsonData) {
            NSLog(@"Failed to translate TaskInfo dictionary to string");
            return "";
        }
        NSString *strInfo = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
        return strInfo.UTF8String;
    }
}

string JsonUtils::ConfigToJsonString(const Config &config)
{
    @autoreleasepool {
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        [dict setValue:[NSNumber numberWithInteger:(int)config.action] forKey: @"action"];
        [dict setValue:CStringToNSString(config.url) forKey: @"url"];
        [dict setValue:[NSNumber numberWithInteger:(int)config.version] forKey: @"version"];
        [dict setValue:[NSNumber numberWithInteger:(int)config.mode] forKey: @"mode"];
        [dict setValue:[NSNumber numberWithInteger:(int)config.network] forKey: @"network"];
        [dict setValue:[NSNumber numberWithInteger:config.index] forKey: @"index"];
        [dict setValue:[NSNumber numberWithInteger:config.begins] forKey: @"begins"];
        [dict setValue:[NSNumber numberWithInteger:config.ends] forKey: @"ends"];
        [dict setValue:[NSNumber numberWithInteger:config.overwrite] forKey: @"overwrite"];
        [dict setValue:[NSNumber numberWithInteger:config.metered] forKey: @"metered"];
        [dict setValue:[NSNumber numberWithInteger:config.roaming] forKey: @"roaming"];
        [dict setValue:[NSNumber numberWithInteger:config.retry] forKey: @"retry"];
        [dict setValue:[NSNumber numberWithInteger:config.redirect] forKey: @"redirect"];
        [dict setValue:[NSNumber numberWithInteger:config.gauge] forKey: @"gauge"];
        [dict setValue:[NSNumber numberWithInteger:config.precise] forKey: @"precise"];
        [dict setValue:[NSNumber numberWithInteger:config.background] forKey: @"background"];
        [dict setValue:CStringToNSString(config.title) forKey: @"title"];
        [dict setValue:CStringToNSString(config.saveas) forKey: @"saveas"];
        [dict setValue:CStringToNSString(config.method) forKey: @"method"];
        [dict setValue:CStringToNSString(config.token) forKey: @"token"];
        [dict setValue:CStringToNSString(config.description) forKey: @"desc"];
        [dict setValue:CStringToNSString(config.data) forKey: @"data"];
        // headers
        NSMutableDictionary *dictHeaders = [NSMutableDictionary dictionary];
        for (auto it = config.headers.begin(); it != config.headers.end(); it++) {
            NSString *key = CStringToNSString(it->first);
            NSString *value = CStringToNSString(it->second);
            [dictHeaders setValue:value forKey:key];
        }
        [dict setObject:dictHeaders forKey:@"headers"];
        // forms
        NSMutableArray *arrForms = [NSMutableArray array];
        for (auto &form : config.forms) {
            NSDictionary *dictFrom = @{
                @"name":CStringToNSString(form.name),
                @"value":CStringToNSString(form.value)
            };
            [arrForms addObject:dictFrom];
        }
        [dict setObject:arrForms forKey:@"forms"];
        // files
        NSMutableArray *arrFiles = [NSMutableArray array];
        for (auto &file : config.files) {
            NSDictionary *dictFile = @{
                @"name":CStringToNSString(file.name),
                @"uri":CStringToNSString(file.uri),
                @"filename":CStringToNSString(file.filename),
                @"type":CStringToNSString(file.type),
                @"fd":[NSNumber numberWithInteger:file.fd]
            };
            [arrFiles addObject:dictFile];
        }
        [dict setObject:arrFiles forKey:@"files"];
        // bodyFds
        NSMutableArray *arrBodyFds = [NSMutableArray array];
        for (auto &fd : config.bodyFds) {
            [arrBodyFds addObject:[NSNumber numberWithInteger:fd]];
        }
        [dict setObject:arrBodyFds forKey:@"bodyFds"];
        // bodyFileNames
        NSMutableArray *arrBodyFileNames = [NSMutableArray array];
        for (auto &name : config.bodyFileNames) {
            [arrBodyFileNames addObject:CStringToNSString(name)];
        }
        [dict setObject:arrBodyFileNames forKey:@"bodyFileNames"];
        // extras
        NSMutableDictionary *dictExtras = [NSMutableDictionary dictionary];
        for (auto it = config.extras.begin(); it != config.extras.end(); it++) {
            NSString *key = CStringToNSString(it->first);
            NSString *value = CStringToNSString(it->second);
            [dictExtras setValue:value forKey:key];
        }
        [dict setObject:dictExtras forKey:@"extras"];
        
        NSError *error;
        NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dict options:NSJSONWritingPrettyPrinted
                                                             error:&error];
        if (!jsonData) {
            NSLog(@"Failed to translate Config dictionary to string");
            return "";
        }
        NSString *strConfig = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
        return strConfig.UTF8String;
    }
}

string JsonUtils::ProgressToJsonString(const Progress &progress)
{
    @autoreleasepool {
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        [dict setValue:[NSNumber numberWithInt:(int)progress.state] forKey:@"state"];
        [dict setValue:[NSNumber numberWithInt:progress.index] forKey:@"index"];
        [dict setValue:[NSNumber numberWithInteger:progress.processed] forKey:@"processed"];
        [dict setValue:[NSNumber numberWithInteger:progress.totalProcessed] forKey:@"totalProcessed"];
        NSMutableArray *arrSizes = [NSMutableArray array];
        for (auto &size : progress.sizes) {
            [arrSizes addObject:[NSNumber numberWithInteger:size]];
        }
        [dict setObject:arrSizes forKey:@"sizes"];
        NSMutableDictionary *dictExtras = [NSMutableDictionary dictionary];
        for (auto it = progress.extras.begin(); it != progress.extras.end(); ++it) {
            NSString *key = CStringToNSString(it->first);
            NSString *value = CStringToNSString(it->second);
            [dictExtras setValue:value forKey:key];
        }
        [dict setObject:dictExtras forKey:@"extras"];
        
        NSMutableArray *arrBodyBytes = [NSMutableArray array];
        for (auto &bodyByte : progress.bodyBytes) {
            [arrBodyBytes addObject:[NSNumber numberWithInteger:bodyByte]];
        }
        [dict setObject:arrBodyBytes forKey:@"bodyBytes"];
        
        NSError *error;
        NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dict options:NSJSONWritingPrettyPrinted
                                                             error:&error];
        if (!jsonData) {
            NSLog(@"Failed to translate progress dictionary to string");
            return "";
        }
        NSString *strProgress = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
        return strProgress.UTF8String;
    }
}

string JsonUtils::TaskStatesToJsonStirng(const std::vector<TaskState> &taskStates)
{
    @autoreleasepool {
        NSMutableArray *array = [NSMutableArray array];
        for (auto &state : taskStates) {
            NSMutableDictionary *dict = [NSMutableDictionary dictionary];
            [dict setValue:CStringToNSString(state.path) forKey:@"path"];
            [dict setValue:[NSNumber numberWithInt:state.responseCode] forKey:@"responseCode"];
            [dict setValue:CStringToNSString(state.message) forKey:@"message"];
            [array addObject:dict];
        }
        
        NSError *error = nil;
        NSData *jsonData = [NSJSONSerialization dataWithJSONObject:array options:NSJSONWritingPrettyPrinted
                                                             error:&error];
        if (!jsonData) {
            NSLog(@"failed to translate taskStates array to string");
            return "";
        }
        NSString *strTaskState = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
        return strTaskState.UTF8String;
    }
}

std::string JsonUtils::FilesToJsonStirng(const std::vector<FileSpec> &files)
{
    @autoreleasepool {
        NSMutableArray *array = [NSMutableArray array];
        for (auto &file : files) {
            NSMutableDictionary *dict = [NSMutableDictionary dictionary];
            [dict setValue:CStringToNSString(file.filename) forKey:@"filename"];
            [dict setValue:CStringToNSString(file.name) forKey:@"name"];
            [dict setValue:CStringToNSString(file.uri) forKey:@"uri"];
            [dict setValue:CStringToNSString(file.type) forKey:@"type"];
            [dict setValue:[NSNumber numberWithInt:file.fd] forKey:@"fd"];
            [array addObject:dict];
        }
        
        NSError *error = nil;
        NSData *jsonData = [NSJSONSerialization dataWithJSONObject:array options:NSJSONWritingPrettyPrinted
                                                             error:&error];
        if (!jsonData) {
            NSLog(@"failed to translate taskStates array to string");
            return "";
        }
        NSString *strFiles = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
        return strFiles.UTF8String;
    }
}

void JsonUtils::JsonStringToProgress(const string &jsonProgress, Progress &progress)
{
    @autoreleasepool {
        NSString *jsonString = CStringToNSString(jsonProgress);
        NSData *jsonData = [jsonString dataUsingEncoding:NSUTF8StringEncoding];
        NSError *error;
        NSDictionary *dict = [NSJSONSerialization JSONObjectWithData:jsonData options:NSJSONReadingMutableContainers
                                                               error:&error];
        if (dict == nil) {
            NSLog(@"failed to get progress dict");
            return;
        }
        progress.state = static_cast<State>([dict[@"state"] intValue]);
        progress.index = [dict[@"index"] intValue];
        progress.processed = [dict[@"processed"] longLongValue];
        progress.totalProcessed = [dict[@"totalProcessed"] longLongValue];
        NSArray *sizesArr = dict[@"sizes"];
        if (sizesArr != nil) {
            for (id size : sizesArr) {
                progress.sizes.emplace_back([size longLongValue]);
            }
        }
        NSDictionary *dictExtras = dict[@"extras"];
        if (dictExtras != nil) {
            for (id key in dictExtras) {
                string value = [dictExtras[key] UTF8String];
                progress.extras.emplace(make_pair([key UTF8String], value));
            }
        }
        NSArray *bodyBytesArr = dict[@"bodyBytes"];
        if (bodyBytesArr != nil) {
            for (id item : bodyBytesArr) {
                progress.bodyBytes.emplace_back([item intValue]);
            }
        }
    }
}

void JsonUtils::JsonStringToFiles(const string &jsonFiles, vector<FileSpec> &files)
{
    @autoreleasepool {
        NSString *jsonString = CStringToNSString(jsonFiles);
        NSData *jsonData = [jsonString dataUsingEncoding:NSUTF8StringEncoding];
        NSError *error;
        NSArray *array = [NSJSONSerialization JSONObjectWithData:jsonData options:NSJSONReadingMutableContainers
                                                           error:&error];
        if (array == nil) {
            NSLog(@"failed to get files array");
            return;
        }
        for (id dictFile : array) {
            FileSpec file;
            file.name = [dictFile[@"name"] UTF8String];
            file.uri = [dictFile[@"uri"] UTF8String];
            file.filename = [dictFile[@"filename"] UTF8String];
            file.type = [dictFile[@"type"] UTF8String];
            file.fd = [dictFile[@"fd"] intValue];
            files.emplace_back(file);
        }
    }
}

void JsonUtils::JsonStringToForms(const string &jsonForms, vector<FormItem> &forms)
{
    @autoreleasepool {
        NSString *jsonString = CStringToNSString(jsonForms);
        NSData *jsonData = [jsonString dataUsingEncoding:NSUTF8StringEncoding];
        NSError *error;
        NSArray *array = [NSJSONSerialization JSONObjectWithData:jsonData options:NSJSONReadingMutableContainers
                                                           error:&error];
        if (array == nil) {
            NSLog(@"failed to get forms array");
            return;
        }
        for (id dictForm : array) {
            FormItem form;
            form.name = [dictForm[@"name"] UTF8String];
            form.value = [dictForm[@"value"] UTF8String];
            forms.emplace_back(form);
        }
    }
}

void JsonUtils::JsonStringToExtras(const string &jsonExtras, map<string, string> &extras)
{
    @autoreleasepool {
        NSString *jsonString = CStringToNSString(jsonExtras);
        NSData *jsonData = [jsonString dataUsingEncoding:NSUTF8StringEncoding];
        NSError *error;
        NSDictionary *dict = [NSJSONSerialization JSONObjectWithData:jsonData options:NSJSONReadingMutableContainers
                                                               error:&error];
        if (dict == nil) {
            NSLog(@"failed to get extras dictionary");
            return;
        }
        for (id key in dict) {
            string value = [dict[key] UTF8String];
            extras.emplace(make_pair([key UTF8String], value));
        }
    }
}

void JsonUtils::JsonStringToTaskStates(const string &jsonTaskStates, vector<TaskState> &taskStates)
{
    @autoreleasepool {
        NSString *jsonString = CStringToNSString(jsonTaskStates);
        NSData *jsonData = [jsonString dataUsingEncoding:NSUTF8StringEncoding];
        NSError *error;
        NSArray *array = [NSJSONSerialization JSONObjectWithData:jsonData options:NSJSONReadingMutableContainers
                                                           error:&error];
        if (array == nil) {
            NSLog(@"failed to get taskStates array");
            return;
        }
        for (id dictTaskState : array) {
            TaskState taskState;
            taskState.path = [dictTaskState[@"path"] UTF8String];
            taskState.responseCode = [dictTaskState[@"responseCode"] intValue];
            taskState.message = [dictTaskState[@"message"] UTF8String];
            taskStates.emplace_back(taskState);
        }
    }
}

}
