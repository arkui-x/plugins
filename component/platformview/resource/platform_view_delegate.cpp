/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "platform_view_delegate.h"
#include "base/log/log.h"
#include "base/utils/utils.h"

namespace OHOS::Ace {
const char REGISTER_PLATFORM_VIEW[] = "registerPlatformView";
const char PLATFORM_VIEW_READY[] = "platformViewReady";
const char PLATFORM_VIEW_TYPE[] = "platformViewType";
const char UPDATE_PLATFORM_VIEW_LAYOUT[] = "updateLayout";
const char EXCHANGE_BIND[] = "exchangeBind";

const char PLATFORM_VIEW_TOUCH_DOWN[] = "touchDown";
const char PLATFORM_VIEW_TOUCH_UP[] = "touchUp";
const char PLATFORM_VIEW_TOUCH_MOVE[] = "touchMove";
const char PLATFORM_VIEW_TOUCH_CANCEL[] = "touchCancel";
const char DISPOSE_PLATFORM_VIEW[] = "dispose";
const char PLATFORM_VIEW_HEIGHT[] = "platformViewHeight";
const char PLATFORM_VIEW_WIDTH[] = "platformViewWidth";
const char PLATFORM_VIEW_TOP[] = "platformViewTop";
const char PLATFORM_VIEW_LEFT[] = "platformViewLeft";
const char PLATFORM_VIEW_TOUCH_POINT_OFFSET_X[] = "platformViewTouchPointOffsetX";
const char PLATFORM_VIEW_TOUCH_POINT_OFFSET_Y[] = "platformViewTouchPointOffsetY";
const char VIEW_TAG[] = "viewTag";
const char DATA_TAG[] = "dataTag";
const char DATA_EMPTY_TAG[] = "dataEmptyTag";
const char TEXTURE_ID[] = "textureId";
const char INSTANCE_ID[] = "instanceId";
const char PLATFORM_VIEW_ERROR_CODE_CREATEFAIL[] = "error_platform_view_000001";
const char PLATFORM_VIEW_ERROR_MSG_CREATEFAIL[] = "Create platformView failed.";
const char SET_SCALE[] = "setScale";
const char SET_ROTATION[] = "setRotation";
const char SET_TRANSLATE[] = "setTranslate";
const char SET_TRANSFORM_MATRIX[] = "setTransformMatrix";
const char X[] = "X";
const char Y[] = "Y";
const char Z[] = "Z";
const char CENTER_X[] = "centerX";
const char CENTER_Y[] = "centerY";
const char CENTER_Z[] = "centerZ";
const char PERSPECTIVE[] = "perspective";
const char ANGLE[] = "angle";

PlatformViewDelegate::~PlatformViewDelegate()
{
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto resRegister = context->GetPlatformResRegister();
    CHECK_NULL_VOID(resRegister);
    auto platformTaskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::PLATFORM);
    if (platformTaskExecutor.IsRunOnCurrentThread()) {
        resRegister->UnregisterEvent(MakeEventHash(PLATFORM_VIEW_READY));
    } else {
        WeakPtr<PlatformResRegister> weak = resRegister;
        platformTaskExecutor.PostTask(
            [eventHash = MakeEventHash(PLATFORM_VIEW_READY), weak] {
                auto resRegister = weak.Upgrade();
                CHECK_NULL_VOID(resRegister);
                resRegister->UnregisterEvent(eventHash);
            },
            "ArkUI-XPlatformViewDelegatePlatformViewDelegate");
    }
}

void PlatformViewDelegate::Create(const std::string& viewTag, const std::optional<std::string>& data)
{
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);

    auto platformTaskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::PLATFORM);
    platformTaskExecutor.PostSyncTask(
        [weak = WeakClaim(this), viewTag, data] {
            auto platformView = weak.Upgrade();
            if (platformView) {
                platformView->CreatePlatformView(viewTag, data);
            }
        },
        "ArkUI-XPlatformViewDelegateCreate");
}

void PlatformViewDelegate::CreatePlatformView(const std::string& viewTag, const std::optional<std::string>& data)
{
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto resRegister = context->GetPlatformResRegister();
    CHECK_NULL_VOID(resRegister);
    std::stringstream paramStream;
    paramStream << VIEW_TAG << PLATFORM_VIEW_PARAM_EQUALS << viewTag;
    if (data.has_value()) {
        paramStream << PLATFORM_VIEW_PARAM_AND << DATA_TAG << PLATFORM_VIEW_PARAM_EQUALS << data.value();
        paramStream << PLATFORM_VIEW_PARAM_AND << DATA_EMPTY_TAG << PLATFORM_VIEW_PARAM_EQUALS << data->empty();
    }
    std::string param = paramStream.str();
    id_ = resRegister->CreateResource(type_, param);
    if (id_ == PLATFORM_VIEW_INVALID_ID) {
        onError_(PLATFORM_VIEW_ERROR_CODE_CREATEFAIL, PLATFORM_VIEW_ERROR_MSG_CREATEFAIL);
        return;
    }
    hash_ = MakeResourceHash();

    resRegister->RegisterEvent(
        MakeEventHash(PLATFORM_VIEW_READY), [weak = WeakClaim(this)](const std::string& param) {
            auto platformView = weak.Upgrade();
            if (platformView) {
                if (platformView->onPlatformViewReady_) {
                    platformView->onPlatformViewReady_();
                }
            }
        });
}

void PlatformViewDelegate::RegisterPlatformViewTexture(int64_t textureId, std::string& viewTag)
{
    std::stringstream paramStream;
    paramStream << TEXTURE_ID << PLATFORM_VIEW_PARAM_EQUALS << textureId << PLATFORM_VIEW_PARAM_AND << VIEW_TAG
                << PLATFORM_VIEW_PARAM_EQUALS << viewTag;
    std::string param = paramStream.str();
    CallResRegisterMethod(MakeMethodHash(REGISTER_PLATFORM_VIEW), param);
}

void PlatformViewDelegate::RegisterPlatformView(std::string& viewTag)
{
    std::stringstream paramStream;
    paramStream << VIEW_TAG << PLATFORM_VIEW_PARAM_EQUALS << viewTag;
    std::string param = paramStream.str();
    CallResRegisterMethod(MakeMethodHash(REGISTER_PLATFORM_VIEW), param);
}

void PlatformViewDelegate::UpdatePlatformViewLayout(const NG::SizeF& drawSize, const NG::OffsetF& offset)
{
    std::stringstream paramStream;
    paramStream << PLATFORM_VIEW_WIDTH << PLATFORM_VIEW_PARAM_EQUALS
                << drawSize.Width() << PLATFORM_VIEW_PARAM_AND
                << PLATFORM_VIEW_HEIGHT << PLATFORM_VIEW_PARAM_EQUALS
                << drawSize.Height() << PLATFORM_VIEW_PARAM_AND
                << PLATFORM_VIEW_LEFT << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetX() << PLATFORM_VIEW_PARAM_AND
                << PLATFORM_VIEW_TOP << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetY();
    std::string param = paramStream.str();
    CallResRegisterMethod(MakeMethodHash(UPDATE_PLATFORM_VIEW_LAYOUT), param);
}

void PlatformViewDelegate::ExchangeBind()
{
    CallSyncResRegisterMethod(MakeMethodHash(EXCHANGE_BIND), "");
}

void PlatformViewDelegate::GetPlatformViewType(int32_t& type)
{
    CallSyncResRegisterMethod(MakeMethodHash(PLATFORM_VIEW_TYPE), "",
        [this, &type](std::string& result) mutable {
            type = GetIntParam(result, "type");
    });
}

void PlatformViewDelegate::HandleTouchDown(const NG::OffsetF& offset)
{
    std::stringstream paramStream;
    paramStream << PLATFORM_VIEW_TOUCH_POINT_OFFSET_X << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetX() << PLATFORM_VIEW_PARAM_AND
                << PLATFORM_VIEW_TOUCH_POINT_OFFSET_Y << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetY();
    std::string param = paramStream.str();
    CallResRegisterMethod(MakeMethodHash(PLATFORM_VIEW_TOUCH_DOWN), param);
}

void PlatformViewDelegate::HandleTouchUp(const NG::OffsetF& offset)
{
    std::stringstream paramStream;
    paramStream << PLATFORM_VIEW_TOUCH_POINT_OFFSET_X << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetX() << PLATFORM_VIEW_PARAM_AND
                << PLATFORM_VIEW_TOUCH_POINT_OFFSET_Y << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetY();
    std::string param = paramStream.str();
    CallResRegisterMethod(MakeMethodHash(PLATFORM_VIEW_TOUCH_UP), param);
}

void PlatformViewDelegate::HandleTouchMove(const NG::OffsetF& offset)
{
    std::stringstream paramStream;
    paramStream << PLATFORM_VIEW_TOUCH_POINT_OFFSET_X << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetX() << PLATFORM_VIEW_PARAM_AND
                << PLATFORM_VIEW_TOUCH_POINT_OFFSET_Y << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetY();
    std::string param = paramStream.str();
    CallResRegisterMethod(MakeMethodHash(PLATFORM_VIEW_TOUCH_MOVE), param);
}

void PlatformViewDelegate::HandleTouchCancel(const NG::OffsetF& offset)
{
    std::stringstream paramStream;
    paramStream << PLATFORM_VIEW_TOUCH_POINT_OFFSET_X << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetX() << PLATFORM_VIEW_PARAM_AND
                << PLATFORM_VIEW_TOUCH_POINT_OFFSET_Y << PLATFORM_VIEW_PARAM_EQUALS
                << offset.GetY();
    std::string param = paramStream.str();
    CallResRegisterMethod(MakeMethodHash(PLATFORM_VIEW_TOUCH_CANCEL), param);
}

void PlatformViewDelegate::Dispose()
{
    CallResRegisterMethod(MakeMethodHash(DISPOSE_PLATFORM_VIEW), "");
}

void PlatformViewDelegate::SetScale(float x, float y, float z, const std::string& centerX, const std::string& centerY)
{
    std::stringstream paramStream;
    paramStream << X << PLATFORM_VIEW_PARAM_EQUALS
                << x << PLATFORM_VIEW_PARAM_AND
                << Y << PLATFORM_VIEW_PARAM_EQUALS
                << y << PLATFORM_VIEW_PARAM_AND
                << Z << PLATFORM_VIEW_PARAM_EQUALS
                << z << PLATFORM_VIEW_PARAM_AND
                << CENTER_X << PLATFORM_VIEW_PARAM_EQUALS
                << centerX << PLATFORM_VIEW_PARAM_AND
                << CENTER_Y << PLATFORM_VIEW_PARAM_EQUALS
                << centerY;
    std::string param = paramStream.str();
    CallResRegisterMethod(MakeMethodHash(SET_SCALE), param);
}

void PlatformViewDelegate::SetRotation(float x, float y, float z, const std::string& angle, const std::string& centerX,
    const std::string& centerY, const std::string& centerZ, const std::string& perspective)
{
    std::stringstream paramStream;
    paramStream << X << PLATFORM_VIEW_PARAM_EQUALS
                << x << PLATFORM_VIEW_PARAM_AND
                << Y << PLATFORM_VIEW_PARAM_EQUALS
                << y << PLATFORM_VIEW_PARAM_AND
                << Z << PLATFORM_VIEW_PARAM_EQUALS
                << z << PLATFORM_VIEW_PARAM_AND
                << ANGLE << PLATFORM_VIEW_PARAM_EQUALS
                << angle << PLATFORM_VIEW_PARAM_AND
                << CENTER_X << PLATFORM_VIEW_PARAM_EQUALS
                << centerX << PLATFORM_VIEW_PARAM_AND
                << CENTER_Y << PLATFORM_VIEW_PARAM_EQUALS
                << centerY << PLATFORM_VIEW_PARAM_AND
                << CENTER_Z << PLATFORM_VIEW_PARAM_EQUALS
                << centerZ << PLATFORM_VIEW_PARAM_AND
                << PERSPECTIVE << PLATFORM_VIEW_PARAM_EQUALS
                << perspective;
    std::string param = paramStream.str();
    CallResRegisterMethod(MakeMethodHash(SET_ROTATION), param);
}

void PlatformViewDelegate::SetTranslate(const std::string& x, const std::string& y, const std::string& z)
{
    std::stringstream paramStream;
    paramStream << X << PLATFORM_VIEW_PARAM_EQUALS
                << x << PLATFORM_VIEW_PARAM_AND
                << Y << PLATFORM_VIEW_PARAM_EQUALS
                << y << PLATFORM_VIEW_PARAM_AND
                << Z << PLATFORM_VIEW_PARAM_EQUALS
                << z;
    std::string param = paramStream.str();
    CallResRegisterMethod(MakeMethodHash(SET_TRANSLATE), param);
}

void PlatformViewDelegate::SetTransformMatrix(const std::vector<float>& matrix)
{
    std::stringstream paramStream;
    for (size_t i = 0; i < matrix.size(); ++i) {
        paramStream << "m" << i << PLATFORM_VIEW_PARAM_EQUALS << matrix[i];
        if (i < matrix.size() - 1) {
            paramStream << PLATFORM_VIEW_PARAM_AND;
        }
    }
    std::string param = paramStream.str();
    CallResRegisterMethod(MakeMethodHash(SET_TRANSFORM_MATRIX), param);
}
} // namespace OHOS::Ace