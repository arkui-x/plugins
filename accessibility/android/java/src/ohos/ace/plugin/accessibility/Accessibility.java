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

import android.accessibilityservice.AccessibilityServiceInfo;
import android.content.Context;
import android.util.Log;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityManager;

import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.ArrayList;
import java.util.HashSet;

/**
 * This class represents the Accessibility features in the application.
 *
 * @since 20
 */
public class Accessibility {
/**
 * AccessibilityAbilityInfo class for storing accessibility ability information.
 */
public static class AccessibilityAbilityInfo {
    /** Unique identifier for the accessibility service */
    public String id;

    /** Name of the accessibility service */
    public String name;

    /** Package name of the application containing the service */
    public String bundleName;

    /** List of package names that the service targets */
    public List<String> targetBundleNames;

    /** Types of accessibility abilities provided by the service */
    public List<String> abilityTypes;

    /** Capabilities supported by the accessibility service */
    public List<String> capabilities;

    /** Description of the accessibility service */
    public String description;

    /** Types of events that the service handles */
    public List<String> eventTypes;

    /** Flag indicating whether the service should be hidden */
    public boolean needHide;

    /** Label for the accessibility service */
    public String label;

    public AccessibilityAbilityInfo() {
        id = "";
        name = "";
        bundleName = "";
        targetBundleNames = new ArrayList<>();
        abilityTypes = new ArrayList<>();
        capabilities = new ArrayList<>();
        description = "";
        eventTypes = new ArrayList<>();
        label = "";
    }
}

private static final String LOG_TAG = "Accessibility";
private static final String ACCESSIBILITY_STATE_CHANGE_STRING = "accessibilityStateChange";
private Context mContext = null;
private AccessibilityManager accessibilityManager = null;
private AccessibilityManager.AccessibilityStateChangeListener stateChangeListener = null;
private AccessibilityManager.TouchExplorationStateChangeListener touchExplorationListener = null;

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
 * Gets the list of accessibility extension.
 *
 * @param abilityType the ability type
 * @param stateType   the state type
 * @return the list of accessibility ability info
 */
public List<AccessibilityAbilityInfo> getAccessibilityExtensionList(String abilityType, String stateType) {
    List<AccessibilityAbilityInfo> result = new ArrayList<>();
    getSystemService();
    if (accessibilityManager == null) {
        return result;
    }

    int feedbackType = mapAbilityTypeToFeedbackType(abilityType);
    if (feedbackType == 0) {
        Log.e(LOG_TAG, "Invalid abilityType: " + abilityType);
        return result;
    }

    List<AccessibilityServiceInfo> serviceList = getServiceListByState(stateType, feedbackType);
    if (serviceList == null) {
        Log.e(LOG_TAG, "Invalid serviceList");
        return result;
    }

    populateAccessibilityAbilityInfoList(result, serviceList);

    boolean isAllType = "all".equals(abilityType);
    if (!isAllType) {
        filterAccessibilityAbilityInfoList(result, abilityType);
    }
    return result;
}

private void filterAccessibilityAbilityInfoList(List<AccessibilityAbilityInfo> result, String abilityType) {
    Iterator<AccessibilityAbilityInfo> iterator = result.iterator();
    while (iterator.hasNext()) {
        AccessibilityAbilityInfo info = iterator.next();
        if (!info.abilityTypes.contains(abilityType)) {
            iterator.remove();
        }
    }
}

private List<AccessibilityServiceInfo> getServiceListByState(String stateType, int feedbackType) {
    switch (stateType) {
        case "enable":
            return accessibilityManager.getEnabledAccessibilityServiceList(feedbackType);
        case "disable":
            return getDisabledAccessibilityServices(feedbackType);
        case "install":
            return accessibilityManager.getInstalledAccessibilityServiceList();
        default:
            return new ArrayList<AccessibilityServiceInfo>();
    }
}

private List<AccessibilityServiceInfo> getDisabledAccessibilityServices(int feedbackType) {
    List<AccessibilityServiceInfo> allInstalled = accessibilityManager.getInstalledAccessibilityServiceList();
    List<AccessibilityServiceInfo> enabled = accessibilityManager.getEnabledAccessibilityServiceList(feedbackType);
    List<AccessibilityServiceInfo> disabledServices = new ArrayList<>();

    Set<String> enabledIds = new HashSet<>();
    for (AccessibilityServiceInfo info : enabled) {
        enabledIds.add(info.getId());
    }

    for (AccessibilityServiceInfo info : allInstalled) {
        if (!enabledIds.contains(info.getId())) {
            disabledServices.add(info);
        }
    }

    return disabledServices;
}

private void populateAccessibilityAbilityInfoList(List<AccessibilityAbilityInfo> result,
        List<AccessibilityServiceInfo> serviceList) {
    for (AccessibilityServiceInfo serviceInfo : serviceList) {
        AccessibilityAbilityInfo info = createAccessibilityAbilityInfo(serviceInfo);
        result.add(info);
    }
}

private AccessibilityAbilityInfo createAccessibilityAbilityInfo(AccessibilityServiceInfo serviceInfo) {
    AccessibilityAbilityInfo info = new AccessibilityAbilityInfo();
    info.id = serviceInfo.getId();
    info.name = serviceInfo.getSettingsActivityName();
    info.bundleName = serviceInfo.getResolveInfo().serviceInfo.packageName;
    info.description = serviceInfo.loadDescription(mContext.getPackageManager());

    extractAbilityTypes(info, serviceInfo);
    extractCapabilities(info, serviceInfo);
    extractEventTypes(info, serviceInfo);

    info.targetBundleNames = new ArrayList<>();
    if (serviceInfo.packageNames != null) {
        for (String packageName : serviceInfo.packageNames) {
            info.targetBundleNames.add(packageName);
        }
    }
    info.needHide = false;
    return info;
}

private void extractAbilityTypes(AccessibilityAbilityInfo info, AccessibilityServiceInfo serviceInfo) {
    info.abilityTypes = new ArrayList<>();
    int feedbackType = serviceInfo.feedbackType;

    if ((feedbackType & AccessibilityServiceInfo.FEEDBACK_AUDIBLE) != 0) {
        info.abilityTypes.add("audible");
    }
    if ((feedbackType & AccessibilityServiceInfo.FEEDBACK_SPOKEN) != 0) {
        info.abilityTypes.add("spoken");
    }
    if ((feedbackType & AccessibilityServiceInfo.FEEDBACK_VISUAL) != 0) {
        info.abilityTypes.add("visual");
    }
    if ((feedbackType & AccessibilityServiceInfo.FEEDBACK_HAPTIC) != 0) {
        info.abilityTypes.add("haptic");
    }
    if ((feedbackType & AccessibilityServiceInfo.FEEDBACK_GENERIC) != 0) {
        info.abilityTypes.add("generic");
    }
}

private void extractCapabilities(AccessibilityAbilityInfo info, AccessibilityServiceInfo serviceInfo) {
    info.capabilities = new ArrayList<>();
    int capabilities = serviceInfo.getCapabilities();

    if (capabilities != 0) {
        if ((capabilities & AccessibilityServiceInfo.CAPABILITY_CAN_RETRIEVE_WINDOW_CONTENT) != 0) {
            info.capabilities.add("retrieve");
        }
        if ((capabilities & AccessibilityServiceInfo.CAPABILITY_CAN_REQUEST_TOUCH_EXPLORATION) != 0) {
            info.capabilities.add("touchGuide");
        }
        if ((capabilities & AccessibilityServiceInfo.CAPABILITY_CAN_REQUEST_FILTER_KEY_EVENTS) != 0) {
            info.capabilities.add("keyEventObserver");
        }
        if ((capabilities & AccessibilityServiceInfo.CAPABILITY_CAN_CONTROL_MAGNIFICATION) != 0) {
            info.capabilities.add("zoom");
        }
        if ((capabilities & AccessibilityServiceInfo.CAPABILITY_CAN_PERFORM_GESTURES) != 0) {
            info.capabilities.add("gesture");
        }
    }
}

private void extractEventTypes(AccessibilityAbilityInfo info, AccessibilityServiceInfo serviceInfo) {
    info.eventTypes = new ArrayList<>();
    int eventTypes = serviceInfo.eventTypes;

    if (eventTypes != 0) {
        addEventTypeIfMatch(info.eventTypes, eventTypes, AccessibilityEvent.TYPE_VIEW_ACCESSIBILITY_FOCUSED,
                "accessibilityFocus");
        addEventTypeIfMatch(info.eventTypes, eventTypes, AccessibilityEvent.TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED,
                "accessibilityFocusClear");
        addEventTypeIfMatch(info.eventTypes, eventTypes, AccessibilityEvent.TYPE_VIEW_CLICKED, "click");
        addEventTypeIfMatch(info.eventTypes, eventTypes, AccessibilityEvent.TYPE_VIEW_LONG_CLICKED, "longClick");
        addEventTypeIfMatch(info.eventTypes, eventTypes, AccessibilityEvent.TYPE_VIEW_FOCUSED, "focus");
        addEventTypeIfMatch(info.eventTypes, eventTypes, AccessibilityEvent.TYPE_VIEW_SELECTED, "select");
        addEventTypeIfMatch(info.eventTypes, eventTypes, AccessibilityEvent.TYPE_VIEW_HOVER_ENTER, "hoverEnter");
        addEventTypeIfMatch(info.eventTypes, eventTypes, AccessibilityEvent.TYPE_VIEW_HOVER_EXIT, "hoverExit");
        addEventTypeIfMatch(info.eventTypes, eventTypes, AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED, "textUpdate");
        addEventTypeIfMatch(info.eventTypes, eventTypes, AccessibilityEvent.TYPE_VIEW_TEXT_SELECTION_CHANGED,
                "textSelectionUpdate");
        addEventTypeIfMatch(info.eventTypes, eventTypes, AccessibilityEvent.TYPE_VIEW_SCROLLED, "scroll");
    }
}

private void addEventTypeIfMatch(List<String> eventTypesList, int serviceEventTypes,
        int eventTypeFlag, String eventTypeString) {
    if ((serviceEventTypes & eventTypeFlag) != 0) {
        eventTypesList.add(eventTypeString);
    }
}

private int mapAbilityTypeToFeedbackType(String abilityType) {
    switch (abilityType) {
        case "audible":
            return AccessibilityServiceInfo.FEEDBACK_AUDIBLE;
        case "spoken":
            return AccessibilityServiceInfo.FEEDBACK_SPOKEN;
        case "visual":
            return AccessibilityServiceInfo.FEEDBACK_VISUAL;
        case "haptic":
            return AccessibilityServiceInfo.FEEDBACK_HAPTIC;
        case "generic":
            return AccessibilityServiceInfo.FEEDBACK_GENERIC;
        case "all":
            return AccessibilityServiceInfo.FEEDBACK_ALL_MASK;
        default:
            return 0;
    }
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
                onAccessibilityStateChangedCallback(isEnable, ACCESSIBILITY_STATE_CHANGE_STRING);
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

/**
 * Get TouchExploration status
 *
 * @return boolean
 */
public boolean isTouchExplorationEnabled() {
    getSystemService();
    if (this.accessibilityManager != null) {
        return this.accessibilityManager.isTouchExplorationEnabled();
    }
    return false;
}

/**
 * Registers an touchExplorationListener to listen for changes in touchExploration state.
 */
public void registerTouchExplorationListener() {
    if (this.touchExplorationListener == null) {
        this.touchExplorationListener = new AccessibilityManager.TouchExplorationStateChangeListener() {
            @Override
            public void onTouchExplorationStateChanged(boolean enabled) {
                onAccessibilityStateChangedCallback(enabled, "");
            }
        };
    }
    getSystemService();
    if (this.accessibilityManager != null && this.touchExplorationListener != null) {
        this.accessibilityManager.addTouchExplorationStateChangeListener(this.touchExplorationListener);
    }
}

/**
 * Unregisters the touchExplorationListener listener if it is currently registered.
 */
public void unRegisterTouchExplorationListener() {
    getSystemService();
    if (this.accessibilityManager != null && this.touchExplorationListener != null) {
        this.accessibilityManager.removeTouchExplorationStateChangeListener(this.touchExplorationListener);
        this.touchExplorationListener = null;
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
 * @param eventName the name of the event that triggered the callback
 */
protected native void onAccessibilityStateChangedCallback(boolean isEnable, String eventName);
}
