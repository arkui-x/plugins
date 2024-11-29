
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

#include "bluetooth_a2dp_impl.h"

#import <AVFAudio/AVFAudio.h>
#import <AVFoundation/AVFoundation.h>
#include <thread>

#import "BluetoothA2dp.h"
#import "BluetoothCentralManager.h"
#include "bluetooth_a2dp_src.h"
#include "bluetooth_def.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"

#define A2DP_IMPL_PAUSE_TIME 50
#define A2DP_IMPL_PAUSE_COUNT 8

using namespace OHOS::bluetooth;
namespace OHOS {
namespace Bluetooth {
BluetoothA2DPImpl::BluetoothA2DPImpl() {}

BluetoothA2DPImpl::~BluetoothA2DPImpl() {}

int BluetoothA2DPImpl::Connect(const RawAddress& device)
{
    return BT_NO_ERROR;
}

int BluetoothA2DPImpl::Disconnect(const RawAddress& device)
{
    return BT_NO_ERROR;
}

void BluetoothA2DPImpl::RegisterObserver(const sptr<IBluetoothA2dpSourceObserver>& observer)
{
    [BluetoothCentralManager sharedInstance];
    std::lock_guard<std::mutex> lock(observerMutex_);
    observer_ = observer;
    BluetoothA2dp* a2dp = [BluetoothA2dp sharedInstance];
    [a2dp setNotifyRouteChange];
    a2dp.connectStateChangeBlock = ^(NSString* strDeviceId, int32_t state) {
      if (!observer_) {
          return;
      }
      OHOS::bluetooth::RawAddress rawAdd(strDeviceId.UTF8String);
      switch (state) {
          case AVAudioSessionRouteChangeReasonNewDeviceAvailable:
              observer_->OnConnectionStateChanged(rawAdd, static_cast<int>(BTConnectState::CONNECTED),
                  static_cast<int>(ConnChangeCause::CONNECT_CHANGE_COMMON_CAUSE));
              break;
          case AVAudioSessionRouteChangeReasonOldDeviceUnavailable:
              observer_->OnConnectionStateChanged(rawAdd, static_cast<int>(BTConnectState::DISCONNECTED),
                  static_cast<int>(ConnChangeCause::CONNECT_CHANGE_COMMON_CAUSE));
              break;
          default:
              break;
      }
    };
}

void BluetoothA2DPImpl::DeregisterObserver(const sptr<IBluetoothA2dpSourceObserver>& observer)
{
    [[BluetoothA2dp sharedInstance] closeNotifyRouteChange];
}

int BluetoothA2DPImpl::GetDevicesByStates(const std::vector<int32_t>& states, std::vector<RawAddress>& rawAddrs)
{
    NSArray* arrAddrs = [BluetoothA2dp GetDevicesByStates];
    for (NSString* strAddrs in arrAddrs) {
        OHOS::bluetooth::RawAddress rawAdd(strAddrs.UTF8String);
        rawAddrs.push_back(rawAdd);
    }
    return BT_NO_ERROR;
}

int BluetoothA2DPImpl::GetDeviceState(const RawAddress& device, int& state)
{
    NSString* strDeviceId = [NSString stringWithFormat:@"%s", device.GetAddress().c_str()];
    state = [BluetoothA2dp GetDeviceState:strDeviceId];
    return BT_NO_ERROR;
}

int BluetoothA2DPImpl::GetPlayingState(const RawAddress& device, int& state)
{
    return BT_NO_ERROR;
}

int BluetoothA2DPImpl::SetConnectStrategy(const RawAddress& device, int32_t strategy)
{
    return BT_NO_ERROR;
}

int BluetoothA2DPImpl::GetConnectStrategy(const RawAddress& device, int& strategy)
{
    return BT_NO_ERROR;
}

int BluetoothA2DPImpl::SetActiveSinkDevice(const RawAddress& device)
{
    return BT_NO_ERROR;
}

RawAddress BluetoothA2DPImpl::GetActiveSinkDevice()
{
    RawAddress ret;
    return ret;
}

BluetoothA2dpCodecStatus BluetoothA2DPImpl::GetCodecStatus(const RawAddress& device)
{
    BluetoothA2dpCodecStatus ret;
    return ret;
}

int BluetoothA2DPImpl::GetCodecPreference(const RawAddress& device, BluetoothA2dpCodecInfo& info)
{
    return BT_NO_ERROR;
}

int BluetoothA2DPImpl::SetCodecPreference(const RawAddress& device, const BluetoothA2dpCodecInfo& info)
{
    return BT_NO_ERROR;
}

void BluetoothA2DPImpl::SwitchOptionalCodecs(const RawAddress& device, bool isEnable) {}

int BluetoothA2DPImpl::GetOptionalCodecsSupportState(const RawAddress& device)
{
    return BT_NO_ERROR;
}

int BluetoothA2DPImpl::StartPlaying(const RawAddress& device)
{
    return BT_NO_ERROR;
}

int BluetoothA2DPImpl::SuspendPlaying(const RawAddress& device)
{
    return BT_NO_ERROR;
}

int BluetoothA2DPImpl::StopPlaying(const RawAddress& device)
{
    return BT_NO_ERROR;
}

int BluetoothA2DPImpl::WriteFrame(const uint8_t* data, uint32_t size)
{
    return BT_NO_ERROR;
}

int BluetoothA2DPImpl::GetRenderPosition(
    const RawAddress& device, uint32_t& delayValue, uint64_t& sendDataSize, uint32_t& timeStamp)
{
    return BT_NO_ERROR;
}

int BluetoothA2DPImpl::OffloadStartPlaying(const RawAddress& device, const std::vector<int32_t>& sessionsId)
{
    return BT_NO_ERROR;
}

int BluetoothA2DPImpl::OffloadStopPlaying(const RawAddress& device, const std::vector<int32_t>& sessionsId)
{
    return BT_NO_ERROR;
}

int BluetoothA2DPImpl::A2dpOffloadSessionPathRequest(
    const RawAddress& device, const std::vector<BluetoothA2dpStreamInfo>& info)
{
    return BT_NO_ERROR;
}

BluetoothA2dpOffloadCodecStatus BluetoothA2DPImpl::GetOffloadCodecStatus(const RawAddress& device)
{
    BluetoothA2dpOffloadCodecStatus ret;
    return ret;
}

int BluetoothA2DPImpl::EnableAutoPlay(const RawAddress& device)
{
    return BT_NO_ERROR;
}

int BluetoothA2DPImpl::DisableAutoPlay(const RawAddress& device, const int duration)
{
    return BT_NO_ERROR;
}

void BluetoothA2DPImpl::GetVirtualDeviceList(std::vector<std::string>& devices) {}

void BluetoothA2DPImpl::UpdateVirtualDevice(int32_t action, const std::string& address) {}

int BluetoothA2DPImpl::GetAutoPlayDisabledDuration(const RawAddress& device, int& duration)
{
    return BT_NO_ERROR;
}
} // namespace Bluetooth
} // namespace OHOS
