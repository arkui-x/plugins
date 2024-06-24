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

#ifndef PHOTO_PICKER_CALLBACK_H
#define PHOTO_PICKER_CALLBACK_H

#include <string>
#include <vector>

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
struct PickerCallBack {
    bool ready = false;
    bool isOrigin;
    int32_t resultCode;
    std::vector<std::string> uris;
};

class PhotoPickerCallback {
public:
    EXPORT PhotoPickerCallback();
    EXPORT ~PhotoPickerCallback();
    static std::shared_ptr<PickerCallBack> pickerCallBack;
};
} // namespace Media
} // namespace OHOS
#endif /* PHOTO_PICKER_NAPI_H */
