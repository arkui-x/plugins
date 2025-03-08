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

package ohos.ace.plugin.recorderplugin;

import android.content.Context;
import android.media.MediaRecorder;
import android.view.Surface;
import android.util.Log;
import java.util.Map;
import java.util.HashMap;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * RecorderPlugin
 *
 * @since 1
 */
public class RecorderPlugin implements MediaRecorder.OnInfoListener, MediaRecorder.OnErrorListener {
    private static final String LOG_TAG = "RecorderPlugin";
    private Map<Long, MediaRecorder> mediaRecorderMap = new HashMap<>();

    /**
     * RecorderPlugin
     *
     * @param context context of the application
     */
    public RecorderPlugin(Context context) {
        if (context == null) {
            Log.e(LOG_TAG, "context is null");
            return;
        }
        nativeInit();
    }

    /**
     * Called when an error occurs while recording.
     *
     * @param mr The MediaRecorder that encountered the error
     * @param what The type of error that has occurred
     * @param extra An extra code, specific to the error type
     */
    @Override
    public void onError(MediaRecorder mr, int what, int extra) {
        long key = getMediaRecorderKey(mr);
        if (key != 0) {
            nativeOnError(key, what, extra);
        }
    }

    /**
     * Called to indicate an info or a warning during recording.
     *
     * @param mr    The MediaRecorder the info pertains to
     * @param what  The type of info or warning that has occurred
     * @param extra An extra code, specific to the info type
     */
    @Override
    public void onInfo(MediaRecorder mr, int what, int extra) {
        long key = getMediaRecorderKey(mr);
        if (key != 0) {
            nativeOnInfo(key, what, extra);
        }
    }

    /**
     * Creates a MediaRecorder object.
     *
     * @param key The key of the MediaRecorder.
     */
    public void createMediaRecorder(long key) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            Log.e(LOG_TAG, "createMediaRecorder mediaRecorder exist.");
            return;
        }

        mediaRecorder = new MediaRecorder();
        mediaRecorderMap.put(key, mediaRecorder);
        mediaRecorder.setOnErrorListener(this);
        mediaRecorder.setOnInfoListener(this);
    }

    /**
     * Releases a MediaRecorder object.
     *
     * @param key The key of the MediaRecorder.
     */
    public void releaseMediaRecorder(long key) {
        mediaRecorderMap.remove(key);
    }

    /**
     * Prepares the recorder to begin capturing and encoding data.
     *
     * @param key The key of the MediaRecorder.
     */
    public void prepare(long key) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            try {
                mediaRecorder.prepare();
            } catch (IOException ex) {
                // Handling IOException exceptions.
                System.out.println("IOException exceptions: " + ex.getMessage());
                ex.printStackTrace();
            }
        }
    }

    /**
     * Begins capturing and encoding data to the file.
     *
     * @param key The key of the MediaRecorder.
     */
    public void start(long key) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            mediaRecorder.start();
        }
    }

    /**
     * Pauses recording.
     *
     * @param key The key of the MediaRecorder.
     */
    public void pause(long key) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            mediaRecorder.pause();
        }
    }

    /**
     * Resumes recording.
     *
     * @param key The key of the MediaRecorder.
     */
    public void resume(long key) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            mediaRecorder.resume();
        }
    }

    /**
     * Stop the MediaRecorder to its idle state.
     *
     * @param key The key of the MediaRecorder.
     */
    public void stop(long key) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            mediaRecorder.stop();
        }
    }

    /**
     * Restarts the MediaRecorder to its idle state.
     *
     * @param key The key of the MediaRecorder.
     */
    public void reset(long key) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            mediaRecorder.reset();
        }
    }

    /**
     * Releases resources associated with this MediaRecorder object.
     *
     * @param key The key of the MediaRecorder.
     */
    public void release(long key) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            mediaRecorder.release();
        }
    }

    /**
     * Sets the width and height of the video to be captured.
     *
     * @param key The key of the MediaRecorder.
     * @param width The width of the video to be captured.
     * @param height The height of the video to be captured.
     */
    public void setVideoSize(long key, int width, int height) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            mediaRecorder.setVideoSize(width, height);
        }
    }

    /**
     * Sets the video encoder to be used for recording.
     *
     * @param key The key of the MediaRecorder.
     * @param video_encoder The video encoder to use.
     */
    public void setVideoEncoder(long key, int video_encoder) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            mediaRecorder.setVideoEncoder(video_encoder);
        }
    }

    /**
     * Sets the video encoding bit rate for recording.
     *
     * @param key The key of the MediaRecorder.
     * @param bitRate The video encoding bit rate in bits per second.
     */
    public void setVideoEncodingBitRate(long key, int bitRate) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            mediaRecorder.setVideoEncodingBitRate(bitRate);
        }
    }

    /**
     * Sets the frame rate of the video to be captured.
     *
     * @param key The key of the MediaRecorder.
     * @param rate The number of frames per second of video to capture
     */
    public void setVideoFrameRate(long key, int rate) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            mediaRecorder.setVideoFrameRate(rate);
        }
    }

    /**
     * Sets the video source to be used for recording.
     *
     * @param key The key of the MediaRecorder.
     * @param video_source The video source to use.
     */
    public void setVideoSource(long key, int video_source) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            mediaRecorder.setVideoSource(video_source);
        }
    }

    /**
     * Gets the surface to record from when using SURFACE video source.
     *
     * @param key The key of the MediaRecorder.
     * @return Surface
     */
    public Surface getSurface(long key) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            return mediaRecorder.getSurface();
        }
        return null;
    }

    /**
     * Gets the surface to record from when using SURFACE video source.
     *
     * @param key The key of the MediaRecorder.
     * @param audioSource The value of the audio source.
     */
    public void setAudioSource(long key, int audioSource) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            mediaRecorder.setAudioSource(audioSource);
        }
    }

    /**
     * Gets the surface to record from when using SURFACE video source.
     *
     * @param key The key of the MediaRecorder.
     * @param samplingRate The audio sampling rate in Hertz.
     */
    public void setAudioSamplingRate(long key, int samplingRate) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            mediaRecorder.setAudioSamplingRate(samplingRate);
        }
    }

    /**
     * Sets the audio encoding bit rate for recording.
     *
     * @param key The key of the MediaRecorder.
     * @param bitRate The audio encoding bit rate in bits per second.
     */
    public void setAudioEncodingBitRate(long key, int bitRate) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            mediaRecorder.setAudioEncodingBitRate(bitRate);
        }
    }

    /**
     * Sets the audio encoder to be used for recording.
     *
     * @param key The key of the MediaRecorder.
     * @param audio_encoder The value of the audio encoder.
     */
    public void setAudioEncoder(long key, int audio_encoder) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            mediaRecorder.setAudioEncoder(audio_encoder);
        }
    }

    /**
     * Sets the number of audio channels for recording.
     *
     * @param key The key of the MediaRecorder.
     * @param numChannels The number of audio channels.
     */
    public void setAudioChannels(long key, int numChannels) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            mediaRecorder.setAudioChannels(numChannels);
        }
    }

    /**
     * Sets the format of the output file produced during recording.
     *
     * @param key The key of the MediaRecorder.
     * @param output_format The output format to use.
     */
    public void setOutputFormat(long key, int output_format) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            mediaRecorder.setOutputFormat(output_format);
        }
    }

    /**
     * Pass in the file descriptor of the file to be written.
     *
     * @param key The key of the MediaRecorder.
     * @param url The file descriptor of the file to be written.
     */
    public void setOutputFile(long key, String url) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            try {
                FileOutputStream fs = new FileOutputStream(url);
                mediaRecorder.setOutputFile(fs.getFD());
            } catch (IOException ex) {
                Log.e(LOG_TAG, "setOutputFile IOException:" + ex.getMessage());
            }
        }
    }

    /**
     * Set and store the geodata (latitude and longitude) in the output file.
     *
     * @param key The key of the MediaRecorder.
     * @param latitude Latitude in degrees. Its value must be in the range [-90, 90].
     * @param longitude Longitude in degrees. Its value must be in the range [-180, 180].
     */
    public void setLocation(long key, float latitude, float longitude) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            mediaRecorder.setLocation(latitude, longitude);
        }
    }

    /**
     * Sets the orientation hint for output video playback.
     *
     * @param key The key of the MediaRecorder.
     * @param degrees The angle to be rotated clockwise in degrees.
     */
    public void setOrientationHint(long key, int degrees) {
        MediaRecorder mediaRecorder = mediaRecorderMap.get(key);
        if (mediaRecorder != null) {
            mediaRecorder.setOrientationHint(degrees);
        }
    }

    /**
     * Gets the key of the MediaRecorder.
     *
     * @param mr The MediaRecorder.
     * @return long The key of the MediaRecorder.
     */
    private long getMediaRecorderKey(MediaRecorder mr) {
        for (long key : mediaRecorderMap.keySet()) {
            MediaRecorder value = mediaRecorderMap.get(key);
            if (value == mr) {
                return key;
            }
        }
        return 0;
    }

    /**
     * Init RecorderPlugin jni.
     *
     */
    protected native void nativeInit();

    /**
     * Called to indicate an info or a warning during recording.
     *
     * @param key The key of the MediaRecorder.
     * @param what The type of info or warning that has occurred
     * @param extra An extra code, specific to the info type
     */
    protected native void nativeOnInfo(long key, int what, int extra);

    /**
     * Called when an error occurs while recording.
     *
     * @param key The key of the MediaRecorder.
     * @param what The type of error that has occurred.
     * @param extra An extra code, specific to the error type.
     */
    protected native void nativeOnError(long key, int what, int extra);
}
