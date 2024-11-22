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
import android.net.Uri;
import android.os.Handler;
import android.util.Log;
import android.view.Surface;
import java.util.Map;
import java.util.HashMap;
import java.io.File;
import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Constructor;
import java.lang.Runnable;

/**
 * PlayerPlugin
 *
 * @since 1
 */
public class PlayerPlugin {
    private static final String LOG_TAG = "PlayerPlugin";

    private volatile Map<Long, MediaPlayer> mediaPlayerMap;
    private Map<Long, MediaDataSourceImpl> mediaDataSourceMap;

    public static final int PLAYER_INFO_TYPE_SPEEDDONE = 2;
    public static final int PLAYER_INFO_TYPE_BITRATEDONE = 3;
    public static final int PLAYER_INFO_TYPE_EOS = 4;
    public static final int PLAYER_INFO_TYPE_STATE_CHANGE = 5;
    public static final int PLAYER_INFO_TYPE_POSITION_UPDATE = 6;
    public static final int PLAYER_INFO_TYPE_DURATION_UPDATE = 15;

    public static final int PLAYER_STATE_ERROR = 0;
    public static final int PLAYER_STATE_IDLE = 1;
    public static final int PLAYER_STATE_INITIALIZED = 2;
    public static final int PLAYER_STATE_PREPARED = 4;
    public static final int PLAYER_STATE_STARTED = 5;
    public static final int PLAYER_STATE_PAUSED = 6;
    public static final int PLAYER_STATE_STOPPED = 7;
    public static final int PLAYER_STATE_PLAYBACK_COMPLETE = 8;
    public static final int PLAYER_STATE_RELEASED = 9;

    public static final int PLAYER_TIME_UPDATE_TIME_DELAY = 100;
    private static final String PLAYER_TRACK_CODEC_MIME = "codec_mime";
    private static final int PLAYER_CODEC_MIME_LENGTH = 5;
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

    public class PreparedCallbackImpl implements MediaPlayer.OnPreparedListener {
        @Override
        public void onPrepared (MediaPlayer mp) {
            long key = getMediaPlayerKey(mp);
            if (key != 0) {
                notifyInfo(key, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_PREPARED);
                notifyInfo(key, PLAYER_INFO_TYPE_DURATION_UPDATE, mp.getDuration());
                notifyInfo(key, PLAYER_INFO_TYPE_POSITION_UPDATE, 0);
            }
        }
    }

    public class CompletionCallbackImpl implements MediaPlayer.OnCompletionListener {
        @Override
        public void onCompletion (MediaPlayer mp) {
            long key = getMediaPlayerKey(mp);
            if (key != 0) {
                nativeOnInfo(key, PLAYER_INFO_TYPE_EOS, 0);
                nativeOnInfo(key, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_PLAYBACK_COMPLETE);
                nativeOnInfo(key, PLAYER_INFO_TYPE_POSITION_UPDATE, mp.getDuration());
            }
        }
    }

    public class SeekCompletionCallbackImpl implements MediaPlayer.OnSeekCompleteListener {
        @Override
        public void onSeekComplete (MediaPlayer mp) {
            long key = getMediaPlayerKey(mp);
            if (key != 0) {
                nativeOnSeekComplete(key, mp.getCurrentPosition());
                notifyInfo(key, PLAYER_INFO_TYPE_POSITION_UPDATE, mp.getCurrentPosition());
            }
        }
    }

    public class BufferingUpdateCallbackImpl implements MediaPlayer.OnBufferingUpdateListener {
        @Override
        public void onBufferingUpdate(MediaPlayer mp, int percent) {
            long key = getMediaPlayerKey(mp);
            if (key != 0) {
                nativeOnBufferingUpdate(key, percent);
            }
        }
    }

    public class VideoSizeChangedCallbackImpl implements MediaPlayer.OnVideoSizeChangedListener {
        @Override
        public void onVideoSizeChanged(MediaPlayer mp, int width, int height) {
            long key = getMediaPlayerKey(mp);
            if (key != 0) {
                nativeOnVideoSizeChanged(key, width, height);
            }
        }
    }

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
        public void close () {
        }
    };

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

    public void releaseMediaPlayer(long key) {
        mediaPlayerMap.remove(key);
    }

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

    public void setDataSourceWithUrl(long id, String url) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        try{
            mp.setDataSource(url);
            notifyInfo(id, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_INITIALIZED);
        } catch(IOException ex){
            Log.e(LOG_TAG, "setDataSourceWithUrl IOException:" + ex.getMessage());
        }
    }

    public void setDataSourceWithFd(long id, String url, long offset, long length) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        try{
            if (length <= 0) {
                File file = new File(url);
                length = file.length();
            }
            FileInputStream fs = new FileInputStream(url);

            mp.setDataSource(fs.getFD(), offset, length);
            notifyInfo(id, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_INITIALIZED);
        } catch(IOException ex){
            Log.e(LOG_TAG, "setDataSourceWithFd IOException:" + ex.getMessage());
        }
    }

    public void prepare(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        try{
            mp.prepare();
            notifyInfo(id, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_PREPARED);
            notifyInfo(id, PLAYER_INFO_TYPE_DURATION_UPDATE, mp.getDuration());
        } catch(IOException ex){
            Log.e(LOG_TAG, "prepare IOException:" + ex.getMessage());
        }
    }

    public void prepareAsync(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mp.prepareAsync();
    }

    public void play(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mp.start();
        notifyInfo(id, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_STARTED);
    }

    public void pause(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mp.pause();
        notifyInfo(id, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_PAUSED);
    }

    public void stop(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mp.stop();
        notifyInfo(id, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_STOPPED);
    }

    public void reset(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mp.reset();
        notifyInfo(id, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_IDLE);
    }

    public void release(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mediaPlayerMap.remove(id);
        mp.release();
        notifyInfo(id, PLAYER_INFO_TYPE_STATE_CHANGE, PLAYER_STATE_RELEASED);
    }

    public void seekTo(long id, int msec, int mode) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mp.seekTo(msec, mode);
    }

    public void setVolume(long id, float leftVolume, float rightVolume) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mp.setVolume(leftVolume, rightVolume);
        nativeOnVolumnChanged(id, leftVolume);
    }

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

    public String getTrackInfoString(long id, int type, String key) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return "";
        }
        for (MediaPlayer.TrackInfo info : mp.getTrackInfo()) {
            if (info.getTrackType() == type) {
                if (key.equals(PLAYER_TRACK_CODEC_MIME)) {
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

    public boolean isLooping(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return false;
        }
        return mp.isLooping();
    }

    public void setLooping(long id, boolean flag) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mp.setLooping(flag);
    }

    public int getCurrentPosition(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return -1;
        }
        return mp.getCurrentPosition();
    }

    public int getDuration(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return -1;
        }
        return mp.getDuration();
    }

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

    public int getVideoWidth(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return 0;
        }
        return mp.getVideoWidth();
    }

    public int getVideoHeight(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return 0;
        }
        return mp.getVideoHeight();
    }

    public void setVideoScalingMode(long id, int mode) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        mp.setVideoScalingMode(mode);
    }

    private int ConvertSpeedToMode(float speed)
    {
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

    private float ConvertModeToSpeed(int mode)
    {
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

    public void setPlaybackParams(long id, int mode) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return;
        }
        boolean isPlaying = mp.isPlaying();
        PlaybackParams param = mp.getPlaybackParams();
        param = param.setSpeed(ConvertModeToSpeed(mode));
        mp.setPlaybackParams(param);
        if (!isPlaying) {
            mp.pause();
        }
        notifyInfo(id, PLAYER_INFO_TYPE_SPEEDDONE, ConvertSpeedToMode(mp.getPlaybackParams().getSpeed()));
    }

    public int getPlaybackParams(long id) {
        MediaPlayer mp = getMediaPlayerById(id);
        if (mp == null) {
            return 0;
        }
        return ConvertSpeedToMode(mp.getPlaybackParams().getSpeed());
    }

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
     *
     * @return void
     */
    protected native void nativeInit();
    protected native void nativeOnInfo(long key, int what, int extra);
    protected native void nativeOnError(long key, int code);
    protected native void nativeOnBufferingUpdate(long key, int percent);
    protected native void nativeOnSeekComplete(long key, int position);
    protected native void nativeOnVideoSizeChanged(long key, int width, int height);
    protected native void nativeOnVolumnChanged(long key, float vol);
    protected native int nativeReadAt(long key, long position, byte[] buffer, int offset, int size);
}
