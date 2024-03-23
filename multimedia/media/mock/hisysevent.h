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
#ifndef HI_SYS_EVENT_H
#define HI_SYS_EVENT_H
#include "hisysevent_c.h"

#ifdef __cplusplus

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "encoded_param.h"
#include "def.h"
#include "hisysevent_c.h"
#include "raw_data.h"
#include "stringfilter.h"
#include "write_controller.h"

/*
 * Usage: define string macro "DOMAIN_MASKS" to disable one or more components.
 *     Method1: add macro in this header file, e.g.          #define DOMAIN_MASKS "AAFWK|APPEXECFWK|ACCOUNT"
 *     Method1: addd cflags in build.gn file, e.g.           -D DOMAIN_MASKS="AAFWK|APPEXECFWK|ACCOUNT"
 */
namespace OHOS {
namespace HiviewDFX {
// init domain masks
#ifndef DOMAIN_MASKS
#define DOMAIN_MASKS ""
#endif

static constexpr char DOMAIN_MASKS_DEF[] = DOMAIN_MASKS;

// split domain masks by '|', then compare with str
template<const std::string_view& str, const std::string_view& masks, std::string::size_type pos = 0>
struct IsMaskedImpl {
    inline static constexpr auto newpos = masks.find('|', pos);
    inline static constexpr bool value =
        IsMaskedImpl<str, masks, (newpos != std::string_view::npos) ? pos + 1 : newpos>::value ||
        (pos != newpos && pos != masks.size() &&
            masks.substr(pos, (newpos != std::string_view::npos) ? newpos - pos : str.size() - pos).compare(str) == 0);
};

template<const std::string_view& str, const std::string_view& masks>
struct IsMaskedImpl<str, masks, std::string_view::npos> {
    inline static constexpr bool value = false;
};

template<const char* domain, const char* domainMasks>
struct IsMaskedCvt {
    inline static constexpr std::string_view domainView {domain};
    inline static constexpr std::string_view domainMasksView {domainMasks};
    inline static constexpr bool value = IsMaskedImpl<domainView, domainMasksView>::value;
};

template<const char* domain>
inline static constexpr bool isMasked = IsMaskedCvt<domain, DOMAIN_MASKS_DEF>::value;

class HiSysEvent {
public:
    friend class HiSysEvent;
    friend class NapiHiSysEventAdapter;

    // system event domain list
    class Domain {
    public:
        static constexpr char AAFWK[] = "AAFWK";
        static constexpr char ACCESS_TOKEN[] = "ACCESS_TOKEN";
        static constexpr char ACCESSIBILITY[] = "ACCESSIBILITY";
        static constexpr char ACCOUNT[] = "ACCOUNT";
        static constexpr char ACE[] = "ACE";
        static constexpr char AI[] = "AI";
        static constexpr char APPEXECFWK[] = "APPEXECFWK";
        static constexpr char AUDIO[] = "AUDIO";
        static constexpr char AV_SESSION[] = "AV_SESSION";
        static constexpr char AV_CODEC[] = "AV_CODEC";
        static constexpr char BACKGROUND_TASK[] = "BACKGROUND_TASK";
        static constexpr char BARRIER_FREE[] = "BARRIERFREE";
        static constexpr char BATTERY[] = "BATTERY";
        static constexpr char BIOMETRICS[] = "BIOMETRICS";
        static constexpr char BLUETOOTH[] = "BLUETOOTH";
        static constexpr char BT_SERVICE[] = "BT_SERVICE";
        static constexpr char BUNDLE_MANAGER[] = "BUNDLE_MANAGER";
        static constexpr char CAM_DRV[] = "CAM_DRV";
        static constexpr char CAMERA[] = "CAMERA";
        static constexpr char CAMERA_IPS[] = "CAMERA_IPS";
        static constexpr char CAST_ENGINE[] = "CASTENGINE";
        static constexpr char CCRUNTIME[] = "CCRUNTIME";
        static constexpr char CERT_MANAGER[] = "CERT_MANAGER";
        static constexpr char CODE_SIGN[] = "CODE_SIGN";
        static constexpr char COMMONEVENT[] = "COMMONEVENT";
        static constexpr char COMMUNICATION[] = "COMMUNICATION";
        static constexpr char CONTAINER[] = "CONTAINER";
        static constexpr char CONTAINER_COMM[] = "CONTAINER_COMM";
        static constexpr char CUSTOMIZATION_CONFIG[] = "CUST_CONFIG";
        static constexpr char CUSTOMIZATION_EDM[] = "CUST_EDM";
        static constexpr char DEVELOPTOOLS[] = "DEVELOPTOOLS";
        static constexpr char DEVICE_AUTH[] = "DEVICE_AUTH";
        static constexpr char DEVICE_PROFILE[] = "DEVICE_PROFILE";
        static constexpr char DISPLAY[] = "DISPLAY";
        static constexpr char DISTRIBUTED_AUDIO[] = "DISTAUDIO";
        static constexpr char DISTRIBUTED_CAMERA[] = "DISTCAMERA";
        static constexpr char DISTRIBUTED_DATAMGR[] = "DISTDATAMGR";
        static constexpr char DISTRIBUTED_DEVICE_MANAGER[] = "DISTDM";
        static constexpr char DISTRIBUTED_HARDWARE_FWK[] = "DISTHWFWK";
        static constexpr char DISTRIBUTED_INPUT[] = "DISTINPUT";
        static constexpr char DISTRIBUTED_SCHEDULE[] = "DISTSCHEDULE";
        static constexpr char DISTRIBUTED_SCREEN[] = "DISTSCREEN";
        static constexpr char DLP_CRE_SERVICE[] = "DLP_CRE_SERVICE";
        static constexpr char DLP[] = "DLP";
        static constexpr char DSLM[] = "DSLM";
        static constexpr char DSOFTBUS[] = "DSOFTBUS";
        static constexpr char FACEID_ALGO[] = "FACEID_ALGO";
        static constexpr char FACE_SETTINGS[] = "FACE_SETTINGS";
        static constexpr char FIDO[] = "FIDO";
        static constexpr char FILEMANAGEMENT[] = "FILEMANAGEMENT";
        static constexpr char FFRT[] = "FFRT";
        static constexpr char FORM_MANAGER[] = "FORM_MANAGER";
        static constexpr char FP_SETTINGS[] = "FP_SETTINGS";
        static constexpr char FRAMEWORK[] = "FRAMEWORK";
        static constexpr char GLOBAL_I18N[] = "GLOBAL_I18N";
        static constexpr char GLOBAL_RESMGR[] = "GLOBAL_RESMGR";
        static constexpr char GLOBAL[] = "GLOBAL";
        static constexpr char GRAPHIC[] = "GRAPHIC";
        static constexpr char HDF_USB[] = "HDF_USB";
        static constexpr char HIVIEWDFX[] = "HIVIEWDFX";
        static constexpr char HUKS[] = "HUKS";
        static constexpr char IAWARE[] = "IAWARE";
        static constexpr char IFAA[] = "IFAA";
        static constexpr char IMS_UT[] = "IMS_UT";
        static constexpr char INPUTMETHOD[] = "INPUTMETHOD";
        static constexpr char INPUT_UE[] = "INPUT_UE";
        static constexpr char INTELLI_ACCESSORIES[] = "INTELLIACC";
        static constexpr char INTELLI_TV[] = "INTELLITV";
        static constexpr char ISHARE[] = "ISHARE";
        static constexpr char IVI_HARDWARE[] = "IVIHARDWARE";
        static constexpr char LIVEVIEW_MANAGER[] = "LIVEVIEW_MANAGER";
        static constexpr char LOCATION[] = "LOCATION";
        static constexpr char MEDICAL_SENSOR[] = "MEDICAL_SENSOR";
        static constexpr char MEMMGR[] = "MEMMGR";
        static constexpr char MISCDEVICE[] = "MISCDEVICE";
        static constexpr char MSDP[] = "MSDP";
        static constexpr char MULTI_MEDIA[] = "MULTIMEDIA";
        static constexpr char MULTI_MODAL_INPUT[] = "MULTIMODALINPUT";
        static constexpr char MULTIWINDOW_UE[] = "MULTIWINDOW_UE";
        static constexpr char NETMANAGER_STANDARD[] = "NETMANAGER";
        static constexpr char NFC[] = "NFC";
        static constexpr char NFC_EXT[] = "NFC_EXT";
        static constexpr char NOTIFICATION[] = "NOTIFICATION";
        static constexpr char PASTEBOARD[] = "PASTEBOARD";
        static constexpr char POWER[] = "POWER";
        static constexpr char PUSH_MANAGER[] = "PUSH_MANAGER";
        static constexpr char RELIABILITY[] = "RELIABILITY";
        static constexpr char REQUEST[] = "REQUEST";
        static constexpr char ROUTER[] = "ROUTER";
        static constexpr char RSS[] = "RSS";
        static constexpr char SAMGR[] = "SAMGR";
        static constexpr char SECURE_ELEMENT[] = "SECURE_ELEMENT";
        static constexpr char SECURITY_AUDIT[] = "SECURITY_AUDIT";
        static constexpr char SECURITY_GUARD[] = "SECURITY_GUARD";
        static constexpr char SEC_COMPONENT[] = "SEC_COMPONENT";
        static constexpr char SENSOR[] = "SENSOR";
        static constexpr char SOURCE_CODE_TRANSFORMER[] = "SRCTRANSFORMER";
        static constexpr char STARTUP[] = "STARTUP";
        static constexpr char STATS[] = "STATS";
        static constexpr char TELEPHONY[] = "TELEPHONY";
        static constexpr char THEME[] = "THEME";
        static constexpr char THERMAL[] = "THERMAL";
        static constexpr char TIME[] = "TIME";
        static constexpr char UPDATE[] = "UPDATE";
        static constexpr char UPDATE_DUE[] = "UPDATE_DUE";
        static constexpr char UPDATE_OUC[] = "UPDATE_OUC";
        static constexpr char USB[] = "USB";
        static constexpr char USERIAM_FACE_HDF[] = "USERIAM_FACE_HDF";
        static constexpr char USERIAM_FP_HDF[] = "USERIAM_FP_HDF";
        static constexpr char USERIAM_FWK[] = "USERIAM_FWK";
        static constexpr char USERIAM_FWK_HDF[] = "USERIAM_FWK_HDF";
        static constexpr char USERIAM_PIN[] = "USERIAM_PIN";
        static constexpr char VIR_SERVICE[] = "VIR_SERVICE";
        static constexpr char VM_SERVICE[] = "VM_SERVICE";
        static constexpr char WEARABLE_HARDWARE[] = "WEARABLEHW";
        static constexpr char WEARABLE[] = "WEARABLE";
        static constexpr char WEBVIEW[] = "WEBVIEW";
        static constexpr char WINDOW_MANAGER[] = "WINDOWMANAGER";
        static constexpr char WORK_SCHEDULER[] = "WORKSCHEDULER";
        static constexpr char OTHERS[] = "OTHERS";
    };

    enum EventType {
        FAULT     = 1,    // system fault event
        STATISTIC = 2,    // system statistic event
        SECURITY  = 3,    // system security event
        BEHAVIOR  = 4     // system behavior event
    };

public:
    template<typename... Types>
    static int Write(const char* func, int64_t line, const std::string &domain,
        const std::string &eventName, EventType type, Types... keyValues)
    {
        ControlParam param {
#ifdef HISYSEVENT_PERIOD
            .period = HISYSEVENT_PERIOD,
#else
            .period = HISYSEVENT_DEFAULT_PERIOD,
#endif
#ifdef HISYSEVENT_THRESHOLD
            .threshold = HISYSEVENT_THRESHOLD,
#else
            .threshold = HISYSEVENT_DEFAULT_THRESHOLD,
#endif
        };
        uint64_t timeStamp = controller.CheckLimitWritingEvent(param, domain.c_str(), eventName.c_str(),
            func, line);
        if (timeStamp == INVALID_TIME_STAMP) {
            return ERR_WRITE_IN_HIGH_FREQ;
        }
        return InnerWrite(domain, eventName, type, timeStamp, keyValues...);
    }

    template<const char* domain, typename... Types, std::enable_if_t<!isMasked<domain>>* = nullptr>
    static int Write(const char* func, int64_t line, const std::string& eventName,
        EventType type, Types... keyValues)
    {
        ControlParam param {
#ifdef HISYSEVENT_PERIOD
            .period = HISYSEVENT_PERIOD,
#else
            .period = HISYSEVENT_DEFAULT_PERIOD,
#endif
#ifdef HISYSEVENT_THRESHOLD
            .threshold = HISYSEVENT_THRESHOLD,
#else
            .threshold = HISYSEVENT_DEFAULT_THRESHOLD,
#endif
        };
        uint64_t timeStamp = controller.CheckLimitWritingEvent(param, domain, eventName.c_str(),
            func, line);
        if (timeStamp == INVALID_TIME_STAMP) {
            return ERR_WRITE_IN_HIGH_FREQ;
        }
        return InnerWrite(std::string(domain), eventName, type, timeStamp, keyValues...);
    }

    template<const char* domain, typename... Types, std::enable_if_t<isMasked<domain>>* = nullptr>
    inline static constexpr int Write(const char*, int64_t, const std::string&, EventType, Types...)
    {
        // do nothing
        return ERR_DOMAIN_MASKED;
    }

private:
    class EventBase {
    public:
        EventBase(const std::string& domain, const std::string& eventName, int type, uint64_t timeStamp = 0);
        ~EventBase() = default;

    public:
        int GetRetCode();
        void SetRetCode(int retCode);
        void AppendParam(std::shared_ptr<Encoded::EncodedParam> param);
        void WritebaseInfo();
        size_t GetParamCnt();
        std::shared_ptr<Encoded::RawData> GetEventRawData();

    private:
        int retCode_ = 0;
        size_t paramCnt_ = 0;
        size_t paramCntWroteOffset_ = 0;
        struct Encoded::HiSysEventHeader header_ = {
            .domain = {0},
            .name = {0},
            .timestamp = 0,
            .timeZone = 0,
            .uid = 0,
            .pid = 0,
            .tid = 0,
            .id = 0,
            .type = 0,
            .isTraceOpened = 0,
        };
        struct Encoded::TraceInfo traceInfo_ = {
            .traceFlag = 0,
            .traceId = 0,
            .spanId = 0,
            .pSpanId = 0,
        };
        std::shared_ptr<Encoded::RawData> rawData_ = nullptr;
    };

private:
    template<typename... Types>
    static int InnerWrite(const std::string& domain, const std::string& eventName,
        int type, uint64_t timeStamp, Types... keyValues)
    {
        if (!StringFilter::GetInstance().IsValidName(domain, MAX_DOMAIN_LENGTH)) {
            return ExplainThenReturnRetCode(ERR_DOMAIN_NAME_INVALID);
        }
        if (!StringFilter::GetInstance().IsValidName(eventName, MAX_EVENT_NAME_LENGTH)) {
            return ExplainThenReturnRetCode(ERR_EVENT_NAME_INVALID);
        }
        EventBase eventBase(domain, eventName, type, timeStamp);
        WritebaseInfo(eventBase);
        if (IsError(eventBase)) {
            return ExplainThenReturnRetCode(eventBase.GetRetCode());
        }

        InnerWrite(eventBase, keyValues...);
        if (IsError(eventBase)) {
            return ExplainThenReturnRetCode(eventBase.GetRetCode());
        }

        SendSysEvent(eventBase);
        return eventBase.GetRetCode();
    }

    static bool CheckParamValidity(EventBase& eventBase, const std::string &key)
    {
        if (IsWarnAndUpdate(CheckKey(key), eventBase)) {
            return false;
        }
        if (UpdateAndCheckKeyNumIsOver(eventBase)) {
            return false;
        }
        return true;
    }

    template<typename T>
    static bool CheckArrayValidity(EventBase& eventBase, const T* array)
    {
        if (array == nullptr) {
            eventBase.SetRetCode(ERR_VALUE_INVALID);
            return false;
        }
        return true;
    }

    template<typename T>
    static bool CheckArrayParamsValidity(EventBase& eventBase, const std::string& key, const std::vector<T>& value)
    {
        if (!CheckParamValidity(eventBase, key)) {
            return false;
        }
        if (value.empty()) {
            std::vector<bool> boolArrayValue;
            eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedArrayParam<bool>>(key,
                boolArrayValue));
            return false;
        }
        IsWarnAndUpdate(CheckArraySize(value.size()), eventBase);
        return true;
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, bool value, Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedParam<bool>>(key, value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const char value, Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedParam<int8_t>>(key,
                static_cast<int8_t>(value)));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const unsigned char value,
        Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.AppendParam(std::make_shared<Encoded::UnsignedVarintEncodedParam<uint8_t>>(key,
                static_cast<uint8_t>(value)));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const short value, Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedParam<int16_t>>(key,
                static_cast<int16_t>(value)));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const unsigned short value,
        Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.AppendParam(std::make_shared<Encoded::UnsignedVarintEncodedParam<uint16_t>>(key,
                static_cast<uint16_t>(value)));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const int value, Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedParam<int32_t>>(key, value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const unsigned int value,
        Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.AppendParam(std::make_shared<Encoded::UnsignedVarintEncodedParam<uint32_t>>(key, value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const long value, Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedParam<int64_t>>(key,
                static_cast<int64_t>(value)));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const unsigned long value,
        Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.AppendParam(std::make_shared<Encoded::UnsignedVarintEncodedParam<uint64_t>>(key,
                static_cast<uint64_t>(value)));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const long long value, Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedParam<int64_t>>(key,
                static_cast<int64_t>(value)));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const unsigned long long value,
        Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.AppendParam(std::make_shared<Encoded::UnsignedVarintEncodedParam<uint64_t>>(key,
                static_cast<uint64_t>(value)));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const float value, Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.AppendParam(std::make_shared<Encoded::FloatingNumberEncodedParam<float>>(key, value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const double value, Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            eventBase.AppendParam(std::make_shared<Encoded::FloatingNumberEncodedParam<double>>(key, value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::string& value,
        Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            IsWarnAndUpdate(CheckValue(value), eventBase);
            auto rawStr = StringFilter::GetInstance().EscapeToRaw(value);
            eventBase.AppendParam(std::make_shared<Encoded::StringEncodedParam>(key, rawStr));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const char* value, Types... keyValues)
    {
        if (CheckParamValidity(eventBase, key)) {
            IsWarnAndUpdate(CheckValue(std::string(value)), eventBase);
            auto rawStr = StringFilter::GetInstance().EscapeToRaw(std::string(value));
            eventBase.AppendParam(std::make_shared<Encoded::StringEncodedParam>(key, std::string(rawStr)));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<bool>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedArrayParam<bool>>(key, value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<char>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            std::vector<int8_t> translated;
            for (auto item : value) {
                translated.emplace_back(static_cast<int8_t>(item));
            }
            eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedArrayParam<int8_t>>(key, translated));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<unsigned char>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            std::vector<uint8_t> translated;
            for (auto item : value) {
                translated.emplace_back(static_cast<uint8_t>(item));
            }
            eventBase.AppendParam(std::make_shared<Encoded::UnsignedVarintEncodedArrayParam<uint8_t>>(key,
                translated));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<short>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            std::vector<int16_t> translated;
            for (auto item : value) {
                translated.emplace_back(static_cast<int16_t>(item));
            }
            eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedArrayParam<int16_t>>(key, translated));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<unsigned short>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            std::vector<uint16_t> translated;
            for (auto item : value) {
                translated.emplace_back(static_cast<uint16_t>(item));
            }
            eventBase.AppendParam(std::make_shared<Encoded::UnsignedVarintEncodedArrayParam<uint16_t>>(key,
                translated));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<int>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedArrayParam<int32_t>>(key, value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<unsigned int>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            eventBase.AppendParam(std::make_shared<Encoded::UnsignedVarintEncodedArrayParam<uint32_t>>(key, value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<long>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            std::vector<int64_t> translated;
            for (auto item : value) {
                translated.emplace_back(static_cast<int64_t>(item));
            }
            eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedArrayParam<int64_t>>(key, translated));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<unsigned long>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            std::vector<uint64_t> translated;
            for (auto item : value) {
                translated.emplace_back(static_cast<uint64_t>(item));
            }
            eventBase.AppendParam(std::make_shared<Encoded::UnsignedVarintEncodedArrayParam<uint64_t>>(key,
                translated));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<long long>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            std::vector<int64_t> translated;
            for (auto item : value) {
                translated.emplace_back(static_cast<int64_t>(item));
            }
            eventBase.AppendParam(std::make_shared<Encoded::SignedVarintEncodedArrayParam<int64_t>>(key, translated));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<unsigned long long>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            std::vector<uint64_t> translated;
            for (auto item : value) {
                translated.emplace_back(static_cast<uint64_t>(item));
            }
            eventBase.AppendParam(std::make_shared<Encoded::UnsignedVarintEncodedArrayParam<uint64_t>>(key,
                translated));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<float>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            eventBase.AppendParam(std::make_shared<Encoded::FloatingNumberEncodedArrayParam<float>>(key, value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<double>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            eventBase.AppendParam(std::make_shared<Encoded::FloatingNumberEncodedArrayParam<double>>(key, value));
        }
        InnerWrite(eventBase, keyValues...);
    }

    template<typename... Types>
    static void InnerWrite(EventBase& eventBase, const std::string& key, const std::vector<std::string>& value,
        Types... keyValues)
    {
        if (CheckArrayParamsValidity(eventBase, key, value)) {
            std::vector<std::string> rawStrs;
            for (auto& item : value) {
                IsWarnAndUpdate(CheckValue(item), eventBase);
                rawStrs.emplace_back(StringFilter::GetInstance().EscapeToRaw(item));
            }
            eventBase.AppendParam(std::make_shared<Encoded::StringEncodedArrayParam>(key, rawStrs));
        }
        InnerWrite(eventBase, keyValues...);
    }

private:
    static void InnerWrite(EventBase& eventBase);
    static void InnerWrite(EventBase& eventBase, const HiSysEventParam params[], size_t size);
    static void WritebaseInfo(EventBase& eventBase);
    static void AppendHexData(EventBase& eventBase, const std::string& key, uint64_t value);
    static int CheckKey(const std::string& key);
    static int CheckValue(const std::string& value);
    static int CheckArraySize(const size_t size);
    static bool IsErrorAndUpdate(int retCode, EventBase& eventBase);
    static bool IsWarnAndUpdate(int retCode, EventBase& eventBase);
    static bool UpdateAndCheckKeyNumIsOver(EventBase& eventBase);
    static bool IsError(EventBase& eventBase);
    static int ExplainThenReturnRetCode(const int retCode);
    static void SendSysEvent(EventBase& eventBase);
    static void AppendInvalidParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendBoolParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendInt8Param(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendUint8Param(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendInt16Param(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendUint16Param(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendInt32Param(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendUint32Param(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendInt64Param(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendUint64Param(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendFloatParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendDoubleParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendStringParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendBoolArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendInt8ArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendUint8ArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendInt16ArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendUint16ArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendInt32ArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendUint32ArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendInt64ArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendUint64ArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendFloatArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendDoubleArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendStringArrayParam(EventBase& eventBase, const HiSysEventParam& param);
    static void AppendParam(EventBase& eventBase, const HiSysEventParam& param);

    static WriteController controller;
};

/**
 * @brief Macro interface for writing system event.
 * @param domain      event domain.
 * @param eventName   event name.
 * @param type        event type.
 * @return 0 means success,
 *     greater than 0 also means success but with some data ignored,
 *     less than 0 means failure.
 */
#define HiSysEventWrite(domain, eventName, type, ...) \
({ \
    int hiSysEventWriteRet2023___ = OHOS::HiviewDFX::ERR_DOMAIN_MASKED; \
    if constexpr (!OHOS::HiviewDFX::isMasked<domain>) { \
        hiSysEventWriteRet2023___ = OHOS::HiviewDFX::HiSysEvent::Write<domain>(__FUNCTION__, __LINE__, \
            eventName, type, ##__VA_ARGS__); \
    } \
    hiSysEventWriteRet2023___; \
})
} // namespace HiviewDFX
} // namespace OHOS

#endif // __cplusplus
#endif // HI_SYS_EVENT_H
