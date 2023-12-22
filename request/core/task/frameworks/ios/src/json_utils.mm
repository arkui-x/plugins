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
#include "log.h"

namespace OHOS::Plugin::Request {
using namespace std;
string JsonUtils::TaskInfoToJsonString(const TaskInfo &info)
{
    NSMutableDictionary * dict = [NSMutableDictionary dictionary];
    [dict setValue:[NSNumber numberWithInteger:(int)info.version] forKey:@"version"];
    [dict setValue:[NSString stringWithUTF8String:info.url.c_str()] forKey:@"url"];
    [dict setValue:[NSString stringWithUTF8String:info.data.c_str()] forKey:@"data"];
    // files
    NSMutableArray *filesArr = [NSMutableArray array];
    for (auto &file : info.files) {
        NSMutableDictionary *dictFile = [NSMutableDictionary dictionary];
        [dictFile setValue:[NSString stringWithUTF8String:file.name.c_str()] forKey:@"name"];
        [dictFile setValue:[NSString stringWithUTF8String:file.uri.c_str()] forKey:@"uri"];
        [dictFile setValue:[NSString stringWithUTF8String:file.filename.c_str()] forKey:@"filename"];
        [dictFile setValue:[NSString stringWithUTF8String:file.type.c_str()] forKey:@"type"];
        [dictFile setValue:[NSNumber numberWithInteger:file.fd] forKey:@"fd"];
        [filesArr addObject:dictFile];
    }
    [dict setObject:filesArr forKey:@"files"];
    // forms
    NSMutableArray *formsArr = [NSMutableArray array];
    for (auto &form : info.forms) {
        NSMutableDictionary *dictForm = [NSMutableDictionary dictionary];
        [dictForm setValue:[NSString stringWithUTF8String:form.name.c_str()] forKey:@"name"];
        [dictForm setValue:[NSString stringWithUTF8String:form.value.c_str()] forKey:@"value"];
        [formsArr addObject:dictForm];
    }
    [dict setObject:formsArr forKey:@"forms"];
    [dict setValue:[NSString stringWithUTF8String:info.tid.c_str()] forKey:@"tid"];
    [dict setValue:[NSString stringWithUTF8String:info.title.c_str()] forKey:@"title"];
    [dict setValue:[NSString stringWithUTF8String:info.description.c_str()] forKey:@"desc"];
    [dict setValue:[NSNumber numberWithInteger:(int)info.action] forKey:@"action"];
    [dict setValue:[NSNumber numberWithInteger:(int)info.mode] forKey:@"mode"];
    [dict setValue:[NSString stringWithUTF8String:info.mimeType.c_str()] forKey:@"mimeType"];
    // progress
    NSMutableDictionary *dictProgress = [NSMutableDictionary dictionary];
    [dictProgress setValue:[NSNumber numberWithInteger:(int)info.progress.state] forKey:@"state"];
    [dictProgress setValue:[NSNumber numberWithInteger:(int)info.progress.index] forKey:@"index"];
    [dictProgress setValue:[NSNumber numberWithInteger:(int)info.progress.processed] forKey:@"processed"];
    [dictProgress setValue:[NSNumber numberWithInteger:(int)info.progress.totalProcessed] forKey:@"totalProcessed"];
    NSMutableArray *sizesArr = [NSMutableArray array];
    for (auto &size : info.progress.sizes) {
        [sizesArr addObject:[NSNumber numberWithInteger:size]];
    }
    [dictProgress setObject:sizesArr forKey:@"sizes"];
    NSMutableDictionary *dictExtras = [NSMutableDictionary dictionary];
    for (auto it = info.progress.extras.begin(); it != info.progress.extras.end(); it++) {
        NSString *key = [NSString stringWithUTF8String:it->first.c_str()];
        NSString *value = [NSString stringWithUTF8String:it->second.c_str()];
        [dictExtras setValue:value forKey:key];
    }
    [dictProgress setObject:dictExtras forKey:@"extras"];
    NSMutableArray *bodyBytesArr = [NSMutableArray array];
    for (auto &bodyByte : info.progress.bodyBytes) {
        [bodyBytesArr addObject:[NSNumber numberWithInteger:(int)bodyByte]];
    }
    [dictProgress setObject:bodyBytesArr forKey:@"bodyBytes"];
    [dict setObject:dictProgress forKey:@"progress"];
    [dict setValue:[NSNumber numberWithInteger:info.gauge] forKey:@"gauge"];
    [dict setValue:[NSNumber numberWithInteger:info.ctime] forKey:@"ctime"];
    [dict setValue:[NSNumber numberWithInteger:info.mtime] forKey:@"mtime"];
    [dict setValue:[NSNumber numberWithInteger:info.retry] forKey:@"retry"];
    [dict setValue:[NSNumber numberWithInteger:info.tries] forKey:@"tries"];
    [dict setValue:[NSNumber numberWithInteger:(int)info.faults] forKey:@"faults"];
    [dict setValue:[NSNumber numberWithInteger:info.code] forKey:@"code"];
    [dict setValue:[NSString stringWithUTF8String:info.reason.c_str()] forKey:@"reason"];
    [dict setValue:[NSNumber numberWithInteger:info.withSystem] forKey:@"withSystem"];
    [dict setValue:[NSNumber numberWithInteger:info.priority] forKey:@"priority"];
    NSMutableDictionary *dictInfoExtras = [NSMutableDictionary dictionary];
    for (auto it = info.extras.begin(); it != info.extras.end(); it++) {
        NSString *key = [NSString stringWithUTF8String:it->first.c_str()];
        NSString *value = [NSString stringWithUTF8String:it->second.c_str()];
        [dictInfoExtras setValue:value forKey:key];
    }
    [dict setObject:dictInfoExtras forKey:@"extras"];

    NSMutableArray *taskStatesArr = [NSMutableArray array];
    for (auto &state : info.taskStates) {
        NSMutableDictionary *dictState = [NSMutableDictionary dictionary];
        [dictState setValue:[NSString stringWithUTF8String:state.path.c_str()] forKey:@"path"];
        [dictState setValue:[NSNumber numberWithInteger:state.responseCode] forKey:@"responseCode"];
        [dictState setValue:[NSString stringWithUTF8String:state.message.c_str()] forKey:@"message"];
        [taskStatesArr addObject:dictState];
    }
    [dict setObject:taskStatesArr forKey:@"taskStates"];

    NSError *error;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dict options:NSJSONWritingPrettyPrinted
        error:&error];
    if (!jsonData) {
        REQUEST_HILOGE("Failed to translate TaskInfo dictionary to string");
        return "";
    }
    NSString *strInfo = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
    return [strInfo UTF8String];
}

string JsonUtils::ConfigToJsonString(const Config &config)
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    [dict setValue:[NSNumber numberWithInteger:(int)config.action] forKey: @"action"];
    [dict setValue:[NSString stringWithUTF8String:config.url.c_str()] forKey: @"url"];
    [dict setValue:[NSNumber numberWithInteger:(int)config.version] forKey: @"version"];
    [dict setValue:[NSNumber numberWithInteger:(int)config.mode] forKey: @"mode"];
    [dict setValue:[NSNumber numberWithInteger:(int)config.network] forKey: @"network"];
    [dict setValue:[NSNumber numberWithInteger:config.index] forKey: @"index"];
    [dict setValue:[NSNumber numberWithInteger:config.begins] forKey: @"begins"];
    [dict setValue:[NSNumber numberWithInteger:config.ends] forKey: @"ends"];
    [dict setValue:[NSNumber numberWithInteger:config.priority] forKey: @"priority"];
    [dict setValue:[NSNumber numberWithInteger:config.overwrite] forKey: @"overwrite"];
    [dict setValue:[NSNumber numberWithInteger:config.metered] forKey: @"metered"];
    [dict setValue:[NSNumber numberWithInteger:config.roaming] forKey: @"roaming"];
    [dict setValue:[NSNumber numberWithInteger:config.retry] forKey: @"retry"];
    [dict setValue:[NSNumber numberWithInteger:config.redirect] forKey: @"redirect"];
    [dict setValue:[NSNumber numberWithInteger:config.gauge] forKey: @"gauge"];
    [dict setValue:[NSNumber numberWithInteger:config.precise] forKey: @"precise"];
    [dict setValue:[NSNumber numberWithInteger:config.background] forKey: @"background"];
    [dict setValue:[NSString stringWithUTF8String:config.title.c_str()] forKey: @"title"];
    [dict setValue:[NSString stringWithUTF8String:config.saveas.c_str()] forKey: @"saveas"];
    [dict setValue:[NSString stringWithUTF8String:config.method.c_str()] forKey: @"method"];
    [dict setValue:[NSString stringWithUTF8String:config.data.c_str()] forKey: @"token"];
    [dict setValue:[NSString stringWithUTF8String:config.description.c_str()] forKey: @"desc"];
    [dict setValue:[NSString stringWithUTF8String:config.data.c_str()] forKey: @"data"];
    // headers
    NSMutableDictionary *dictHeaders = [NSMutableDictionary dictionary];
    for (auto it = config.headers.begin(); it != config.headers.end(); it++) {
        NSString *key = [NSString stringWithUTF8String:it->first.c_str()];
        NSString *value = [NSString stringWithUTF8String:it->second.c_str()];
        [dictHeaders setValue:value forKey:key];
    }
    [dict setObject:dictHeaders forKey:@"headers"];
    // forms
    NSMutableArray *arrForms = [NSMutableArray array];
    for (auto &form : config.forms) {
        NSDictionary *dictFrom = @{
            @"name":[NSString stringWithUTF8String:form.name.c_str()],
            @"value":[NSString stringWithUTF8String:form.value.c_str()]
        };
        [arrForms addObject:dictFrom];
    }
    [dict setObject:arrForms forKey:@"forms"];
    // files
    NSMutableArray *arrFiles = [NSMutableArray array];
    for (auto &file : config.files) {
        NSDictionary *dictFile = @{
            @"name":[NSString stringWithUTF8String:file.name.c_str()],
            @"uri":[NSString stringWithUTF8String:file.uri.c_str()],
            @"filename":[NSString stringWithUTF8String:file.filename.c_str()],
            @"type":[NSString stringWithUTF8String:file.type.c_str()],
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
        [arrBodyFileNames addObject:[NSString stringWithUTF8String:name.c_str()]];
    }
    [dict setObject:arrBodyFileNames forKey:@"bodyFileNames"];
    // extras
    NSMutableDictionary *dictExtras = [NSMutableDictionary dictionary];
    for (auto it = config.extras.begin(); it != config.extras.end(); it++) {
        NSString *key = [NSString stringWithUTF8String:it->first.c_str()];
        NSString *value = [NSString stringWithUTF8String:it->second.c_str()];
        [dictExtras setValue:value forKey:key];
    }
    [dict setObject:dictExtras forKey:@"extras"];

    NSError *error;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dict options:NSJSONWritingPrettyPrinted
        error:&error];
    if (!jsonData) {
        REQUEST_HILOGE("Failed to translate Config dictionary to string");
        return "";
    }
    NSString *strConfig = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
    return [strConfig UTF8String];
}

string JsonUtils::ProgressToJsonString(const Progress &progress)
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    [dict setValue:[NSNumber numberWithInt:(int)progress.state] forKey:@"state"];
    [dict setValue:[NSNumber numberWithInt:progress.index] forKey:@"index"];
    [dict setValue:[NSNumber numberWithInteger:progress.processed] forKey:@"processed"];
    [dict setValue:[NSNumber numberWithInteger:progress.totalProcessed] forKey:@"totalProcessed"];
    NSMutableArray *arrSizes = [NSMutableArray array];
    for (auto &size : progress.sizes) {
        [arrSizes addObject:[NSNumber numberWithInteger:size]];
    }
    NSMutableDictionary *dictExtras = [NSMutableDictionary dictionary];
    for (auto it = progress.extras.begin(); it != progress.extras.end(); ++it) {
        NSString *key = [NSString stringWithUTF8String:it->first.c_str()];
        NSString *value = [NSString stringWithUTF8String:it->second.c_str()];
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
        REQUEST_HILOGE("Failed to translate progress dictionary to string");
        return "";
    }
    NSString *strProgress = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
    return [strProgress UTF8String];
}

string JsonUtils::TaskStateToJsonStirng(const TaskState &state)
{
    NSMutableDictionary *dict = [NSMutableDictionary dictionary];
    [dict setValue:[NSString stringWithUTF8String:state.path.c_str()] forKey:@"path"];
    [dict setValue:[NSNumber numberWithInt:state.responseCode] forKey:@"responseCode"];
    [dict setValue:[NSString stringWithUTF8String:state.message.c_str()] forKey:@"message"];

    NSError *error;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:dict options:NSJSONWritingPrettyPrinted
        error:&error];
    if (!jsonData) {
        REQUEST_HILOGE("Failed to translate progress dictionary to string");
        return "";
    }
    NSString *strTaskState = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
    return [strTaskState UTF8String];
}

void JsonUtils::JsonStringToProgress(const string &jsonProgress, Progress &progress)
{
    NSString * jsonString = [NSString stringWithUTF8String:jsonProgress.c_str()];
    NSData *jsonData = [jsonString dataUsingEncoding:NSUTF8StringEncoding];
    NSError *error;
    NSDictionary *dict = [NSJSONSerialization JSONObjectWithData:jsonData options:NSJSONReadingMutableContainers
        error:&error];
    if (dict == nil) {
        REQUEST_HILOGE("failed to get progress dict");
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
        for(id item in dictExtras) {
            string key = [item UTF8String];
            string value = [dictExtras[item] UTF8String];
            progress.extras.emplace(make_pair(key, value));
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
