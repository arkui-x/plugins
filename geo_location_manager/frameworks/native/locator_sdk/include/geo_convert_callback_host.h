/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef GEO_CONVERT_CALLBACK_HOST_H
#define GEO_CONVERT_CALLBACK_HOST_H

#include "i_geocode_callback.h"
#include "iremote_stub.h"

namespace OHOS {
namespace Location {
class GeoConvertCallbackHost : public IRemoteStub<IGeocodeCallback> {
public:
    void OnResults(std::list<std::shared_ptr<GeoAddress>> &results) override;
    void OnErrorReport(const int errorCode) override;
    std::list<std::shared_ptr<GeoAddress>> GetResult();

    bool ready_ = false;
    std::list<std::shared_ptr<GeoAddress>> result_;
    std::mutex mutex_;
    std::condition_variable condition_;
};
} // namespace Location
} // namespace OHOS
#endif // FEATURE_GEOCODE_SUPPORT
