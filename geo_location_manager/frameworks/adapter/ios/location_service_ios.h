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

#ifndef ARKUI_LOCATION_SERVICE_IOS_H
#define ARKUI_LOCATION_SERVICE_IOS_H
#ifdef __cplusplus
extern "C" {
#endif

void ArkUILocationServiceStart();
void ArkUILocationServiceStop();
int32_t ArkUIRequestSingleLocation(void* locatorCallbackHost);
void ArkUIRegisterCountryCodeObserver();
void ArkUIUnregisterCountryCodeObserver();
void ArkUISubscribeLocationError();
void ArkUIUnSubscribeLocationError();
void ArkUIAddLocationErrorCallback(void* callbackHost);
void ArkUIRemoveLocationErrorCallbackByHost(void* callbackHost);
void ArkUIRegisterSwitchCallback();
void ArkUIUnregisterSwitchCallback();
void ArkUIAddSwitchCallback(void* callbackHost);
void ArkUIRemoveSwitchCallbackByHost(void* callbackHost);
void ArkUIAddLocatorCallback(void* locatorCallbackHost);
void ArkUIClearLocatorCallbacks();
void ArkUIRemoveLocatorCallbackByHost(void* locatorCallbackHost);
void ArkUIAddCountryCodeCallback(void* callbackHost);
void ArkUIRemoveCountryCodeCallbackByHost(void* callbackHost);
void ArkUIAddBluetoothScanResultCallback(void* callbackHost);
void ArkUIRemoveBluetoothScanResultCallbackByHost(void* callbackHost);
void ArkUIConfigureLocation(int scenario, int priority, int timeInterval,
    double distanceInterval, float maxAccuracy);
#ifdef __cplusplus
}
#endif

#endif // ARKUI_LOCATION_SERVICE_IOS_H
