/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
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

#ifndef DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_UTILS_H
#define DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_UTILS_H

#include "parcel.h"
#include "securec.h"
#include "unified_meta.h"

namespace OHOS::MiscServices {

struct RawMem {
    uintptr_t buffer;
    size_t bufferLen;
    // notice:Keep the parcel reference to prevent the memory in the parcel from being destructed
    std::shared_ptr<OHOS::Parcel> parcel;
};

class TLVUtils {
public:
    // parcelable to buffer
    static RawMem Parcelable2Raw(const Parcelable* value);

    // buffer to parcelable
    template<typename ParcelableType>
    static std::shared_ptr<ParcelableType> Raw2Parcelable(const RawMem& rawMem)
    {
        Parcel parcel(nullptr);
        if (!Raw2Parcel(rawMem, parcel)) {
            return nullptr;
        }
        auto* raw = ParcelableType::Unmarshalling(parcel);
        return raw == nullptr ? nullptr : std::shared_ptr<ParcelableType>(raw);
    }
    // buffer to parcelable
    API_EXPORT static bool Raw2Parcel(const RawMem& rawMem, Parcel& parcel);

    static std::shared_ptr<Media::PixelMap> Vector2PixelMap(std::vector<std::uint8_t>& value);

    static std::vector<std::uint8_t> PixelMap2Vector(std::shared_ptr<Media::PixelMap> pixelMap);

    static std::string Want2Json(const AAFwk::Want& want);

    static std::shared_ptr<AAFwk::Want> Json2Want(const std::string& json);
};
} // namespace OHOS::MiscServices
#endif // DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_UTILS_H
