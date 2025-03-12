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

package ohos.ace.plugin.avmetadatahelperplugin;

import android.content.Context;
import android.util.Log;
import java.util.Map;
import java.util.HashMap;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import android.media.MediaDataSource;
import android.media.MediaMetadataRetriever;

/**
 * AVMetadataHelperPlugin
 *
 * @since 1
 */
public class AVMetadataHelperPlugin {
    /**
     * nativeInit is used to initialize the native library.
     */
    private static final String LOG_TAG = "AVMetadataHelperPlugin";

    /**
     * HELPER_STATE_ERROR is used to indicate the state of the helper is error.
     */
    public static final int HELPER_STATE_ERROR = 0;

    /**
     * HELPER_IDLE is used to indicate the state of the helper is idle.
     */
    public static final int HELPER_IDLE = 1;

    /**
     * HELPER_PREPARING is used to indicate the state of the helper is preparing.
     */
    public static final int HELPER_PREPARED = 2;

    /**
     * HELPER_CALL_DONE is used to indicate the call is done.
     */
    public static final int HELPER_CALL_DONE = 3;

    /**
     * HELPER_RELEASED is used to indicate the state of the helper is released.
     */
    public static final int HELPER_RELEASED = 4;

    private Map<Long, MediaMetadataRetriever> metadataRetrieverMap = new HashMap<>();
    private Map<Long, MediaDataSourceImpl> dataSourceMap = new HashMap<>();

    /**
     * AVMetadataHelperPlugin
     *
     * @param context context of the application
     */
    public AVMetadataHelperPlugin(Context context) {
        if (context == null) {
            Log.e(LOG_TAG, "context is null");
            return;
        }
        nativeInit();
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

    private class CallbackThread extends Thread {
        private long id;
        private int state;

        @Override
        public void run() {
            nativeOnStateChanged(id, state);
        }

        public void setParams(long key, int info) {
            id = key;
            state = info;
        }
    }

    /**
     * creates a MediaMetadataRetriever object.
     *
     * @param key The key of the MediaMetadataRetriever.
     */
    public void createMetadataRetriever(long key) {
        MediaMetadataRetriever metadataRetriever = metadataRetrieverMap.get(key);
        if (metadataRetriever != null) {
            Log.e(LOG_TAG, "createMetadataRetriever metadataRetriever exist.");
            return;
        }

        metadataRetriever = new MediaMetadataRetriever();
        metadataRetrieverMap.put(key, metadataRetriever);
        notifyInfo(key, HELPER_IDLE);
    }

    private void notifyInfo(long key, int info) {
        CallbackThread td = new CallbackThread();
        td.setParams(key, info);
        td.start();
    }


    /**
     * releases a MediaMetadataRetriever object.
     *
     * @param key The key of the MediaMetadataRetriever.
     */
    public void releaseMetadataRetriever(long key) {
        metadataRetrieverMap.remove(key);
    }

    /**
     * Sets the data source (FileDescriptor) to use.
     *
     * @param key The key of the MediaMetadataRetriever.
     * @param url the url for the file you want to play.
     * @param offset the offset into the file where the data to be played starts, in bytes. It must be non-negative
     * @param size the length in bytes of the data to be played. The negative value is invalid.
     */
    public void setDataSource(long key, String url, long offset, long size) {
        MediaMetadataRetriever metadataRetriever = metadataRetrieverMap.get(key);
        if (metadataRetriever == null) {
            Log.e("LOG_TAG", "MediaMetadataRetriever is not created, setDataSource(fd).");
            return;
        }
        try {
            if (size <= 0) {
                File file = new File(url);
                size = file.length();
            }
            FileInputStream fs = new FileInputStream(url);
            metadataRetriever.setDataSource(fs.getFD(), offset, size);
            notifyInfo(key, HELPER_PREPARED);
        } catch (IOException ex) {
            Log.e(LOG_TAG, "setDataSource IOException:" + ex.getMessage());
        }
    }

    /**
     * Sets the data source (file pathname) to use.
     *
     * @param key The key of the MediaMetadataRetriever.
     */
    public void setDataSource(long key) {
        MediaMetadataRetriever metadataRetriever = metadataRetrieverMap.get(key);
        if (metadataRetriever == null) {
            Log.e("LOG_TAG", "MediaMetadataRetriever is not created, setDataSource(dataSource).");
            return;
        }
        MediaDataSourceImpl dataSource = new MediaDataSourceImpl(key);
        dataSourceMap.put(key, dataSource);
        metadataRetriever.setDataSource(dataSource);
        notifyInfo(key, HELPER_PREPARED);
    }

    /**
     * Retrieves the meta data value associated with the keyCode.
     *
     * @param key The key of the MediaMetadataRetriever.
     * @param keyCode The key of metadata.
     */
    public String extractMetadata(long key, int keyCode) {
        MediaMetadataRetriever metadataRetriever = metadataRetrieverMap.get(key);
        if (metadataRetriever == null) {
            Log.e("LOG_TAG", "MediaMetadataRetriever is not created, extractMetadata.");
            return "";
        }
        String ret = metadataRetriever.extractMetadata(keyCode);
        return ret == null ? "" : ret;
    }

    /**
     * Retrieves the optional graphic or album/cover art associated associated with the data source.
     *
     * @param key The key of the MediaMetadataRetriever.
     */
    public void extractMetadataDone(long key) {
        notifyInfo(key, HELPER_CALL_DONE);
    }

    /**
     * Retrieves the optional graphic or album/cover art associated associated with the data source.
     *
     * @param key The key of the MediaMetadataRetriever.
     * @return The size of the embedded picture.
     */
    public int getEmbeddedPictureSize(long key) {
        MediaMetadataRetriever metadataRetriever = metadataRetrieverMap.get(key);
        if (metadataRetriever == null) {
            Log.e("LOG_TAG", "MediaMetadataRetriever is not created, getEmbeddedPicture.");
            return -1;
        }
        byte[] pic = metadataRetriever.getEmbeddedPicture();
        if (pic == null) {
            notifyInfo(key, HELPER_CALL_DONE);
            return -1;
        } else {
            return pic.length;
        }
    }

    /**
     * Finds the optional graphic or album/cover art associated associated with the data source..
     *
     * @param key The key of the MediaMetadataRetriever.
     */
    public byte[] getEmbeddedPicture(long key) {
        MediaMetadataRetriever metadataRetriever = metadataRetrieverMap.get(key);
        if (metadataRetriever == null) {
            Log.e("LOG_TAG", "MediaMetadataRetriever is not created, getEmbeddedPicture.");
            return null;
        }
        notifyInfo(key, HELPER_CALL_DONE);
        return metadataRetriever.getEmbeddedPicture();
    }

    /**
     * releases a MediaMetadataRetriever object.
     *
     * @param key The key of the MediaMetadataRetriever.
     */
    public void release(long key) {
        MediaMetadataRetriever metadataRetriever = metadataRetrieverMap.get(key);
        if (metadataRetriever == null) {
            Log.e("LOG_TAG", "MediaMetadataRetriever is not created, release.");
            return;
        }
        metadataRetriever.release();
        notifyInfo(key, HELPER_RELEASED);
    }

    /**
     * Init AVMetadataHelperPlugin jni.
     *
     */
    protected native void nativeInit();

    /**
     * Read the media data source.
     *
     * @param key The key of the MediaMetadataRetriever.
     * @param position The position in the data source to read from.
     * @param buffer The buffer to read the data into.
     * @param offset The offset within buffer to read the data into.
     * @param size The number of bytes to read.
     * @return int The number of bytes read, or -1 if end of stream is reached.
     */
    protected native int nativeReadAt(long key, long position, byte[] buffer, int offset, int size);

    /**
     * Notify the state of MediaMetadataRetriever changed.
     *
     * @param key The key of the MediaMetadataRetriever.
     * @param state The state of the MediaMetadataRetriever.
     * @return void.
     */
    protected native void nativeOnStateChanged(long key, int state);
}
