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

package ohos.ace.plugin.audiorenderer;

import android.content.Context;
import android.media.AudioAttributes;
import android.media.AudioDeviceInfo;
import android.media.AudioFormat;
import android.media.AudioRouting;
import android.media.AudioRouting.OnRoutingChangedListener;
import android.media.AudioTimestamp;
import android.media.AudioTrack;
import android.media.AudioTrack.OnPlaybackPositionUpdateListener;
import android.media.PlaybackParams;
import android.util.Log;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Map;
import java.util.HashMap;

/**
 * AudioRendererPlugin
 */
public class AudioRendererPlugin {
    private static final String LOG_TAG = "AudioRendererPlugin";
    private Map<Long, AudioTrack> audioTracks = new HashMap<>();
    private Map<Long, OnPlaybackPositionUpdateListenerImpl> positionUpdateListeners = new HashMap<>();
    private Map<Long, OnRoutingChangedListenerImpl> deviceChangeListeners = new HashMap<>();

    /**
     * AudioRendererPlugin
     *
     * @param context context of the application
     */
    public AudioRendererPlugin(Context context) {
        if (context == null) {
            Log.e(LOG_TAG, "context is null");
            return;
        }
        nativeInit();
    }

    public void createAudioTrack(long rendererPtr, int sampleRate, int channel, int encoding,
                                int channelLayout, int usage, int content, int privacyType, float maxSpeed) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack != null) {
            return;
        }

        // create AudioAttributes
        AudioAttributes.Builder attributeBuilder = new AudioAttributes.Builder();
        attributeBuilder.setUsage(usage);
        attributeBuilder.setContentType(content);
        invoke("android.media.AudioAttributes.Builder", "setAllowedCapturePolicy",
            attributeBuilder, new Class<?>[] {int.class}, privacyType);
        AudioAttributes attribute = attributeBuilder.build();

        // create AudioFormat
        AudioFormat.Builder formatBuilder = new AudioFormat.Builder();
        formatBuilder.setEncoding(encoding);
        formatBuilder.setSampleRate(sampleRate);
        formatBuilder.setChannelMask(channelLayout);
        AudioFormat format = formatBuilder.build();

        int bufferSizeInBytes = AudioTrack.getMinBufferSize(sampleRate, channelLayout, encoding);
        Log.i(LOG_TAG, "getMinBufferSize bufferSizeInBytes=" + bufferSizeInBytes);

        // create AudioFormat
        audioTrack = new AudioTrack.Builder()
                .setAudioAttributes(attribute)
                .setAudioFormat(format)
                .setBufferSizeInBytes((int) (bufferSizeInBytes * maxSpeed))
                .build();

        audioTracks.put(rendererPtr, audioTrack);
    }

    public void finalize(long rendererPtr) {
        audioTracks.remove(rendererPtr);
        positionUpdateListeners.remove(rendererPtr);
        deviceChangeListeners.remove(rendererPtr);
    }

    public void play(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return;
        }
        audioTrack.play();
    }

    public void pause(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return;
        }
        audioTrack.pause();
    }

    public void stop(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return;
        }
        audioTrack.stop();
    }

    public void release(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return;
        }
        audioTrack.release();
        finalize(rendererPtr);
    }

    public void flush(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return;
        }
        audioTrack.flush();
    }

    public void setSpeed(long rendererPtr, float speed) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return;
        }

        PlaybackParams params = audioTrack.getPlaybackParams();
        params.setSpeed(speed);
        audioTrack.setPlaybackParams(params);
    }

    public float getSpeed(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return 0.0f;
        }

        PlaybackParams params = audioTrack.getPlaybackParams();
        return params.getSpeed();
    }

    public int getMinBufferSize(long rendererPtr, int sampleRateInHz, int channelConfig, int audioFormat) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return -1;
        }
        return AudioTrack.getMinBufferSize(sampleRateInHz, channelConfig, audioFormat);
    }

    public int write(long rendererPtr, byte[] buffer, int size) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return -1;
        }
        return audioTrack.write(buffer, 0, size);
    }

    public int setVolume(long rendererPtr, float gain) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return AudioTrack.ERROR_INVALID_OPERATION;
        }
        return audioTrack.setVolume(gain);
    }

    public float getMinVolume(long rendererPtr) {
        return AudioTrack.getMinVolume();
    }

    public float getMaxVolume(long rendererPtr) {
        return AudioTrack.getMaxVolume();
    }

    public int getUnderrunCount(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return -1;
        }
        return audioTrack.getUnderrunCount();
    }

    public int setNotificationMarkerPosition(long rendererPtr, int markerInFrames) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return AudioTrack.ERROR_INVALID_OPERATION;
        }
        return audioTrack.setNotificationMarkerPosition(markerInFrames);
    }

    public void setPlaybackPositionUpdateListener(long rendererPtr) {
        OnPlaybackPositionUpdateListenerImpl listener = positionUpdateListeners.get(rendererPtr);
        if (listener != null) {
            return;
        }
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return;
        }
        listener = new OnPlaybackPositionUpdateListenerImpl();
        audioTrack.setPlaybackPositionUpdateListener(listener);
        positionUpdateListeners.put(rendererPtr, listener);
    }

    public void unsetPlaybackPositionUpdateListener(long rendererPtr) {
        OnPlaybackPositionUpdateListenerImpl listener = positionUpdateListeners.get(rendererPtr);
        if (listener == null) {
            return;
        }
        positionUpdateListeners.remove(rendererPtr);
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return;
        }
        audioTrack.setPlaybackPositionUpdateListener(null);
    }

    public int setPositionNotificationPeriod(long rendererPtr, int periodInFrames) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return AudioTrack.ERROR_INVALID_OPERATION;
        }
        return audioTrack.setPositionNotificationPeriod(periodInFrames);
    }

    public AudioDeviceInfo getCurrentOutputDevices(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return null;
        }
        return audioTrack.getRoutedDevice();
    }

    public int getAudioSessionId(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return -1;
        }
        return audioTrack.getAudioSessionId();
    }

    public void addOnRoutingChangedListener(long rendererPtr) {
        OnRoutingChangedListenerImpl listener = deviceChangeListeners.get(rendererPtr);
        if (listener != null) {
            return;
        }
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return;
        }
        listener = new OnRoutingChangedListenerImpl();
        audioTrack.addOnRoutingChangedListener(listener, null);
        deviceChangeListeners.put(rendererPtr, listener);
    }

    public void removeOnRoutingChangedListener(long rendererPtr) {
        OnRoutingChangedListenerImpl listener = deviceChangeListeners.get(rendererPtr);
        if (listener == null) {
            return;
        }
        deviceChangeListeners.remove(rendererPtr);
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return;
        }
        audioTrack.removeOnRoutingChangedListener(listener);
    }

    public boolean setDualMonoMode(long rendererPtr, int dualMonoMode) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return false;
        }
        Object result = invoke("android.media.AudioTrack", "setDualMonoMode",
                                audioTrack, new Class<?>[] {int.class}, dualMonoMode);
        if (result instanceof Boolean) {
            return (boolean) result;
        }
        return false;
    }

    public AudioAttributes getAudioAttributes(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return null;
        }
        Object result = invoke("android.media.AudioTrack", "getAudioAttributes", audioTrack, null);
        if (result instanceof AudioAttributes) {
            return (AudioAttributes) result;
        }
        return null;
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

    public AudioFormat getFormat(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return null;
        }
        return audioTrack.getFormat();
    }

    public boolean getTimestamp(long rendererPtr, AudioTimestamp timestamp) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return false;
        }
        return audioTrack.getTimestamp(timestamp);
    }

    /**
     * Used for notifications when the previously set marker has been reached by the playback head.
     */
    public class OnPlaybackPositionUpdateListenerImpl implements AudioTrack.OnPlaybackPositionUpdateListener {
        @Override
        public void onMarkerReached(AudioTrack track) {
            for (long key : positionUpdateListeners.keySet()) {
                OnPlaybackPositionUpdateListenerImpl value = positionUpdateListeners.get(key);
                if (value == this) {
                    int position = track.getNotificationMarkerPosition();
                    nativeOnMarkerReached(key, position);
                    return;
                }
            }
        }

        @Override
        public void onPeriodicNotification(AudioTrack track) {
            for (long key : positionUpdateListeners.keySet()) {
                OnPlaybackPositionUpdateListenerImpl value = positionUpdateListeners.get(key);
                if (value == this) {
                    int period = track.getPositionNotificationPeriod();
                    nativeOnPeriodicNotification(key, period);
                    return;
                }
            }
        }
    };

    /**
     * Used for notifications when routing changes.
     */
    public class OnRoutingChangedListenerImpl implements AudioRouting.OnRoutingChangedListener {
        @Override
        public void onRoutingChanged(AudioRouting router) {
            for (long key : deviceChangeListeners.keySet()) {
                OnRoutingChangedListenerImpl value = deviceChangeListeners.get(key);
                if (value == this) {
                    AudioDeviceInfo deviceInfo = router.getRoutedDevice();
                    nativeOnRoutingChanged(key, deviceInfo);
                    return;
                }
            }
        }
    }

    /**
     * AudioRendererPlugin native method.
     */
    protected native void nativeInit();
    protected native void nativeOnMarkerReached(long key, int position);
    protected native void nativeOnPeriodicNotification(long key, int period);
    protected native void nativeOnRoutingChanged(long key, AudioDeviceInfo deviceInfo);
}
