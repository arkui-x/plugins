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

package ohos.ace.plugin.vibratorplugin;

import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.util.Log;
import android.os.Vibrator;
import android.os.VibrationEffect;
import android.media.AudioManager;
import android.app.NotificationManager;
import android.app.Activity;
import android.Manifest;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;
import java.util.Locale;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Field;
import java.lang.reflect.Method;

/**
 * Vibrator android plugin module
 *
 * @since 2025-10
 */
public class VibratorPlugin {
    private static final String LOG_TAG = "VibratorPlugin";
    private static final String LANGUAGE_SHARE_PREFERENCE = "language_prefs";
    private static final String KEY_LANGUAGE = "app_language";
    private static final int EFFECT_TICK_DURATION = 50;
    private static final int EFFECT_CLICK_DURATION = 100;
    private static final int EFFECT_HEAVY_DURATION = 200;
    private static final int DEFAULT_VIBRATOR_ID = 0;
    private static final int API_31 = 31;
    private static final int REQUEST_VIBRATE_PERMISSION = 1001;
    private static final int PERMISSION_DENIED = 201;
    private static final int PARAMETER_ERROR = 401;
    private static final int IS_NOT_SUPPORTED = 801;
    private static final int VIBRATE_SUCCESS = 0;

    private Context mContext;
    private Vibrator mVibrator;
    private Object mVibratorManager;
    private AudioManager mAudioManager;
    private NotificationManager mNotificationManager;
    private VibrateMode mMode = VibrateMode.DEFAULT;

    private native void nativeGetEffectInfo(int duration, boolean isSupportEffect);
    private native void nativeGetMultiVibratorInfo(List<List<Object>> vibratorInfos);

    /**
     * VibrateMode
     */
    public enum VibrateMode {
        TIME,
        PRESET,
        PATTERN,
        DEFAULT
    }

    /**
     * VibratorUsage
     */
    public enum VibratorUsage {
        USAGE_UNKNOWN(0),
        USAGE_ALARM(1),
        USAGE_RING(2),
        USAGE_NOTIFICATION(3),
        USAGE_COMMUNICATION(4),
        USAGE_TOUCH(5),
        USAGE_MEDIA(6),
        USAGE_PHYSICAL_FEEDBACK(7),
        USAGE_SIMULATE_REALITY(8),
        USAGE_MAX(9);

        private final int value;

        VibratorUsage(int value) {
            this.value = value;
        }

        /**
         * getValue
         */
        public int getValue() {
            return value;
        }

        /**
         * fromInt
         */
        public static VibratorUsage fromInt(int value) {
            for (VibratorUsage usage : values()) {
                if (usage.value == value) {
                    return usage;
                }
            }
            return USAGE_UNKNOWN;
        }
    }

    /**
     * Determines if vibration is allowed based on the given UsageKey parameters
     *
     * @param isHapticSupport Whether haptic feedback is supported
     * @param isRingerEnabled Whether ringer is enabled
     * @param usage The VibratorUsage type
     * @return True if vibration is allowed, false otherwise
     */
    public static boolean isSupportedVibrate(boolean isHapticSupport, boolean isRingerEnabled, VibratorUsage usage) {
        if (isHapticSupport && isRingerEnabled) {
            switch (usage) {
                case USAGE_UNKNOWN:
                case USAGE_ALARM:
                case USAGE_RING:
                case USAGE_NOTIFICATION:
                case USAGE_COMMUNICATION:
                case USAGE_TOUCH:
                case USAGE_MEDIA:
                case USAGE_PHYSICAL_FEEDBACK:
                case USAGE_SIMULATE_REALITY:
                    return true;
                default:
                    return false;
            }
        } else {
            switch (usage) {
                case USAGE_UNKNOWN:
                case USAGE_TOUCH:
                case USAGE_MEDIA:
                case USAGE_PHYSICAL_FEEDBACK:
                case USAGE_SIMULATE_REALITY:
                    return true;
                case USAGE_ALARM:
                case USAGE_RING:
                case USAGE_NOTIFICATION:
                case USAGE_COMMUNICATION:
                    return false;
                default:
                    return false;
            }
        }
    }

    /**
     * getVibratorService
     */
    public void getVibratorService() {
        mVibrator = (Vibrator) mContext.getSystemService(Context.VIBRATOR_SERVICE);
    }

    /**
     * VibratorPlugin
     */
    public VibratorPlugin(Context context) {
        mContext = context;
        mAudioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        mNotificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        init();
        if (Build.VERSION.SDK_INT >= API_31) {
            try {
                Class<?> vibratorManagerClass = Class.forName("android.os.VibratorManager");
                String managerType = vibratorManagerClass.getName();
                Log.i(LOG_TAG, "vibratorManagerClass Type: " + managerType);

                Method getSystemServiceMethod = Context.class.getMethod(
                    "getSystemService",
                    Class.class
                );
                Log.i(LOG_TAG, "getSystemServiceMethod success");
                mVibratorManager = getSystemServiceMethod.invoke(
                    mContext,
                    vibratorManagerClass
                );
                Log.i(LOG_TAG, "mVibratorManager success");
            } catch (SecurityException | ClassNotFoundException | InvocationTargetException |
                        NoSuchMethodException | IllegalAccessException e) {
                Log.e(LOG_TAG, "VibratorManager init failed: " + e.getMessage());
                mVibratorManager = null;
            }
        }
    }

    /**
     * isHapticFeedbackEnabled
     */
    public boolean isHapticFeedbackEnabled() {
        if (mNotificationManager == null) {
            return true;
        }
        int interruptionFilter = mNotificationManager.getCurrentInterruptionFilter();
        if (interruptionFilter == NotificationManager.INTERRUPTION_FILTER_NONE ||
            interruptionFilter == NotificationManager.INTERRUPTION_FILTER_PRIORITY ||
            interruptionFilter == NotificationManager.INTERRUPTION_FILTER_UNKNOWN) {
            return false;
        } else {
            return true;
        }
    }

    /**
     * getVibrationMode
     */
    public boolean getVibrationMode() {
        if (mAudioManager == null) {
            return true;
        }
        int ringerMode = mAudioManager.getRingerMode();
        switch (ringerMode) {
            case AudioManager.RINGER_MODE_SILENT:
                return false;
            case AudioManager.RINGER_MODE_VIBRATE:
                return true;
            case AudioManager.RINGER_MODE_NORMAL:
                return true;
            default:
                return true;
        }
    }

    /**
     * vibrate
     *
     * @param duration Indicates the duration that the one-shot vibration lasts, in milliseconds.
     */
    public int vibrate(long duration, int usage) {
        if (mContext.checkCallingOrSelfPermission(android.Manifest.permission.VIBRATE)
                != PackageManager.PERMISSION_GRANTED) {
            Log.e(LOG_TAG, "vibrate: Vibration permission not granted");
            return PERMISSION_DENIED;
        }
        if (mVibrator == null) {
            Log.i(LOG_TAG, "vibrate: init Vibrator service");
            getVibratorService();
        }
        if (!mVibrator.hasVibrator()) {
            Log.e(LOG_TAG, "vibrate: Device does not have a vibrator");
            return IS_NOT_SUPPORTED;
        }
        try {
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                boolean isHapticSupport = isHapticFeedbackEnabled();
                boolean isRingerEnabled = getVibrationMode();
                VibratorUsage enumUsage = VibratorUsage.fromInt(usage);
                if (isSupportedVibrate(isHapticSupport, isRingerEnabled, enumUsage)) {
                    VibrationEffect effect = VibrationEffect.createOneShot(duration,
                                                                            VibrationEffect.DEFAULT_AMPLITUDE);
                    mMode = VibrateMode.TIME;
                    mVibrator.vibrate(effect);
                    Log.i(LOG_TAG, "vibrate: Vibrator is success");
                } else {
                    Log.e(LOG_TAG, "vibrate: Vibrator is error");
                    return IS_NOT_SUPPORTED;
                }
            }
            return VIBRATE_SUCCESS;
        } catch (SecurityException e) {
            Log.e(LOG_TAG, "vibrate: Local vibration failed: " + e.getMessage());
            return IS_NOT_SUPPORTED;
        }
    }

    /**
     * playVibratorEffect
     *
     * @param effect effect
     * @param loopCount loop count
     * @param usage usage
     * @return Operation result, 0 for success, other values indicate errors.
     */
    public int playVibratorEffect(String effect, int loopCount, int usage) {
        if (mContext.checkCallingOrSelfPermission(android.Manifest.permission.VIBRATE)
                != PackageManager.PERMISSION_GRANTED) {
            Log.e(LOG_TAG, "vibrate: Vibration permission not granted");
            return PERMISSION_DENIED;
        }
        if (mVibrator == null) {
            Log.i(LOG_TAG, "vibrate: init Vibrator service");
            getVibratorService();
        }
        if (!mVibrator.hasVibrator()) {
            Log.e(LOG_TAG, "vibrate: Device does not have a vibrator");
            return IS_NOT_SUPPORTED;
        }
        try {
            int amplitude = 0;
            int loopCountTemp = loopCount;
            long duration = 0L;
            if (isSupportedEffect(effect)) {
                amplitude = getAmplitude(effect);
                duration = getDuration(effect);
            } else {
                return IS_NOT_SUPPORTED;
            }
            if (effect.equals("haptic.notice.success") || effect.equals("haptic.notice.fail")) {
                loopCountTemp *= 2;
                duration /= 2;
            }
            if (effect.equals("haptic.notice.warning")) {
                loopCountTemp *= 3;
                duration /= 3;
            }
            int arraySize = loopCountTemp * 2;
            long[] durationParam = new long[arraySize];
            int[] amplitudeParam = new int[arraySize];
            for (int i = 0; i < arraySize; i += 2) {
                durationParam[i] = duration;
                amplitudeParam[i] = amplitude;
                durationParam[i + 1] = duration;
                amplitudeParam[i + 1] = 0;
            }
            VibrationEffect effectObject = VibrationEffect.createWaveform(durationParam, amplitudeParam, -1);
            mMode = VibrateMode.PRESET;
            mVibrator.vibrate(effectObject);
            Log.i(LOG_TAG, "playVibratorEffect: Vibrator is success");
            return VIBRATE_SUCCESS;
        } catch (SecurityException e) {
            Log.e(LOG_TAG, "SecurityException when playVibratorEffect: " + e.getMessage());
            return IS_NOT_SUPPORTED;
        }
    }

    /**
     * stopVibrator
     *
     * @return Operation result, 0 for success, other values indicate errors.
     */
    public int stopVibrator(int vibratorId) {
        if (mContext.checkCallingOrSelfPermission(android.Manifest.permission.VIBRATE)
                != PackageManager.PERMISSION_GRANTED) {
            Log.e(LOG_TAG, "stopVibrator: Vibration permission not granted");
            return PERMISSION_DENIED;
        }
        if (mVibrator == null) {
            Log.i(LOG_TAG, "vibrate: init Vibrator service");
            getVibratorService();
        }
        if (!mVibrator.hasVibrator()) {
            Log.e(LOG_TAG, "vibrate: Device does not have a vibrator");
            return IS_NOT_SUPPORTED;
        }

        try {
            if (Build.VERSION.SDK_INT >= API_31) {
                Class<?> vibratorManagerClass = Class.forName("android.os.VibratorManager");
                String managerType = vibratorManagerClass.getName();
                Log.i(LOG_TAG, "mVibratorManager Type: " + managerType);
                Method getVibratorMethod = vibratorManagerClass.getMethod("getVibrator", int.class);
                Object fieldValue = getVibratorMethod.invoke(mVibratorManager, vibratorId);
                if (fieldValue == null || !(fieldValue instanceof Vibrator)) {
                    return IS_NOT_SUPPORTED;
                }
                Vibrator vibrator = (Vibrator) fieldValue;
                Log.i(LOG_TAG, "getVibrator invoke success: " + vibratorId);
                if (vibrator != null) {
                    vibrator.cancel();
                }
                mVibrator.cancel();
            } else {
                mVibrator.cancel();
                Log.i(LOG_TAG, " stopVibrator: is success");
            }
            return VIBRATE_SUCCESS;
        } catch (SecurityException | ClassNotFoundException | InvocationTargetException |
                 NoSuchMethodException | IllegalAccessException e) {
            Log.e(LOG_TAG, "stopVibrator: SecurityException when stopping vibrator: " + e.getMessage());
            return IS_NOT_SUPPORTED;
        }
    }

    /**
     * stopVibratorByMode
     *
     * @return Operation result, 0 for success, other values indicate errors.
     */
    public int stopVibratorByMode(String mode) {
        if (mContext.checkCallingOrSelfPermission(android.Manifest.permission.VIBRATE)
                != PackageManager.PERMISSION_GRANTED) {
            Log.e(LOG_TAG, "vibrate: Vibration permission not granted");
            return PERMISSION_DENIED;
        }
        String lowerMode = mode.toLowerCase(Locale.ROOT).trim();
        if ((lowerMode.equals("time") && mMode == VibrateMode.TIME) ||
            (lowerMode.equals("preset") && mMode == VibrateMode.PRESET) ||
            (lowerMode.equals("pattern") && mMode == VibrateMode.PATTERN)) {
            mMode = VibrateMode.DEFAULT;
            return stopVibrator(DEFAULT_VIBRATOR_ID);
        }
        return VIBRATE_SUCCESS;
    }

    /**
     * isSupportEffect
     *
     * @return Operation result, 0 for success, other values indicate errors.
     */
    public int isSupportEffect(String effect, boolean[] state) {
        if (mContext.checkCallingOrSelfPermission(android.Manifest.permission.VIBRATE)
                != PackageManager.PERMISSION_GRANTED) {
            Log.e(LOG_TAG, "vibrate: Vibration permission not granted");
            return PERMISSION_DENIED;
        }
        if (mVibrator == null) {
            Log.i(LOG_TAG, "vibrate: init Vibrator service");
            getVibratorService();
        }
        if (!mVibrator.hasVibrator()) {
            Log.e(LOG_TAG, "vibrate: Device does not have a vibrator");
            return IS_NOT_SUPPORTED;
        }
        state[0] = isSupportedEffect(effect);
        return VIBRATE_SUCCESS;
    }

    /**
     * playPattern
     *
     * @return Operation result, 0 for success, other values indicate errors.
     */
    public int playPattern(long[] timings, int[] amplitudes, int repeat, int usage) {
        Log.i(LOG_TAG, "1Java playPattern called");

        if (mContext.checkCallingOrSelfPermission(android.Manifest.permission.VIBRATE)
                != PackageManager.PERMISSION_GRANTED) {
            Log.e(LOG_TAG, "vibrate: Vibration permission not granted");
            return PERMISSION_DENIED;
        }
        if (mVibrator == null) {
            Log.i(LOG_TAG, "vibrate: init Vibrator service");
            getVibratorService();
        }
        if (!mVibrator.hasVibrator()) {
            Log.e(LOG_TAG, "vibrate: Device does not have a vibrator");
            return IS_NOT_SUPPORTED;
        }
        try {
            boolean isHapticSupport = isHapticFeedbackEnabled();
            boolean isRingerEnabled = getVibrationMode();
            VibratorUsage enumUsage = VibratorUsage.fromInt(usage);
            if (isSupportedVibrate(isHapticSupport, isRingerEnabled, enumUsage)) {
                VibrationEffect effect;
                if (amplitudes != null) {
                    effect = VibrationEffect.createWaveform(timings, amplitudes, repeat);
                } else {
                    effect = VibrationEffect.createWaveform(timings, repeat);
                }
                mVibrator.vibrate(effect);
                mMode = VibrateMode.PATTERN;
            } else {
                Log.e(LOG_TAG, "playPattern: playPattern is error");
                return IS_NOT_SUPPORTED;
            }
            return VIBRATE_SUCCESS;
        } catch (SecurityException e) {
            Log.e(LOG_TAG, "SecurityException when playPattern: " + e.getMessage());
            return IS_NOT_SUPPORTED;
        }
    }

    /**
     * Gets the vibrator list.
     *
     * @return Operation result, 0 for success, other values indicate errors.
     * @throws SecurityException Thrown when vibration permission is missing.
     * @throws ClassNotFoundException Thrown if VibratorManager class is not found.
     * @throws NoSuchMethodException Thrown if getVibratorIds method is not found.
     * @throws IllegalAccessException Thrown when accessing method illegally.
     * @throws InvocationTargetException Thrown when method invocation fails.
     */
    public int getVibratorList() {
        if (mVibrator == null) {
            return IS_NOT_SUPPORTED;
        }
        int vibratorId = -1;
        if (Build.VERSION.SDK_INT >= API_31) {
            try {
                Class<?> vibratorManagerClass = Class.forName("android.os.VibratorManager");
                Method getIdsMethod = vibratorManagerClass.getMethod("getVibratorIds");
                int[] ids = (int[]) getIdsMethod.invoke(mVibratorManager);
                if (ids.length > 0) {
                    vibratorId = ids[0];
                }
            } catch (SecurityException | ClassNotFoundException | InvocationTargetException |
                 NoSuchMethodException | IllegalAccessException e) {
                Log.e(LOG_TAG, "getVibratorList: Exception when stopping vibrator: " + e.getMessage());
                return IS_NOT_SUPPORTED;
            }
        }
        int deviceId = -1;
        String deviceName = "";
        boolean isSupportHdHaptic = true;
        boolean isLocalVibrator = true;
        int position = 0;
        List<List<Object>> vibratorInfo = new ArrayList<>();
        List<Object> infoRow = new ArrayList<>();
        infoRow.add(deviceId);
        infoRow.add(vibratorId);
        infoRow.add(deviceName);
        infoRow.add(isSupportHdHaptic);
        infoRow.add(isLocalVibrator);
        infoRow.add(position);
        vibratorInfo.add(infoRow);
        nativeGetMultiVibratorInfo(vibratorInfo);
        return VIBRATE_SUCCESS;
    }

    /**
     * getEffectInfo
     *
     * @param vibratorUsage vibrator usage
     * @return Operation result, 0 for success, other values indicate errors.
     */
    public int getEffectInfo(String vibratorUsage) {
        if (mVibrator == null || !mVibrator.hasVibrator()) {
            return IS_NOT_SUPPORTED;
        }
        int duration = (int) getDuration(vibratorUsage);
        boolean isSupported = isSupportedEffect(vibratorUsage);
        nativeGetEffectInfo(duration, isSupported);
        return VIBRATE_SUCCESS;
    }

    /**
     * getVibratorCapacity
     *
     * @return isHdHapticSupport
     */
    public boolean getVibratorCapacity() {
        Log.i(LOG_TAG, "1Java getVibratorCapacity called");
        if (mVibrator == null || !mVibrator.hasVibrator()) {
            return false;
        }
        return true;
    }

    /**
     * Get vibration duration based on effectId
     *
     * @param effectId The effect ID
     * @return Vibration duration in milliseconds, returns 0 by default
     */
    public static long getDuration(String effectId) {
        switch (effectId) {
            case "haptic.clock.timer":
                return 300L;
            case "haptic.effect.soft":
                return 200L;
            case "haptic.effect.hard":
                return 250L;
            case "haptic.effect.sharp":
                return 150L;
            case "haptic.notice.success":
            case "haptic.notice.fail":
                return 400L;
            case "haptic.notice.warning":
                return 500L;
            default:
                return 0L;
        }
    }

    /**
     * Get enable status based on effectId
     *
     * @param effectId The effect ID
     * @return Whether it is enabled, returns false by default
     */
    public static boolean isSupportedEffect(String effectId) {
        switch (effectId) {
            case "haptic.clock.timer":
            case "haptic.effect.soft":
            case "haptic.effect.hard":
            case "haptic.effect.sharp":
            case "haptic.notice.success":
            case "haptic.notice.fail":
            case "haptic.notice.warning":
                return true;
            default:
                return false;
        }
    }

    /**
     * Get intensity value based on effectId
     *
     * @param effectId The effect ID
     * @return Intensity value, returns 0 by default
     */
    public static int getAmplitude(String effectId) {
        switch (effectId) {
            case "haptic.clock.timer":
                return 204;
            case "haptic.effect.soft":
            case "haptic.effect.sharp":
                return 128;
            case "haptic.effect.hard":
                return 153;
            case "haptic.notice.success":
                return 204;
            case "haptic.notice.fail":
            case "haptic.notice.warning":
                return 255;
            default:
                return 0;
        }
    }

    private Activity getActivity() {
        try {
            Class activityThreadClass = Class.forName("android.app.ActivityThread");
            Object activityThread = activityThreadClass.getMethod("currentActivityThread").invoke(null);
            Field mActivities = activityThreadClass.getDeclaredField("mActivities");
            mActivities.setAccessible(true);
            Object fieldValue = mActivities.get(activityThread);
            if (fieldValue == null || !(fieldValue instanceof Map)) {
                return null;
            }
            Map activitiesMap = (Map) fieldValue;
            for (Object activityClientRecord : activitiesMap.values()) {
                Class activityClientRecordClass = activityClientRecord.getClass();
                Field paused = activityClientRecordClass.getDeclaredField("paused");
                paused.setAccessible(true);
                if (paused.getBoolean(activityClientRecord)) {
                    return null;
                }
                Field activityField = activityClientRecordClass.getDeclaredField("activity");
                activityField.setAccessible(true);
                Object activityFieldValue = activityField.get(activityClientRecord);
                if (activityFieldValue != null && activityFieldValue instanceof Activity) {
                    return (Activity) activityFieldValue;
                }
                return null;
            }
        } catch (ClassNotFoundException ex) {
            Log.e(LOG_TAG, "getActivity failed: " + ex.getMessage());
        } catch (InvocationTargetException ex) {
            Log.e(LOG_TAG, "getActivity failed: " + ex.getMessage());
        } catch (NoSuchMethodException ex) {
            Log.e(LOG_TAG, "getActivity failed: " + ex.getMessage());
        } catch (NoSuchFieldException ex) {
            Log.e(LOG_TAG, "getActivity failed: " + ex.getMessage());
        } catch (IllegalAccessException ex) {
            Log.e(LOG_TAG, "getActivity failed: " + ex.getMessage());
        }
        return null;
    }

    /**
     * nativeInit
     */
    protected native void nativeInit();

    private void init() {
        nativeInit();
    }
}
