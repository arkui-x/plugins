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

#include "document_picker_impl.h"
#include "utils/log.h"
#import "ios_document_picker.h"

namespace OHOS::Plugin {

napi_value DocumentFilePickerImpl::select(napi_env env, DocumentSelectOptions& options) {
    HILOG_INFO("DocumentFilePickerImpl::select enter");
    DocumentFilePicker::napienv = env;
    napi_value promise = nullptr;
    DocumentFilePicker::deferred = nullptr;
    napi_create_promise(env, &DocumentFilePicker::deferred, &promise);
    
    NSMutableArray *types = [NSMutableArray array]; 
    for(int i = 0; i < options.fileSuffixFilters.size(); i++) {
        std::string str = options.fileSuffixFilters[i];
        NSString *ocString = [NSString stringWithCString:str.c_str() encoding:NSUTF8StringEncoding];
        if (ocString != nil && ocString.length > 0) {
            [types addObject:ocString];
        }
    }
    ios_document_select_options *iOSOptions = [ios_document_select_options new];
    iOSOptions.maxSelectNumber = options.maxSelectNumber;
    iOSOptions.fileSuffixFilters = types;
    [[ios_document_picker shareManager] selectOptions:iOSOptions callBack:^(NSArray<NSString *> * _Nonnull results, int errCode) {
        std::vector<std::string> result;
        for (int i = 0; i < results.count; ++i) {
            NSString *path = results[i];
            if (path.length > 0) {
                std::string cppStrPath = [path UTF8String];
                result.push_back(cppStrPath);
            }
        }
        DocumentFilePicker::onPickerResult(result, errCode);
    }];
    return promise;
}

napi_value DocumentFilePickerImpl::save(napi_env env,DocumentSaveOptions& options){
    return nullptr;
}

} // namespace OHOS::Plugin
