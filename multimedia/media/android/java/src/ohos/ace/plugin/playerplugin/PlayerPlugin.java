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

package ohos.ace.plugin.playerplugin;

import android.content.Context;
import android.media.MediaPlayer;
import android.media.MediaDataSource;
import android.media.PlaybackParams;
import android.media.MediaFormat;
import android.os.Handler;
import android.util.Log;
import android.view.Surface;
import java.util.Map;
import java.util.HashMap;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.Runnable;

/**
 * PlayerPlugin
 *
 * @since 1
 */
public class PlayerPlugin {
    /**
     * player instance
     */
    private static final String LOG_TAG = "PlayerPlugin";

    /**
     * player info type speed done
     */
    public static final int PLAYER_INFO_TYPE_SPEEDDONE = 2;

    /**
     * player info type bitrate done
     */
    public static final int PLAYER_INFO_TYPE_BITRATEDONE = 3;

    /**
     * player info type eos
     */
    public static final int PLAYER_INFO_TYPE_EOS = 4;

    /**
     * player info type state change
     */
    public static final int PLAYER_INFO_TYPE_STATE_CHANGE = 5;

    /**
     * player info type position update
     */
    public static final int PLAYER_INFO_TYPE_POSITION_UPDATE = 6;

    /**
     * player info type duration update
     */
    public static final int PLAYER_INFO_TYPE_DURATION_UPDATE = 15;

    /**
     * PLAYER_STATE_ERROR
     */
    public static final int PLAYER_STATE_ERROR = 0;

    /**
     * PLAYER_STATE_IDLE
     */
    public static final int PLAYER_STATE_IDLE = 1;

    /**
     * PLAYER_STATE_INITIALIZED
     */
    public static final int PLAYER_STATE_INITIALIZED = 2;

    /**
     * PLAYER_STATE_PREPARING
     */
    public static final int PLAYER_STATE_PREPARED = 4;

    /**
     * PLAYER_STATE_STARTED
     */
    public static final int PLAYER_STATE_STARTED = 5;

    /**
     * PLAYER_STATE_PAUSED
     */
    public static final int PLAYER_STATE_PAUSED = 6;

    /**
     * PLAYER_STATE_STOPPED
     */
    public static final int PLAYER_STATE_STOPPED = 7;

    /**
     * PLAYER_STATE_PLAYBACK_COMPLETE
     */
    public static final int PLAYER_STATE_PLAYBACK_COMPLETE = 8;

    /**
     * PLAYER_STATE_RELEASED
     */
    public static final int PLAYER_STATE_RELEASED = 9;

    /**
     * PLAYER_TIME_UPDATE_TIME_DELAY
     */
    public static final int PLAYER_TIME_UPDATE_TIME_DELAY = 100;

    /**
     * PLAYER_TRACK_CODEC_MIME
     */
    private static final String PLAYER_TRACK_CODEC_MIME = "codec_mime";

    /**
     * PLAYER_CODEC_MIME_LENGTH
     */
    private static final int PLAYER_CODEC_MIME_LENGTH = 5;

    /**
     * player instance
     */
    private volatile Map<Long, MediaPlayer> mediaPlayerMap;

    /**
     * media data source
     */
    private Map<Long, MediaDataSourceImpl> mediaDataSourceMap;

    /**
     * PlayerPlugin
     *
     * @param context context of the application
     */
    public PlayerPlugin(Context context) {
        if (context == null) {
            Log.e(LOG_TAG, "context is null");
            return;
        }
        mediaPlayerMap = new HashMap<Long, MediaPlayer>();
        mediaDataSourceMap = new HashMap<Long, MediaDataSourceImpl>();
        runTimeUpdate();
        nativeInit();
    }

    /**
     * PreparedCallbackImpl implementation
     */
    public class PreparedCallbackImpl implements MediaPlayer.OnPreparedListener {
        @Override
        public void onPrepared(MediaPlayer mp) {
            long key = getMediaPlayerKey(mp);
            if (key != 0) {
                notifyInfo(key, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_PREPARED);
                notifyInfo(key, PLAYER_INFO_TYPE_DURATION_UPDATE, mp.getDuration());
                notifyInfo(key, PLAYER_INFO_TYPE_POSITION_UPDATE, 0);
            }
        }
    }

    /**
     * CompletionCallbackImpl
     */
    public class CompletionCallbackImpl implements MediaPlayer.OnCompletionListener {
        @Override
        public void onCompletion(MediaPlayer mp) {
            long key = getMediaPlayerKey(mp);
            if (key != 0) {
                nativeOnInfo(key, PLAYER_INFO_TYPE_EOS, 0);
                nativeOnInfo(key, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_PLAYBACK_COMPLETE);
                nativeOnInfo(key, PLAYER_INFO_TYPE_POSITION_UPDATE, mp.getDuration());
            }
        }
    }

    /**
     * SeekCompletionCallbackImpl
     */
    public class SeekCompletionCallbackImpl implements MediaPlayer.OnSeekCompleteListener {
        @Override
        public void onSeekComplete(MediaPlayer mp) {
            long key = getMediaPlayerKey(mp);
            if (key != 0) {
                nativeOnSeekComplete(key, mp.getCurrentPosition());
                notifyInfo(key, PLAYER_INFO_TYPE_POSITION_UPDATE, mp.getCurrentPosition());
            }
        }
    }

    /**
     * BufferingUpdateCallbackImpl
     */
    public class BufferingUpdateCallbackImpl implements MediaPlayer.OnBufferingUpdateListener {
        @Override
        public void onBufferingUpdate(MediaPlayer mp, int percent) {
            long key = getMediaPlayerKey(mp);
            if (key != 0) {
                nativeOnBufferingUpdate(key, percent);
            }
        }
    }

    /**
     * VideoSizeChangedCallbackImpl
     */
    public class VideoSizeChangedCallbackImpl implements MediaPlayer.OnVideoSizeChangedListener {
        @Override
        public void onVideoSizeChanged(MediaPlayer mp, int width, int height) {
            long key = getMediaPlayerKey(mp);
            if (key != 0) {
                nativeOnVideoSizeChanged(key, width, height);
            }
        }
    }

    /**
     * ErrorCallbackImpl
     */
    public class ErrorCallbackImpl implements MediaPlayer.OnErrorListener {
        @Override
        public boolean onError(MediaPlayer mp, int what, int extra) {
            long key = getMediaPlayerKey(mp);
            if (key != 0) {
                nativeOnError(key,
                    what == MediaPlayer.MEDIA_ERROR_UNKNOWN || what == MediaPlayer.MEDIA_ERROR_SERVER_DIED ?
                    what : extra);
                return true;
            }
            return false;
        }
    }

    /**
     * MediaDataSourceImpl
     *
     * The class is used for getting the data source.
     * @since 1
     */
    public class MediaDataSourceImpl extends MediaDataSource {
        private long key;

        public MediaDataSourceImpl(long id) {
            key = id;
        }

        @Override
        public long getSize() {
            return -1;
        }

        @Override
        public int readAt(long position, byte[] buffer, int offset, int size) {
            return nativeReadAt(key, position, buffer, offset, size);
        }

        @Override
        public void close() {
        }
    };

    /**
     * CallbackThread
     */
    private class CallbackThread extends Thread {
        private long id;
        private int infoType;
        private int info;

        @Override
        public void run() {
            nativeOnInfo(id, infoType, info);
        }

        public void setParams(long key, int what, int extra) {
            id = key;
            infoType = what;
            info = extra;
        }
    }

    /**
     * addCallbackThread to callback thread pool
     *
     * @param id the player id
     */
    public void addMediaPlayer(long id) {
        MediaPlayer mp = mediaPlayerMap.get(id);
        if (mp != null) {
            Log.e(LOG_TAG, "addMediaPlayer mediaPlayer exist.");
            return;
        }
        mp = new MediaPlayer();
        mediaPlayerMap.put(id, mp);
        notifyInfo(id, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_IDLE);

        mp.setOnPreparedListener(new PlayerPlugin.PreparedCallbackImpl());
        mp.setOnErrorListener(new PlayerPlugin.ErrorCallbackImpl());
        mp.setOnCompletionListener(new PlayerPlugin.CompletionCallbackImpl());
        mp.setOnSeekCompleteListener(new PlayerPlugin.SeekCompletionCallbackImpl());
        mp.setOnBufferingUpdateListener(new PlayerPlugin.BufferingUpdateCallbackImpl());
        mp.setOnVideoSizeChangedListener(new PlayerPlugin.VideoSizeChangedCallbackImpl());
    }

    private MediaPlayer getMediaPlayerById(long id) {
        return mediaPlayerMap.get(id);
    }

    private long getMediaPlayerKey(MediaPlayer mp) {
        for (long key : mediaPlayerMap.keySet()) {
            MediaPlayer value = mediaPlayerMap.get(key);
            if (value == mp) {
                return key;
            }
        }
        return 0;
    }

    private void notifyInfo(long key, int what, int extra) {
        CallbackThread td = new CallbackThread();
        td.setParams(key, what, extra);
        td.start();
    }

    private void runTimeUpdate() {
        Handler handler = new Handler();
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                for (long key : mediaPlayerMap.keySet()) {
                    MediaPlayer mp = mediaPlayerMap.get(key);
                    if (mp.isPlaying()) {
                        notifyInfo(key, PLAYER_INFO_TYPE_POSITION_UPDATE, mp.getCurrentPosition());
                    }
                }
                handler.postDelayed(this, PLAYER_TIME_UPDATE_TIME_DELAY);
            }
        };
        handler.postDelayed(runnable, 0);
    }

    /**
     * releaseMediaPlayer release the MediaPlayer
     *
     * @param key the key of MediaPlayer
     */
    public void releaseMediaPlayer(long key) {
        mediaPlayerMap.remove(key);
    }

    /**
     * setDataSource set the MediaPlayer data source with MediaDataSource
     *
     * @param id the key of MediaPlayer
     */
    public void setDataSource(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }

        MediaDataSourceImpl dataSource = new MediaDataSourceImpl(id);
        mediaDataSourceMap.put(id, dataSource);

        mp.setDataSource(dataSource);
        notifyInfo(id, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_INITIALIZED);
    }

    /**
     * setDataSourceWithUrl set the MediaPlayer data source with url
     *
     * @param id  the key of MediaPlayer
     * @param url  the file path
     */
    public void setDataSourceWithUrl(long id, String url) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        try {
            mp.setDataSource(url);
            notifyInfo(id, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_INITIALIZED);
        } catch (IOException ex) {
            Log.e(LOG_TAG, "setDataSourceWithUrl IOException:" + ex.getMessage());
        }
    }

    /**
     * setDataSourceWithFd set the MediaPlayer data source with file descriptor
     *
     * @param id    the key of MediaPlayer
     * @param url   the file path
     * @param offset the offset of the file
     * @param length the length of the file
     */
    public void setDataSourceWithFd(long id, String url, long offset, long length) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        try {
            if (length <= 0) {
                File file = new File(url);
                length = file.length();
            }
            FileInputStream fs = new FileInputStream(url);

            mp.setDataSource(fs.getFD(), offset, length);
            notifyInfo(id, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_INITIALIZED);
        } catch (IOException ex) {
            Log.e(LOG_TAG, "setDataSourceWithFd IOException:" + ex.getMessage());
        }
    }

    /**
     * prepare prepare the MediaPlayer
     *
     * @param id the key of MediaPlayer
     */
    public void prepare(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        try {
            mp.prepare();
            notifyInfo(id, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_PREPARED);
            notifyInfo(id, PLAYER_INFO_TYPE_DURATION_UPDATE, mp.getDuration());
        } catch (IOException ex) {
            Log.e(LOG_TAG, "prepare IOException:" + ex.getMessage());
        }
    }

    /**
     * prepareAsync prepare the MediaPlayer asynchronously
     *
     * @param id the key of MediaPlayer
     */
    public void prepareAsync(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mp.prepareAsync();
    }

    /**
     * play play the MediaPlayer
     *
     * @param id the key of MediaPlayer
     */
    public void play(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mp.start();
        notifyInfo(id, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_STARTED);
    }

    /**
     * pause pause the MediaPlayer
     *
     * @param id the key of MediaPlayer
     */
    public void pause(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mp.pause();
        notifyInfo(id, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_PAUSED);
    }

    /**
     * stop stop the MediaPlayer
     *
     * @param id the key of MediaPlayer
     */
    public void stop(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mp.stop();
        notifyInfo(id, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_STOPPED);
    }

    /**
     * reset reset the MediaPlayer
     *
     * @param id the id of MediaPlayer
     */
    public void reset(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mp.reset();
        notifyInfo(id, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_IDLE);
    }

    /**
     * release release the MediaPlayer
     *
     * @param id the id of MediaPlayer
     */
    public void release(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mediaPlayerMap.remove(id);
        mp.release();
        notifyInfo(id, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_RELEASED);
    }

    /**
     * seekTo seek to the specified time position
     *
     * @param id the id of MediaPlayer
     * @param msec the time position to seek to
     * @param mode the mode of seek
     */
    public void seekTo(long id, int msec, int mode) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mp.seekTo(msec, mode);
    }

    /**
     * setVolume set the volume of the MediaPlayer
     *
     * @param id the id of MediaPlayer
     * @param leftVolume the left volume
     * @param rightVolume the right volume
     */
    public void setVolume(long id, float leftVolume, float rightVolume) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mp.setVolume(leftVolume, rightVolume);
        nativeOnVolumnChanged(id, leftVolume);
    }

    /**
     * get track index
     *
     * @param id    player id
     * @param type  track type
     * @return track index
     */
    public int getTrackIndex(long id, int type) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return -1;
        }
        return mp.getSelectedTrack(type);
    }

    private String getTrackMime(MediaPlayer.TrackInfo info) {
        String str = info.toString();
        if (str.indexOf("mime=") != -1) {
            String subStr = str.substring(str.indexOf("mime="));
            return subStr.substring(PLAYER_CODEC_MIME_LENGTH, subStr.indexOf(","));
        }
        return "unknow";
    }

    /**
     * get track info string
     *
     * @param id    player id
     * @param type  track type
     * @param key   track key
     * @return track info string
     */
    public String getTrackInfoString(long id, int type, String key) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return "";
        }
        for (MediaPlayer.TrackInfo info : mp.getTrackInfo()) {
            if (info.getTrackType() == type) {
                if (PLAYER_TRACK_CODEC_MIME.equals(key)) {
                    return getTrackMime(info);
                }
                if (info.getFormat() != null) {
                    return info.getFormat().getString(key);
                }
                return "";
            }
        }
        return "";
    }

    /**
     * get looping
     *
     * @param id player id
     * @return looping
     */
    public boolean isLooping(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return false;
        }
        return mp.isLooping();
    }

    /**
     * set looping
     *
     * @param id player id
     * @param flag looping flag
     */
    public void setLooping(long id, boolean flag) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mp.setLooping(flag);
    }

    /**
     * get current position
     *
     * @param id player id
     * @return current position
     */
    public int getCurrentPosition(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return -1;
        }
        return mp.getCurrentPosition();
    }

    /**
     * get duration
     *
     * @param id player id
     * @return duration
     */
    public int getDuration(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return -1;
        }
        return mp.getDuration();
    }

    /**
     * setSurface set the surface of the MediaPlayer
     *
     * @param id the id of MediaPlayer
     * @param instanceId the instance id of AceSurfaceHolder
     * @param surfaceID the surface id of AceSurfaceHolder
     */
    public void setSurface(long id, int instanceId, long surfaceID) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }

        Class<?>[] classes = {int.class, long.class};
        Object result = invoke("ohos.ace.adapter.AceSurfaceHolder", "getSurface", classes, instanceId, surfaceID);
        if (result instanceof Surface) {
            mp.setSurface((Surface) result);
        }
    }

    private static Object invoke(String className, String methodName, Class<?>[] parameterTypes,
                                 Object... args) {
        Object value = null;

        try {
            Class<?> clz = Class.forName(className);
            Method method = clz.getMethod(methodName, parameterTypes);
            value = method.invoke(null, args);
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
     * get video width
     *
     * @param id player id
     * @return video width
     */
    public int getVideoWidth(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return 0;
        }
        return mp.getVideoWidth();
    }

    /**
     * get video height
     *
     * @param id player id
     * @return video height
     */
    public int getVideoHeight(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return 0;
        }
        return mp.getVideoHeight();
    }

    /**
     * Set the video scaling mode.
     *
     * @param id the player id
     * @param mode the video scaling mode
     */
    public void setVideoScalingMode(long id, int mode) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mp.setVideoScalingMode(mode);
    }

    private int convertSpeedToMode(float speed) {
        if (Float.compare(speed, 1.0f) < 0) {
            return 0;
        } else if (Float.compare(speed, 1.0f) == 0) {
            return 1;
        } else if (Float.compare(speed, 1.25f) <= 0) {
            return 2;
        } else if (Float.compare(speed, 1.75f) <= 0) {
            return 3;
        } else {
            return 4;
        }
    }

    private float convertModeToSpeed(int mode) {
        float speed = 1.0f;
        switch (mode) {
            case 0:
                speed = 0.75f;
                break;
            case 1:
                speed = 1.0f;
                break;
            case 2:
                speed = 1.25f;
                break;
            case 3:
                speed = 1.75f;
                break;
            case 4:
                speed = 2.0f;
                break;
            default:
                break;
        }
        return speed;
    }

    /**
     * Set the playback speed.
     *
     * @param id the player id
     * @param mode the playback speed
     */
    public void setPlaybackParams(long id, int mode) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        boolean isPlaying = mp.isPlaying();
        PlaybackParams param = mp.getPlaybackParams();
        param = param.setSpeed(convertModeToSpeed(mode));
        mp.setPlaybackParams(param);
        if (!isPlaying) {
            mp.pause();
        }
        notifyInfo(id, PLAYER_INFO_TYPE_SPEEDDONE, convertSpeedToMode(mp.getPlaybackParams().getSpeed()));
    }

    /**
     * Get the playback speed.
     *
     * @param id the player id
     * @return the playback speed
     */
    public int getPlaybackParams(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return 0;
        }
        return convertSpeedToMode(mp.getPlaybackParams().getSpeed());
    }

    /**
     * Select the bitrate.
     *
     * @param id the player id
     * @param bitrate the bitrate
     */
    public void selectBitrate(long id, int bitrate) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        for (MediaPlayer.TrackInfo info : mp.getTrackInfo()) {
            if (info.getTrackType() == MediaPlayer.TrackInfo.MEDIA_TRACK_TYPE_VIDEO) {
                if (info.getFormat() != null) {
                    info.getFormat().setInteger(MediaFormat.KEY_BIT_RATE, bitrate);
                    notifyInfo(id, PLAYER_INFO_TYPE_BITRATEDONE, bitrate);
                }
                return;
            }
        }
    }

    /**
     * Init PlayerJni jni.
     */
    protected native void nativeInit();

    /**
     * nativeOnInfo jni.
     *
     * @param key key
     * @param what what
     * @param extra extra
     */
    protected native void nativeOnInfo(long key, int what, int extra);

    /**
     * nativeOnError jni.
     *
     * @param key key
     * @param code code
     */
    protected native void nativeOnError(long key, int code);

    /**
     * nativeOnCompletion jni.
     *
     * @param key key
     * @param percent percent
     */
    protected native void nativeOnBufferingUpdate(long key, int percent);

    /**
     * nativeOnSeekComplete jni.
     *
     * @param key key
     * @param position position
     */
    protected native void nativeOnSeekComplete(long key, int position);

    /**
     * nativeOnVideoSizeChanged jni.
     *
     * @param key key
     * @param width width
     * @param height height
     */
    protected native void nativeOnVideoSizeChanged(long key, int width, int height);

    /**
     * nativeOnVolumnChanged jni.
     *
     * @param key key
     * @param vol vol
     */
    protected native void nativeOnVolumnChanged(long key, float vol);

    /**
     * nativeReadAt jni.
     *
     * @param key key
     * @param position position
     * @param buffer buffer
     * @param offset offset
     * @param size size
     * @return int
     */
    protected native int nativeReadAt(long key, long position, byte[] buffer, int offset, int size);
}
