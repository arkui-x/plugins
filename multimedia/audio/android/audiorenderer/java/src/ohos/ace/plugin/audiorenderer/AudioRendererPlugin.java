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
 *
 * @since 2024-06-24
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

    /**
     * createAudioTrack create audio track
     *
     * @param rendererPtr rendererPtr of the audio renderer
     * @param sampleRate sample rate of the audio track
     * @param channel channel of the audio track
     * @param encoding encoding of the audio track
     * @param channelLayout channel layout of the audio track
     * @param usage usage of the audio track
     * @param content content of the audio track
     * @param privacyType privacy type of the audio track
     * @param maxSpeed max speed of the audio track
     */
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

    /**
     * Finalize the audio renderer.
     *
     * @param rendererPtr the renderer pointer
     */
    public void finalize(long rendererPtr) {
        audioTracks.remove(rendererPtr);
        positionUpdateListeners.remove(rendererPtr);
        deviceChangeListeners.remove(rendererPtr);
    }

    /**
     * Play the audio track.
     *
     * @param rendererPtr the renderer pointer
     */
    public void play(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return;
        }
        audioTrack.play();
    }

    /**
     * Pause the audio track.
     *
     * @param rendererPtr the renderer pointer
     */
    public void pause(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return;
        }
        audioTrack.pause();
    }

    /**
     * Set the volume of the audio track.
     *
     * @param rendererPtr the renderer pointer
     */
    public void stop(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return;
        }
        audioTrack.stop();
    }

    /**
     * Release the audio track.
     *
     * @param rendererPtr the renderer pointer
     */
    public void release(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return;
        }
        audioTrack.release();
        finalize(rendererPtr);
    }

    /**
     * Flush the audio track.
     *
     * @param rendererPtr the renderer pointer
     */
    public void flush(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return;
        }
        audioTrack.flush();
    }

    /**
     * Set the speed of the audio track.
     *
     * @param rendererPtr the renderer pointer
     * @param speed the speed to set
     */
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

    /**
     * Set the volume of the audio track.
     *
     * @param rendererPtr the renderer pointer
     * @return the result of setting the volume
     */
    public float getSpeed(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return 0.0f;
        }

        PlaybackParams params = audioTrack.getPlaybackParams();
        return params.getSpeed();
    }

    /**
     * Get the minimum buffer size of the audio track.
     *
     * @param rendererPtr the renderer pointer
     * @param sampleRateInHz the sample rate in Hz
     * @param channelConfig the channel configuration
     * @param audioFormat the audio format
     * @return the minimum buffer size
     */
    public int getMinBufferSize(long rendererPtr, int sampleRateInHz, int channelConfig, int audioFormat) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return -1;
        }
        return AudioTrack.getMinBufferSize(sampleRateInHz, channelConfig, audioFormat);
    }

    /**
     * Write data to the audio track.
     *
     * @param rendererPtr the renderer pointer
     * @param buffer the buffer to write
     * @param size the size of the buffer
     * @return the result of writing the data
     */
    public int write(long rendererPtr, byte[] buffer, int size) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return -1;
        }
        return audioTrack.write(buffer, 0, size);
    }

    /**
     * Set the volume of the audio track.
     *
     * @param rendererPtr the renderer pointer
     * @param gain the gain to set
     * @return the result of setting the volume
     */
    public int setVolume(long rendererPtr, float gain) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return AudioTrack.ERROR_INVALID_OPERATION;
        }
        return audioTrack.setVolume(gain);
    }

    /**
     * Get the minimum volume of the audio track.
     *
     * @param rendererPtr the renderer pointer
     * @return the minimum volume of the audio track
     */
    public float getMinVolume(long rendererPtr) {
        return AudioTrack.getMinVolume();
    }

    /**
     * Get the maximum volume of the audio track.
     *
     * @param rendererPtr the renderer pointer
     * @return the maximum volume of the audio track
     */
    public float getMaxVolume(long rendererPtr) {
        return AudioTrack.getMaxVolume();
    }

    /**
     * Get the number of times the audio track has underrun.
     *
     * @param rendererPtr the renderer pointer
     * @return the number of times the audio track has underrun
     */
    public int getUnderrunCount(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return -1;
        }
        return audioTrack.getUnderrunCount();
    }

    /**
     * Set the notification marker position of the audio track.
     *
     * @param rendererPtr the renderer pointer
     * @param markerInFrames the marker position in frames
     * @return the result of setting the notification marker position
     */
    public int setNotificationMarkerPosition(long rendererPtr, int markerInFrames) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return AudioTrack.ERROR_INVALID_OPERATION;
        }
        return audioTrack.setNotificationMarkerPosition(markerInFrames);
    }

    /**
     * Set the playback position update listener for the audio track.
     *
     * @param rendererPtr the renderer pointer
     */
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

    /**
     * Unset the playback position update listener for the audio track.
     *
     * @param rendererPtr the renderer pointer
     */
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

    /**
     * Set the position notification period for the audio track.
     *
     * @param rendererPtr the renderer pointer
     * @param periodInFrames the period in frames
     * @return the result of setting the position notification period
     */
    public int setPositionNotificationPeriod(long rendererPtr, int periodInFrames) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return AudioTrack.ERROR_INVALID_OPERATION;
        }
        return audioTrack.setPositionNotificationPeriod(periodInFrames);
    }

    /**
     * Get the current output device
     *
     * @param rendererPtr the renderer pointer
     * @return the current output device
     */
    public AudioDeviceInfo getCurrentOutputDevices(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return null;
        }
        return audioTrack.getRoutedDevice();
    }

    /**
     * Get the audio session id
     *
     * @param rendererPtr the renderer pointer
     * @return the audio session id
     */
    public int getAudioSessionId(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return -1;
        }
        return audioTrack.getAudioSessionId();
    }

    /**
     * Add the routing changed listener.
     *
     * @param rendererPtr the audio track pointer
     */
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

    /**
     * Remove the routing changed listener.
     *
     * @param rendererPtr the audio track pointer
     */
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

    /**
     * Set the dual mono mode of the audio track.
     *
     * @param rendererPtr the audio track pointer
     * @param dualMonoMode the dual mono mode
     * @return true if the operation is successful, false otherwise
     */
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

    /**
     * Get the audio attributes of the audio track.
     *
     * @param rendererPtr the audio track pointer
     * @return the audio attributes
     */
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

    /**
     * Get the audio format of the audio track.
     *
     * @param rendererPtr the audio track pointer
     * @return the audio format
     */
    public AudioFormat getFormat(long rendererPtr) {
        AudioTrack audioTrack = audioTracks.get(rendererPtr);
        if (audioTrack == null) {
            Log.e(LOG_TAG, "audioTrack is null");
            return null;
        }
        return audioTrack.getFormat();
    }

    /**
     * Used for notifications when the playback head reaches a set position.
     *
     * @param rendererPtr The renderer pointer.
     * @param timestamp The timestamp.
     * @return True if the timestamp is set successfully, false otherwise.
     */
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

    /**
     * AudioRendererPlugin native method.
     *
     * @param key key
     * @param position position
     */
    protected native void nativeOnMarkerReached(long key, int position);

    /**
     * AudioRendererPlugin native method.
     *
     * @param key key
     * @param period period
     */
    protected native void nativeOnPeriodicNotification(long key, int period);

    /**
     * AudioRendererPlugin native method.
     *
     * @param key key
     * @param deviceInfo deviceInfo
     */
    protected native void nativeOnRoutingChanged(long key, AudioDeviceInfo deviceInfo);
}
