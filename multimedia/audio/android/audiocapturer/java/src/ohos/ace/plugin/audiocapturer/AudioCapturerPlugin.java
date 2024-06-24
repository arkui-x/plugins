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
import android.media.AudioAttributes;
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

    public int createAudioRecord(long capturerPtr, int sampleRate, int channel, int encoding,
                                  int sourceType) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord != null) {
            return AudioRecord.SUCCESS;
        }
        if (sourceType == AudioSource.MIC) {
            PackageManager pm = appContext.getPackageManager();
            boolean permission = (PackageManager.PERMISSION_GRANTED ==
                pm.checkPermission("android.permission.RECORD_AUDIO", appContext.getPackageName()));
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

    public void finalize(long capturerPtr) {
        mAudioRecords.remove(capturerPtr);
        positionUpdateListeners.remove(capturerPtr);
        deviceChangeListeners.remove(capturerPtr);
        infoChangeListeners.remove(capturerPtr);
    }

    public void startRecording(long capturerPtr) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return;
        }
        audioRecord.startRecording();
    }

    public void stop(long capturerPtr) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return;
        }
        audioRecord.stop();
    }

    public void release(long capturerPtr) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return;
        }
        audioRecord.release();
        finalize(capturerPtr);
    }

    public int getMinBufferSize(long capturerPtr, int sampleRateInHz, int channelConfig, int audioFormat) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return -1;
        }
        return AudioRecord.getMinBufferSize(sampleRateInHz, channelConfig, audioFormat);
    }

    public int read(long capturerPtr, byte[] buffer, int size) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return -1;
        }
        int retSize = audioRecord.read(buffer, 0, size);
        return retSize;
    }

    public int getAudioSource(long capturerPtr) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return -1;
        }
        return audioRecord.getAudioSource();
    }

    public int setNotificationMarkerPosition(long capturerPtr, int markerInFrames) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return AudioRecord.ERROR_INVALID_OPERATION;
        }
        return audioRecord.setNotificationMarkerPosition(markerInFrames);
    }

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

    public int setPositionNotificationPeriod(long capturerPtr, int periodInFrames) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return AudioRecord.ERROR_INVALID_OPERATION;
        }
        return audioRecord.setPositionNotificationPeriod(periodInFrames);
    }

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

    public AudioFormat getFormat(long capturerPtr) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return null;
        }
        return audioRecord.getFormat();
    }

    public int getAudioSessionId(long capturerPtr) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return -1;
        }
        return audioRecord.getAudioSessionId();
    }

    public int getTimestamp(long capturerPtr, AudioTimestamp timestamp) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return AudioRecord.ERROR_INVALID_OPERATION;
        }
        return audioRecord.getTimestamp(timestamp,  AudioTimestamp.TIMEBASE_MONOTONIC);
    }

    public AudioDeviceInfo getRoutedDevice(long capturerPtr) {
        AudioRecord audioRecord = mAudioRecords.get(capturerPtr);
        if (audioRecord == null) {
            Log.e(LOG_TAG, "audioRecord is null");
            return null;
        }
        return audioRecord.getRoutedDevice();
    }

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
    protected native void nativeOnMarkerReached(long key, int position);
    protected native void nativeOnPeriodicNotification(long key, int period);
    protected native void nativeOnRoutingChanged(long key, AudioDeviceInfo deviceInfo);
    protected native void nativeOnInfoChanged(long key, AudioDeviceInfo deviceInfo);
}
