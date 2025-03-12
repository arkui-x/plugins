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

package ohos.ace.plugin.audiomanager;

import android.content.Context;
import android.media.AudioDeviceCallback;
import android.media.AudioDeviceInfo;
import android.media.AudioManager;
import android.media.AudioManager.AudioRecordingCallback;
import android.media.AudioPlaybackConfiguration;
import android.media.AudioRecordingConfiguration;
import android.util.Log;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.List;

/**
 * AudioManagerPlugin
 *
 * @since 2024-06-24
 */
public class AudioManagerPlugin {
    private static final String LOG_TAG = "AudioManagerPlugin";
    private AudioManager audioManager_;
    private AudioDeviceChangeCallbackImpl deviceChangeCallback_;
    private OnAudioRecordingCallbackListenerImpl audioRecordingCallbacklListeners_;
    private OnAudioPlaybackCallbackListenerImpl audioPlaybackCallbacklListeners_;

    /**
     * AudioManagerPlugin
     *
     * @param context context of the application
     */
    public AudioManagerPlugin(Context context) {
        if (context == null) {
            Log.e(LOG_TAG, "context is null");
            return;
        }
        audioManager_ = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        nativeInit();
    }

    private static Object invoke(String className, String methodName, Object obj, Class<?>[] parameterTypes,
                                 Object... args) {
        Object value = null;

        try {
            Class<?> clz = Class.forName(className);
            Method method = clz.getDeclaredMethod(methodName, parameterTypes);
            value = method.invoke(obj, args);
        } catch (NoSuchMethodException | IllegalAccessException | IllegalArgumentException | InvocationTargetException
                | SecurityException | ClassNotFoundException e) {
            Log.e(LOG_TAG, "invoke " + e.getClass().getSimpleName());
        } catch (Exception e) {
            Log.e(LOG_TAG, "unknown Exception in invoke");
            e.printStackTrace();
        }
        return value;
    }

    /**
     * getVolume get volume of the stream
     *
     * @param streamType stream type of the audio
     * @return volume of the stream
     */
    public int getVolume(int streamType) {
        if (audioManager_ == null) {
            Log.e(LOG_TAG, "audioManager_ is null");
            return -1;
        }
        return audioManager_.getStreamVolume(streamType);
    }

    /**
     * getMaxVolume get max volume of the stream
     *
     * @param streamType stream type of the audio
     * @return max volume of the stream
     */
    public int getMaxVolume(int streamType) {
        if (audioManager_ == null) {
            Log.e(LOG_TAG, "audioManager_ is null");
            return -1;
        }
        return audioManager_.getStreamMaxVolume(streamType);
    }

    /**
     * getMinVolume get min volume of the stream
     *
     * @param streamType stream type of the audio
     * @return min volume of the stream
     */
    public int getMinVolume(int streamType) {
        if (audioManager_ == null) {
            Log.e(LOG_TAG, "audioManager_ is null");
            return -1;
        }
        Object result = invoke("android.media.AudioManager", "getStreamMinVolume",
                audioManager_, new Class<?>[] {int.class}, streamType);
        if (result instanceof Integer) {
            return (int) result;
        }
        return -1;
    }

    /**
     * isStreamMute method get stream mute
     *
     * @param streamType stream type of the audio
     * @return stream mute
     */
    public boolean isStreamMute(int streamType) {
        if (audioManager_ == null) {
            Log.e(LOG_TAG, "audioManager_ is null");
            return false;
        }
        return audioManager_.isStreamMute(streamType);
    }

    /**
     * getRingerMode method get ringer mode
     *
     * @return ringer mode
     */
    public int getRingerMode() {
        if (audioManager_ == null) {
            Log.e(LOG_TAG, "audioManager_ is null");
            return -1;
        }
        return audioManager_.getRingerMode();
    }

    /**
     * getRingerModeInternal method
     *
     * @return ringerMode internal
     */
    public boolean isMicrophoneMute() {
        if (audioManager_ == null) {
            Log.e(LOG_TAG, "audioManager_ is null");
            return false;
        }
        return audioManager_.isMicrophoneMute();
    }

    /**
     * isVolumeFixed method get volume fixed
     *
     * @return volume fixed
     */
    public boolean isVolumeFixed() {
        if (audioManager_ == null) {
            Log.e(LOG_TAG, "audioManager_ is null");
            return false;
        }
        return audioManager_.isVolumeFixed();
    }

    /**
     * getSystemVolumeInDb method
     *
     * @param streamType stream type of the audio
     * @param index index of the audio
     * @param deviceType device type of the audio
     * @return volume of the stream in db
     */
    public float getSystemVolumeInDb(int streamType, int index, int deviceType) {
        if (audioManager_ == null) {
            Log.e(LOG_TAG, "audioManager_ is null");
            return 0.0f;
        }
        Object result = invoke("android.media.AudioManager", "getStreamVolumeDb",
                audioManager_, new Class<?>[] {int.class, int.class, int.class}, streamType, index, deviceType);
        if (result instanceof Float) {
            return (float) result;
        }
        return 0.0f;
    }

    /**
     * getAudioScene method get audio scene
     *
     * @return audio scene
     */
    public int getAudioScene() {
        if (audioManager_ == null) {
            Log.e(LOG_TAG, "audioManager_ is null");
            return AudioManager.MODE_INVALID;
        }
        return audioManager_.getMode();
    }

    /**
     * getAudioSceneInternal method
     *
     * @param flags flags of audio scene
     * @return audio scene
     */
    public AudioDeviceInfo[] getDevices(int flags) {
        if (audioManager_ == null) {
            Log.e(LOG_TAG, "audioManager_ is null");
            return null;
        }
        return audioManager_.getDevices(flags);
    }

    void registerAudioDeviceCallback() {
        if (deviceChangeCallback_ == null) {
            if (audioManager_ == null) {
                Log.e(LOG_TAG, "audioManager_ is null");
                return;
            }
            deviceChangeCallback_ = new AudioDeviceChangeCallbackImpl();
            audioManager_.registerAudioDeviceCallback(deviceChangeCallback_, null);
        } else {
            Log.w(LOG_TAG, "AudioDeviceCallback has already been registered");
        }
    }

    void unregisterAudioDeviceCallback() {
        if (deviceChangeCallback_ != null) {
            if (audioManager_ == null) {
                Log.e(LOG_TAG, "audioManager_ is null");
                return;
            }
            audioManager_.unregisterAudioDeviceCallback(deviceChangeCallback_);
            deviceChangeCallback_ = null;
        } else {
            Log.w(LOG_TAG, "AudioDeviceCallback has not been registered");
        }
    }

    /**
     * Get current audio renderer info array
     *
     * @return List<AudioPlaybackConfiguration>
     */
    public List<AudioPlaybackConfiguration> getCurrentAudioRendererInfoArray() {
        if (audioManager_ == null) {
            Log.e(LOG_TAG, "audioManager_ is null");
            return null;
        }
        return audioManager_.getActivePlaybackConfigurations();
    }

    /**
     * Get current audio capturer info array
     *
     * @return List<AudioRecordingConfiguration>
     */
    public List<AudioRecordingConfiguration> getCurrentAudioCapturerInfoArray() {
        if (audioManager_ == null) {
            Log.e(LOG_TAG, "audioManager_ is null");
            return null;
        }
        return audioManager_.getActiveRecordingConfigurations();
    }

    /**
     * registerAudioRendererEventListener method
     */
    public void registerAudioRendererEventListener() {
        if (audioPlaybackCallbacklListeners_ == null) {
            if (audioManager_ == null) {
                Log.e(LOG_TAG, "audioManager_ is null");
                return;
            }
            audioPlaybackCallbacklListeners_ = new OnAudioPlaybackCallbackListenerImpl();
            audioManager_.registerAudioPlaybackCallback(audioPlaybackCallbacklListeners_, null);
        } else {
            Log.w(LOG_TAG, "AudioPlaybackCallback has already been registered");
        }
    }

    /**
     * unregisterAudioRendererEventListener method
     */
    public void unregisterAudioRendererEventListener() {
        if (audioPlaybackCallbacklListeners_ != null) {
            if (audioManager_ == null) {
                Log.e(LOG_TAG, "audioManager_ is null");
                return;
            }
            audioManager_.unregisterAudioPlaybackCallback(audioPlaybackCallbacklListeners_);
            audioPlaybackCallbacklListeners_ = null;
        } else {
            Log.w(LOG_TAG, "AudioPlaybackCallback has not been registered");
        }
    }

    /**
     * registerAudioCapturerEventListener method
     */
    public void registerAudioCapturerEventListener() {
        if (audioRecordingCallbacklListeners_ == null) {
            if (audioManager_ == null) {
                Log.e(LOG_TAG, "audioManager_ is null");
                return;
            }
            audioRecordingCallbacklListeners_ = new OnAudioRecordingCallbackListenerImpl();
            audioManager_.registerAudioRecordingCallback(audioRecordingCallbacklListeners_, null);
        } else {
            Log.w(LOG_TAG, "AudioRecordingCallback has already been registered");
        }
    }

    /**
     * unregisterAudioCapturerEventListener method
     */
    public void unregisterAudioCapturerEventListener() {
        if (audioRecordingCallbacklListeners_ != null) {
            if (audioManager_ == null) {
                Log.e(LOG_TAG, "audioManager_ is null");
                return;
            }
            audioManager_.unregisterAudioRecordingCallback(audioRecordingCallbacklListeners_);
            audioRecordingCallbacklListeners_ = null;
        } else {
            Log.w(LOG_TAG, "AudioRecordingCallback has not been registered");
        }
    }

    /**
     * setDeviceActive method
     *
     * @param deviceType device type
     * @param flag flag
     * @return boolean
     */
    public boolean setDeviceActive(int deviceType, boolean flag) {
        if (audioManager_ == null) {
            Log.e(LOG_TAG, "audioManager_ is null");
            return false;
        }
        if (!flag) {
            invoke("android.media.AudioManager", "clearCommunicationDevice", audioManager_, null);
            return true;
        }
        AudioDeviceInfo lastDeviceInfo = null;
        Object result = invoke("android.media.AudioManager", "getAvailableCommunicationDevices", audioManager_, null);
        if (!(result instanceof List)) {
            return false;
        }
        int size = ((List) result).size();
        for (int i = 0; i < size; i++) {
            AudioDeviceInfo deviceInfo = (AudioDeviceInfo) (((List) result).get(i));
            if (deviceInfo.getType() == deviceType && deviceInfo.isSink()) {
                lastDeviceInfo = deviceInfo;
                break;
            }
        }
        if (lastDeviceInfo != null) {
            Object resultSet = invoke("android.media.AudioManager",
                    "setCommunicationDevice", audioManager_, new Class<?>[] {AudioDeviceInfo.class}, lastDeviceInfo);
            if (resultSet instanceof Boolean) {
                return (Boolean) resultSet;
            }
        }
        return false;
    }

    /**
     * isDeviceActive method
     *
     * @param deviceType device type
     * @return boolean
     */
    public boolean isDeviceActive(int deviceType) {
        if (audioManager_ == null) {
            Log.e(LOG_TAG, "audioManager_ is null");
            return false;
        }
        Object result = invoke("android.media.AudioManager", "getCommunicationDevice",
                audioManager_, null);
        if (result instanceof AudioDeviceInfo) {
            AudioDeviceInfo deviceInfo = (AudioDeviceInfo) result;
            return deviceInfo.getType() == deviceType;
        }
        return false;
    }

    /**
     * Notifications of audio device connection and disconnection events.
     */
    public class AudioDeviceChangeCallbackImpl extends AudioDeviceCallback {
        @Override
        public void onAudioDevicesAdded(AudioDeviceInfo[] addedDevices) {
            nativeOnAudioDeviceChanged(true, addedDevices);
        }

        @Override
        public void onAudioDevicesRemoved(AudioDeviceInfo[] removedDevices) {
            nativeOnAudioDeviceChanged(false, removedDevices);
        }
    }

    /**
     * Notifications of audio playback and recording events.
     */
    public class OnAudioPlaybackCallbackListenerImpl extends AudioManager.AudioPlaybackCallback {
        @Override
        public void onPlaybackConfigChanged(List<AudioPlaybackConfiguration> configs) {
            nativeOnAudioRendererChanged(configs);
        }
    }

    /**
     * Notifications of audio recording events.
     */
    public class OnAudioRecordingCallbackListenerImpl extends AudioManager.AudioRecordingCallback {
        @Override
        public void onRecordingConfigChanged(List<AudioRecordingConfiguration> configs) {
            nativeOnAudioCapturerChanged(configs);
        }
    }

    /**
     * AudioManagerPlugin native method.
     */
    protected native void nativeInit();

    /**
     * AudioManagerPlugin native method.
     *
     * @param isAdded true if the device is added, false if the device is removed
     * @param devices AudioDeviceInfo array
     */
    protected native void nativeOnAudioDeviceChanged(boolean isAdded, AudioDeviceInfo[] devices);

    /**
     * AudioManagerPlugin native method.
     *
     * @param configs AudioPlaybackConfiguration list
     */
    protected native void nativeOnAudioRendererChanged(List<AudioPlaybackConfiguration> configs);

    /**
     * AudioManagerPlugin native method.
     *
     * @param configs AudioRecordingConfiguration list
     */
    protected native void nativeOnAudioCapturerChanged(List<AudioRecordingConfiguration> configs);
}
