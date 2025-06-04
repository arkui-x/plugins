/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

package ohos.ace.plugin.accessibility;

import android.content.Context;
import android.view.accessibility.AccessibilityManager;

/**
 * This class represents the Accessibility features in the application.
 *
 * @since 20
 */
public class Accessibility {
private static final String LOG_TAG = "Accessibility";
private Context mContext = null;
private AccessibilityManager accessibilityManager = null;
private AccessibilityManager.AccessibilityStateChangeListener stateChangeListener = null;

/**
 * Constructor for Accessibility class.
 *
 * @param context the application context
 */
public Accessibility(Context context) {
    this.mContext = context;
    nativeInit();
}

/**
 * Get accessibility status
 *
 * @return boolean
 */
public boolean isAccessibilityEnabled() {
    getSystemService();
    if (this.accessibilityManager != null) {
        return this.accessibilityManager.isEnabled();
    }
    return false;
}

/**
 * Registers an AccessibilityStateListener to listen for changes in accessibility state.
 */
public void registerAccessibilityStateListener() {
    if (this.stateChangeListener == null) {
        this.stateChangeListener = new AccessibilityManager.AccessibilityStateChangeListener() {
            @Override
            public void onAccessibilityStateChanged(boolean isEnable) {
                onAccessibilityStateChangedCallback(isEnable);
            }
        };
    }
    getSystemService();
    if (this.accessibilityManager != null && this.stateChangeListener != null) {
        this.accessibilityManager.addAccessibilityStateChangeListener(this.stateChangeListener);
    }
}

/**
 * Unregisters the accessibility state listener if it is currently registered.
 */
public void unRegisterAccessibilityStateListener() {
    getSystemService();
    if (this.accessibilityManager != null && this.stateChangeListener != null) {
        this.accessibilityManager.removeAccessibilityStateChangeListener(this.stateChangeListener);
        this.stateChangeListener = null;
    }
}

private void getSystemService() {
    if (this.accessibilityManager == null) {
        this.accessibilityManager =
            (AccessibilityManager) this.mContext.getSystemService(Context.ACCESSIBILITY_SERVICE);
    }
}

/**
 * nativeInit
 */
public native void nativeInit();

/**
 * This method is called when the accessibility state changes.
 *
 * @param isEnable a boolean indicating whether accessibility is enabled or disabled
 */
protected native void onAccessibilityStateChangedCallback(boolean isEnable);
}
