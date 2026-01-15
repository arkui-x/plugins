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

#include "tlv_utils.h"

#include <json/json.h>

#include "pasteboard_hilog.h"

namespace OHOS::MiscServices {

namespace {
const std::string BUNDLE_NAME_KEY = "bundleName";
const std::string ABILITY_NAME_KEY = "abilityName";
const std::string MODULE_NAME_KEY = "moduleName";
const std::string TYPE_KEY = "type";
const std::string JSON_CONFIG_INDENTATION = "indentation";
const std::string JSON_CONFIG_COLLECT_COMMENTS = "collectComments";
const std::string JSON_CONFIG_COMMENT_STYLE = "commentStyle";
const std::string JSON_CONFIG_EMIT_UTF8 = "emitUTF8";
} // namespace

RawMem TLVUtils::Parcelable2Raw(const Parcelable* value)
{
    RawMem rawMem {};
    if (value == nullptr) {
        return rawMem;
    }

    rawMem.parcel = std::make_shared<Parcel>(nullptr);
    bool ret = value->Marshalling(*rawMem.parcel);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, rawMem, PASTEBOARD_MODULE_COMMON, "Marshalling failed");

    rawMem.buffer = rawMem.parcel->GetData();
    rawMem.bufferLen = rawMem.parcel->GetDataSize();
    return rawMem;
}

bool TLVUtils::Raw2Parcel(const RawMem& rawMem, Parcel& parcel)
{
    if (rawMem.buffer == 0 || rawMem.bufferLen == 0) {
        return false;
    }
    auto* temp = malloc(rawMem.bufferLen); // free by Parcel!
    if (temp == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "malloc failed, size=%{public}zu", rawMem.bufferLen);
        return false;
    }
    auto err = memcpy_s(temp, rawMem.bufferLen, reinterpret_cast<const void*>(rawMem.buffer), rawMem.bufferLen);
    if (err != EOK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "memcpy RawMem failed, size=%{public}zu", rawMem.bufferLen);
        free(temp);
        return false;
    }
    bool ret = parcel.ParseFrom(reinterpret_cast<uintptr_t>(temp), rawMem.bufferLen);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "parse from RawMem failed, size=%{public}zu", rawMem.bufferLen);
        free(temp);
        return false;
    }
    return true;
}

std::shared_ptr<Media::PixelMap> TLVUtils::Vector2PixelMap(std::vector<std::uint8_t>& value)
{
    return value.empty() ? nullptr : std::shared_ptr<Media::PixelMap>(Media::PixelMap::DecodeTlv(value));
}

std::vector<std::uint8_t> TLVUtils::PixelMap2Vector(std::shared_ptr<Media::PixelMap> pixelMap)
{
    if (pixelMap == nullptr) {
        return {};
    }

    std::vector<std::uint8_t> value;
    bool ret = pixelMap->EncodeTlv(value);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, {}, PASTEBOARD_MODULE_COMMON, "EncodeTlv failed");

    return value;
}

std::string TLVUtils::Want2Json(const AAFwk::Want& want)
{
    Json::Value root;
    const std::string wantJson = want.ToJson();
    Json::CharReaderBuilder readerBuilder;
    readerBuilder[JSON_CONFIG_COLLECT_COMMENTS] = false;
    std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
    if (reader != nullptr) {
        Json::Value parsed;
        std::string errs;
        if (reader->parse(wantJson.data(), wantJson.data() + wantJson.size(), &parsed, &errs) && parsed.isObject()) {
            root = std::move(parsed);
        }
    }
    if (!root.isObject()) {
        root = Json::Value(Json::objectValue);
    }
    root[BUNDLE_NAME_KEY] = want.GetBundleName();
    root[ABILITY_NAME_KEY] = want.GetAbilityName();
    root[MODULE_NAME_KEY] = want.GetModuleName();
    root[TYPE_KEY] = want.GetType();
    Json::StreamWriterBuilder writerBuilder;
    writerBuilder[JSON_CONFIG_COMMENT_STYLE] = "None";
    writerBuilder[JSON_CONFIG_INDENTATION] = "";
    writerBuilder[JSON_CONFIG_EMIT_UTF8] = true;
    std::string out = Json::writeString(writerBuilder, root);
    if (!out.empty() && out.back() == '\n') {
        out.pop_back();
    }
    return out;
}

std::shared_ptr<AAFwk::Want> TLVUtils::Json2Want(const std::string& json)
{
    if (json.empty()) {
        return nullptr;
    }
    Json::CharReaderBuilder readerBuilder;
    readerBuilder[JSON_CONFIG_COLLECT_COMMENTS] = false;
    std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
    if (reader == nullptr) {
        return nullptr;
    }
    Json::Value root;
    std::string errs;
    if (!reader->parse(json.data(), json.data() + json.size(), &root, &errs) || !root.isObject()) {
        return nullptr;
    }

    auto want = std::make_shared<AAFwk::Want>();
    if (want == nullptr) {
        return nullptr;
    }
    want->ParseJson(json);
    const auto& bundleName = root[BUNDLE_NAME_KEY];
    if (bundleName.isString()) {
        want->SetBundleName(bundleName.asString());
    }
    const auto& abilityName = root[ABILITY_NAME_KEY];
    if (abilityName.isString()) {
        want->SetAbilityName(abilityName.asString());
    }
    const auto& moduleName = root[MODULE_NAME_KEY];
    if (moduleName.isString()) {
        want->SetModuleName(moduleName.asString());
    }
    const auto& type = root[TYPE_KEY];
    if (type.isString()) {
        want->SetType(type.asString());
    }
    return want;
}
} // namespace OHOS::MiscServices
