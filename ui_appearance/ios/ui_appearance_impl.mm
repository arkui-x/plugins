/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugins/ui_appearance/ios/ui_appearance_impl.h"

#include <atomic>

#import <UIKit/UIKit.h>

namespace OHOS::Plugin {
namespace {
constexpr int32_t ALWAYS_DARK = 0;
constexpr int32_t ALWAYS_LIGHT = 1;
std::atomic<int32_t> g_darkMode { ALWAYS_LIGHT };
std::atomic<double> g_fontScale { 1.0 };

UITraitCollection* GetCurrentTraitCollection()
{
    UITraitCollection* traitCollection = [UIScreen mainScreen].traitCollection;
    NSArray<UIWindow*>* windows = [UIApplication sharedApplication].windows;
    for (UIWindow* window in windows) {
        if (window.isKeyWindow) {
            traitCollection = window.traitCollection;
            break;
        }
    }
    return traitCollection;
}

void UpdateAppearanceCacheOnMain()
{
    UITraitCollection* traitCollection = GetCurrentTraitCollection();
    if (@available(iOS 13.0, *)) {
        g_darkMode.store(
            traitCollection.userInterfaceStyle == UIUserInterfaceStyleDark ? ALWAYS_DARK : ALWAYS_LIGHT,
            std::memory_order_relaxed);
    } else {
        g_darkMode.store(ALWAYS_LIGHT, std::memory_order_relaxed);
    }

    if (@available(iOS 11.0, *)) {
        double fontScale = [[UIFontMetrics defaultMetrics] scaledValueForValue:1.0
                                                   compatibleWithTraitCollection:traitCollection];
        g_fontScale.store(fontScale, std::memory_order_relaxed);
    } else {
        g_fontScale.store(1.0, std::memory_order_relaxed);
    }
}

void RefreshAppearanceCache()
{
    if ([NSThread isMainThread]) {
        UpdateAppearanceCacheOnMain();
        return;
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        UpdateAppearanceCacheOnMain();
    });
}

void RegisterAppearanceObservers()
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        RefreshAppearanceCache();
        dispatch_async(dispatch_get_main_queue(), ^{
            NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
            [center addObserverForName:UIContentSizeCategoryDidChangeNotification
                                object:nil
                                 queue:[NSOperationQueue mainQueue]
                            usingBlock:^(__unused NSNotification* notification) {
                                UpdateAppearanceCacheOnMain();
                            }];
            [center addObserverForName:UIApplicationDidBecomeActiveNotification
                                object:nil
                                 queue:[NSOperationQueue mainQueue]
                            usingBlock:^(__unused NSNotification* notification) {
                                UpdateAppearanceCacheOnMain();
                            }];
        });
    });
}
}

std::unique_ptr<UiAppearance> UiAppearance::Create()
{
    RegisterAppearanceObservers();
    return std::make_unique<UiAppearanceImpl>();
}

int32_t UiAppearanceImpl::GetDarkMode()
{
    RefreshAppearanceCache();
    return g_darkMode.load(std::memory_order_relaxed);
}

double UiAppearanceImpl::GetFontScale()
{
    RefreshAppearanceCache();
    return g_fontScale.load(std::memory_order_relaxed);
}

} // namespace OHOS::Plugin
