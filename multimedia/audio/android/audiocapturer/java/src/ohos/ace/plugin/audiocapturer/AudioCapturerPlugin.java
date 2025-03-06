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

package ohos.ace.plugin.audiocapturer;

import android.util.Log;
import android.content.Context;
import android.content.pm.PackageManager;
import android.media.AudioRecord;
import android.media.MediaRecorder.AudioSource;
import android.media.AudioDeviceInfo;
import android.media.AudioFormat;
import android.media.AudioTimestamp;
import android.media.AudioRouting;
import android.media.AudioRouting.OnRoutingChangedListener;
import android.media.AudioRecord.OnRecordPositionUpdateListener;
import java.util.Map;
import java.util.HashMap;

/**
 * AudioCapturerPlugin
 *
 * @since 2024-06-24
 */
public class AudioCapturerPlugin {
    private static final String LOG_TAG = "AudioCapturerPlugin";
    private Context appContext;
    private Map<Long, AudioRecord> mAudioRecords = new HashMap<>();
    private Map<Long, OnRecordPositionUpdateListenerImpl> positionUpdateListeners = new HashMap<>();
    private Map<Long, OnRoutingChangedListenerImpl> deviceChangeListeners = new HashMap<>();
    private Map<Long, OnInfoChangedListenerImpl> infoChangeListeners = new HashMap<>();

    /**
     * AudioCapturerPlugin
     *
     * @param context context of the application
     */
    public AudioCapturerPlugin(Context context) {
        if (context == null) {
            Log.e(LOG_TAG, "context is null");
            return;
        }
        appContext = context;
        nativeInit();
    }

    /**
     * createAudioRecord create audio record
     *
     * @param capturerPtr capturerPtr of the audio record
     * @param sampleRate sampleRate of the audio record
     * @param channel channel of the audio record
     * @param encoding encoding of the audio record
     * @param sourceType sourceType of the audio record
     * @return int result of the operation
     */
    public int createAudioRecord(long capturerPtr, int sampleRate, int channel, int encoding,
                                  int sourceType) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord != null) {
            return AudioRecord.SUCCESS;
        }
        if (sourceType == AudioSource.MIC) {
            PackageManager pm = appContext.getPackageManager();
            boolean permission = (pm.checkPermission("android.permission.RECORD_AUDIO",
                    appContext.getPackageName()) == PackageManager.PERMISSION_GRANTED);
            if (!permission) {
                Log.e(LOG_TAG, "No microphone permission.createAudioRecord failed.");
                return AudioRecord.ERROR_INVALID_OPERATION;
            }
        }
        audioRecord = new AudioRecord.Builder()
        .setAudioSource(sourceType)
        .setAudioFormat(new AudioFormat.Builder()
            .setEncoding(encoding)
            .setSampleRate(sampleRate)
            .setChannelIndexMask(channel)
            .build())
        .build();
        mAudioRecords.put(capturerPtr, audioRecord);
        return AudioRecord.SUCCESS;
    }

    /**
     * finalize audio record
     *
     * @param capturerPtr capturerPtr of the audio record
     */
    public void finalize(long capturerPtr) {
        mAudioRecords.remove(capturerPtr);
        positionUpdateListeners.remove(capturerPtr);
        deviceChangeListeners.remove(capturerPtr);
        infoChangeListeners.remove(capturerPtr);
    }

    /**
     * start recording
     *
     * @param capturerPtr capturerPtr
     */
    public void startRecording(long capturerPtr) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return;
        }
        audioRecord.startRecording();
    }

    /**
     * stop recording
     *
     * @param capturerPtr capturerPtr
     */
    public void stop(long capturerPtr) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return;
        }
        audioRecord.stop();
    }

    /**
     * release audioRecord
     *
     * @param capturerPtr capturerPtr
     */
    public void release(long capturerPtr) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return;
        }
        audioRecord.release();
        finalize(capturerPtr);
    }

    /**
     * get min buffer size
     *
     * @param capturerPtr capturerPtr
     * @param sampleRateInHz sampleRateInHz
     * @param channelConfig channelConfig
     * @param audioFormat audioFormat
     * @return min buffer size
     */
    public int getMinBufferSize(long capturerPtr, int sampleRateInHz, int channelConfig, int audioFormat) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return -1;
        }
        return AudioRecord.getMinBufferSize(sampleRateInHz, channelConfig, audioFormat);
    }

    /**
     * read data from audioRecord
     *
     * @param capturerPtr capturerPtr
     * @param buffer buffer
     * @param size size
     * @return read size
     */
    public int read(long capturerPtr, byte[] buffer, int size) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return -1;
        }
        int retSize = audioRecord.read(buffer, 0, size);
        return retSize;
    }

    /**
     * get audio source from audioRecord
     *
     * @param capturerPtr capturerPtr
     * @return audio source
     */
    public int getAudioSource(long capturerPtr) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return -1;
        }
        return audioRecord.getAudioSource();
    }

    /**
     * set notification marker position
     *
     * @param capturerPtr capturerPtr
     * @param markerInFrames markerInFrames
     * @return result
     */
    public int setNotificationMarkerPosition(long capturerPtr, int markerInFrames) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return AudioRecord.ERROR_INVALID_OPERATION;
        }
        return audioRecord.setNotificationMarkerPosition(markerInFrames);
    }

    /**
     * set record position update listener
     *
     * @param capturerPtr capturerPtr
     */
    public void setRecordPositionUpdateListener(long capturerPtr) {
        OnRecordPositionUpdateListenerImpl listener = positionUpdateListeners.get(capturerPtr);
        if (listener != null) {
            return;
        }
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return;
        }
        listener = new OnRecordPositionUpdateListenerImpl();
        audioRecord.setRecordPositionUpdateListener(listener);
        positionUpdateListeners.put(capturerPtr, listener);
    }

    /**
     * unset record position update listener
     *
     * @param capturerPtr capturerPtr
     */
    public void unsetRecordPositionUpdateListener(long capturerPtr) {
        OnRecordPositionUpdateListenerImpl listener = positionUpdateListeners.get(capturerPtr);
        if (listener == null) {
            return;
        }
        positionUpdateListeners.remove(capturerPtr);
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return;
        }
        audioRecord.setRecordPositionUpdateListener(null);
    }

    /**
     * set position notification period
     *
     * @param capturerPtr capturerPtr
     * @param periodInFrames periodInFrames
     * @return result
     */
    public int setPositionNotificationPeriod(long capturerPtr, int periodInFrames) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return AudioRecord.ERROR_INVALID_OPERATION;
        }
        return audioRecord.setPositionNotificationPeriod(periodInFrames);
    }

    /**
     * add on routing changed listener
     *
     * @param capturerPtr capturerPtr
     */
    public void addOnRoutingChangedListener(long capturerPtr) {
        OnRoutingChangedListenerImpl listener = deviceChangeListeners.get(capturerPtr);
        if (listener != null) {
            return;
        }
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return;
        }
        listener = new OnRoutingChangedListenerImpl();
        audioRecord.addOnRoutingChangedListener(listener, null);
        deviceChangeListeners.put(capturerPtr, listener);
    }

    /**
     * add on info changed listener
     *
     * @param capturerPtr capturerPtr
     */
    public void addOnInfoChangedListener(long capturerPtr) {
        OnInfoChangedListenerImpl infoListener = infoChangeListeners.get(capturerPtr);
        if (infoListener != null) {
            return;
        }
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return;
        }
        infoListener = new OnInfoChangedListenerImpl();
        audioRecord.addOnRoutingChangedListener(infoListener, null);
        infoChangeListeners.put(capturerPtr, infoListener);
    }

    /**
     * remove on routing changed listener
     *
     * @param capturerPtr capturerPtr
     */
    public void removeOnRoutingChangedListener(long capturerPtr) {
        OnRoutingChangedListenerImpl listener = deviceChangeListeners.get(capturerPtr);
        if (listener == null) {
            return;
        }
        deviceChangeListeners.remove(capturerPtr);
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return;
        }
        audioRecord.removeOnRoutingChangedListener(listener);
    }

    /**
     * add on info changed listener
     *
     * @param capturerPtr capturerPtr
     */
    public void removeOnInfoChangedListener(long capturerPtr) {
        OnInfoChangedListenerImpl infoListener = infoChangeListeners.get(capturerPtr);
        if (infoListener == null) {
            return;
        }
        infoChangeListeners.remove(capturerPtr);
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return;
        }
        audioRecord.removeOnRoutingChangedListener(infoListener);
    }

    /**
     * Get the audio format of the audio record.
     *
     * @param capturerPtr capturerPtr
     * @return audio format
     */
    public AudioFormat getFormat(long capturerPtr) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return null;
        }
        return audioRecord.getFormat();
    }

    /**
     * Get the audio session id of the audio record.
     *
     * @param capturerPtr capturerPtr
     * @return audio session id
     */
    public int getAudioSessionId(long capturerPtr) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return -1;
        }
        return audioRecord.getAudioSessionId();
    }

    /**
     * Get the audio source of the audio record.
     *
     * @param capturerPtr capturerPtr
     * @param timestamp timestamp
     * @return audio source
     */
    public int getTimestamp(long capturerPtr, AudioTimestamp timestamp) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return AudioRecord.ERROR_INVALID_OPERATION;
        }
        return audioRecord.getTimestamp(timestamp, AudioTimestamp.TIMEBASE_MONOTONIC);
    }

    /**
     * AudioRecord.OnRoutingChangedListener implementation
     *
     * @param capturerPtr capturerPtr
     * @return OnRoutingChangedListenerImpl
     */
    public AudioDeviceInfo getRoutedDevice(long capturerPtr) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return null;
        }
        return audioRecord.getRoutedDevice();
    }

    /**
     * AudioRecord.OnRecordPositionUpdateListener implementation
     */
    public class OnRecordPositionUpdateListenerImpl implements AudioRecord.OnRecordPositionUpdateListener {
        @Override
        public void onMarkerReached(AudioRecord record) {
            for (long key : positionUpdateListeners.keySet()) {
                OnRecordPositionUpdateListenerImpl value = positionUpdateListeners.get(key);
                if (value == this) {
                    int position = record.getNotificationMarkerPosition();
                    nativeOnMarkerReached(key, position);
                    return;
                }
            }
        }

        @Override
        public void onPeriodicNotification(AudioRecord record) {
            for (long key : positionUpdateListeners.keySet()) {
                OnRecordPositionUpdateListenerImpl value = positionUpdateListeners.get(key);
                if (value == this) {
                    int period = record.getPositionNotificationPeriod();
                    nativeOnPeriodicNotification(key, period);
                    return;
                }
            }
        }
    };

    /**
     * AudioRouting.OnRoutingChangedListener implementation
     */
    public class OnRoutingChangedListenerImpl implements AudioRouting.OnRoutingChangedListener {
        @Override
        public void onRoutingChanged(AudioRouting router) {
            for (long key : deviceChangeListeners.keySet()) {
                OnRoutingChangedListenerImpl value = deviceChangeListeners.get(key);
                if (value == this) {
                    AudioDeviceInfo deviceInfo = router.getRoutedDevice();
                    if (deviceInfo != null) {
                        nativeOnRoutingChanged(key, deviceInfo);
                    }
                    return;
                }
            }
        }
    };

    /**
     * AudioDeviceInfo.OnInfoChangedListener implementation
     */
    public class OnInfoChangedListenerImpl implements AudioRouting.OnRoutingChangedListener {
        @Override
        public void onRoutingChanged(AudioRouting router) {
            for (long key : infoChangeListeners.keySet()) {
                OnInfoChangedListenerImpl value = infoChangeListeners.get(key);
                if (value == this) {
                    AudioDeviceInfo deviceInfo = router.getRoutedDevice();
                    if (deviceInfo != null) {
                        nativeOnInfoChanged(key, deviceInfo);
                    }
                    return;
                }
            }
        }
    };

    /**
     * AudioCapturerPlugin native method.
     */
    protected native void nativeInit();

    /**
     * AudioCapturerPlugin native method.
     *
     * @param key the key
     * @param position the position
     */
    protected native void nativeOnMarkerReached(long key, int position);

    /**
     * AudioCapturerPlugin native method.
     *
     * @param key the key
     * @param period the period
     */
    protected native void nativeOnPeriodicNotification(long key, int period);

    /**
     * AudioCapturerPlugin native method.
     *
     * @param key the key
     * @param deviceInfo the deviceInfo
     */
    protected native void nativeOnRoutingChanged(long key, AudioDeviceInfo deviceInfo);

    /**
     * AudioCapturerPlugin native method.
     *
     * @param key the key
     * @param deviceInfo the deviceInfo
     */
    protected native void nativeOnInfoChanged(long key, AudioDeviceInfo deviceInfo);
}
