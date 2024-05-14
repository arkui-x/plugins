/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "photo_client_impl.h"
#import "photoPluginManager.h"
#include "log.h"
#include "plugins/file/photo_access_helper/napi/include/photo_picker_callback.h"

#define RESULT_OK 0

using namespace std;

namespace OHOS {
namespace Media {
std::shared_ptr<PhotoClient> PhotoClient::GetInstance()
{
    return std::make_shared<PhotoClientImpl>();
}

void PhotoClientImpl::startPhotoPicker(std::string &type) {
    NSString *ocType = [NSString stringWithCString:type.c_str() encoding:[NSString defaultCStringEncoding]];
    [[photoPluginManager shareManager] startPhotoPickerIosWithType:ocType callBack:^(NSArray<NSString *> * _Nonnull results, int errorCode) {
        std::vector<std::string> uriNow(0);
        for(int i = 0; i<results.count; i++) {
            NSString *ocString = [results objectAtIndex:i];
            std::string cStringNow = [ocString UTF8String];
            uriNow.emplace_back(cStringNow);
        }
        std::shared_ptr<OHOS::Media::PickerCallBack> photoPickerCallback = OHOS::Media::PhotoPickerCallback::pickerCallBack;
        if (photoPickerCallback != nullptr) {
            photoPickerCallback->resultCode = RESULT_OK;
            photoPickerCallback->uris = uriNow;
            photoPickerCallback->ready = true;
            photoPickerCallback->isOrigin = true;
        }
    }];
}
}
}
