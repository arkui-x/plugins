/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

package ohos.ace.plugin.downloadmanagerplugin;

import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ContentUris;
import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.util.Log;
import android.net.Uri;
import android.database.Cursor;
import android.webkit.MimeTypeMap;
import android.os.Handler;
import android.os.Looper;
import android.app.DownloadManager;

import java.io.File;
import java.net.URLConnection;
import java.util.HashMap;
import java.net.HttpURLConnection;
import java.net.URL;
import java.io.IOException;

/**
 * DownloadManagerPlugin
 *
 * @since 1
 */
public class DownloadManagerPlugin {
    private static final String LOG_TAG = "DownloadManagerPlugin";
    private static final int MAX_RETRY_TIMES = 3;
    private static final int DOWNLOAD_STATUS = 2;
    private static final int DOWNLOAD_COLUMN_REASON = 3;
    private static final int QUERY_PROGRESS_TIME = 1000;
    private static final int ARRAY_INIT_VAL = -1;
    private static final int PAUSE_BY_USER_STATUS = 193;
    private static final int RESUME_BY_USER_STATUS = 192;
    private static final int PAUSE_CONTROL_VALUE = 1;
    private static final int RESUME_CONTROL_VALUE = 0;
    private static final int DOWNLOAD_RECEIVED_SIZE_ARGC = 0;
    private static final int DOWNLOAD_TOTAL_SIZE_ARGC = 1;
    private static final int VISIBILITY_VISIBLE_NOTIFY_COMPLETED = 1;
    private static final int VISIBILITY_HIDDEN = 2;
    private static final int NETWORK_INVALID = 0;
    private static final int NETWORK_WIFI = 1;
    private static final int NETWORK_MOBILE = 2;

    private static Context context;
    private static int alreadyRetried;
    private static boolean isDownloading = false;

    private DownloadManager.Request request;
    private DownloadManager downloadManager;
    private BroadcastReceiver downloadCompleteReceiver;
    private DownloadConfig downloadConfig;
    private long downloadFileSize = 0L;
    private long downloadId = 0L;
    private long downloadProgressObj = 0L;
    private long networkObj = 0L;
    private String downloadUrl;
    private HashMap<String, String> downloadHeader;
    private boolean isDownloadEnableMetered;
    private boolean isDownloadEnableRoaming;
    private String downloadDescription;
    private int downloadNetworkType;
    private String downloadTitle;
    private boolean isDownloadBackground;
    private String downloadFilePath;
    private final Runnable runnable = new Runnable() {
        @Override
        public void run() {
            queryProgress();
            if (isDownloading) {
                handle.postDelayed(runnable, QUERY_PROGRESS_TIME);
            }
        }
    };

    private final Handler handle = new Handler(Looper.getMainLooper());

    /**
     * DownloadManagerPlugin
     *
     * @param context context of the application
     */
    public DownloadManagerPlugin(Context context) {
        if (context != null) {
            this.context = context;
        } else {
            Log.e(LOG_TAG, "context is null");
        }
        new DownloadManagerPlugin(context, true);
        nativeInit();
    }

    /**
     * DownloadManagerPlugin
     *
     * @param context context of the application
     * @param isNativeInit call nativeInit or not
     */
    public DownloadManagerPlugin(Context context, boolean isNativeInit) {
        if (context != null) {
            this.context = context;
        } else {
            Log.e(LOG_TAG, "context is null");
        }
        if (isNativeInit) {
            nativeInit();
        }
    }

    /**
     * DownloadConfig
     *
     * @since 1
     */
    public class DownloadConfig {
        private String url;
        private Object header;
        private boolean isEnableMetered;
        private boolean isEnableRoaming;
        private String description;
        private int networkType;
        private String filePath;
        private String title;
        private boolean isBackground;
    }

    private void registerReceiver() {
        downloadCompleteReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context ct, Intent intent) {
                String action = intent.getAction();
                if (DownloadManager.ACTION_NOTIFICATION_CLICKED.equals(action)) {
                    showDownloadProgress();
                }
            }
        };
        context.registerReceiver(downloadCompleteReceiver, new IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE));
    }

    /**
     * show download progress
     */
    public void showDownloadProgress() {
        Intent downloadIntent = new Intent(DownloadManager.ACTION_VIEW_DOWNLOADS);
        if (downloadIntent.resolveActivity(context.getPackageManager()) != null) {
            context.startActivity(downloadIntent);
        }
    }

    /**
     * get download url from download config
     *
     * @param url the HTTP or HTTPS url
     */
    public void getUrl(String url) {
        this.downloadUrl = url;
    }

    /**
     * get download header from download config
     *
     * @param keys the array of download header key
     * @param values the array of download header value
     */
    public void getHeader(String[] keys, String[] values) {
        this.downloadHeader = new HashMap<>();
        for (int i = 0; i < keys.length; i++) {
            this.downloadHeader.put(keys[i], values[i]);
        }
    }

    /**
     * Allows download in metered connection
     *
     * @param enableMetered metered network connection
     */
    public void getEnableMetered(boolean enableMetered) {
        this.isDownloadEnableMetered = enableMetered;
    }

    /**
     * Allows download in roaming network.
     *
     * @param enableRoaming roaming network connection
     */
    public void getEnableRoaming(boolean enableRoaming) {
        this.isDownloadEnableRoaming = enableRoaming;
    }

    /**
     * get download title from download config
     *
     * @param title Sets a download session title
     */
    public void getTitle(String title) {
        this.downloadTitle = title;
    }

    /**
     * get download network type from download config
     *
     * @param networkType Sets the network type allowed for download
     */
    public void getNetworkType(int networkType) {
        this.downloadNetworkType = networkType;
    }

    /**
     * get download file path from download config
     *
     * @param filePath Sets the path for downloads.
     */
    public void getFilePath(String filePath) {
        this.downloadFilePath = filePath;
    }

    /**
     * get download background from download config
     *
     * @param background Allow download background task notifications
     */
    public void getBackground(boolean background) {
        this.isDownloadBackground = background;
    }

    /**
     * get download description from download config
     *
     * @param description download description
     */
    public void getDescription(String description) {
        this.downloadDescription = description;
    }

    /**
     * get network state is wifi or mobile net
     *
     * @param networkObject network type object
     * @return network type
     */
    public int getNetworkState(long networkObject) {
        ConnectivityManager connectivityManager =
            (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        networkObj = networkObject;
        if (connectivityManager == null) {
            onRequestNetworkTypeCallback(NETWORK_INVALID, networkObj);
            return NETWORK_INVALID;
        }

        NetworkInfo activeNetInfo = connectivityManager.getActiveNetworkInfo();
        if (activeNetInfo == null || !activeNetInfo.isAvailable()) {
            Log.i(LOG_TAG, "current network is invalid");
            onRequestNetworkTypeCallback(NETWORK_INVALID, networkObj);
            return NETWORK_INVALID;
        }

        NetworkInfo wifiInfo = connectivityManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        if (wifiInfo != null) {
            NetworkInfo.State state = wifiInfo.getState();
            if (state != null) {
                if (state == NetworkInfo.State.CONNECTED || state == NetworkInfo.State.CONNECTING) {
                    Log.i(LOG_TAG, "current network is wifi!");
                    onRequestNetworkTypeCallback(NETWORK_WIFI, networkObj);
                    return NETWORK_WIFI;
                }
            }
        }

        NetworkInfo networkInfo = connectivityManager.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
        if (networkInfo != null) {
            NetworkInfo.State state = networkInfo.getState();
            if (state != null) {
                if (state == NetworkInfo.State.CONNECTED || state == NetworkInfo.State.CONNECTING) {
                    Log.i(LOG_TAG, "current network is mobile net!");
                    onRequestNetworkTypeCallback(NETWORK_MOBILE, networkObj);
                    return NETWORK_MOBILE;
                }
            }
        }
        onRequestNetworkTypeCallback(NETWORK_INVALID, networkObj);
        return NETWORK_INVALID;
    }

    /**
     * Start download with url
     *
     * @param downloadProgress download progress object
     * @return the download id
     */
    public long startDownload(long downloadProgress) {
        Log.i(LOG_TAG, "Download: start download manager service, downloadUrl: " + downloadUrl);
        if (getNetworkState(networkObj) == NETWORK_INVALID) {
            Log.e(LOG_TAG, "no network");
            sendFailCallback(16,0);
            return 0;
        }

        if (!canMakeRequest(downloadUrl)) {
            Log.e(LOG_TAG, "can not make request");
            sendFailCallback(16,1);
            return 0;
        }

        downloadManager = (DownloadManager) context.getSystemService(context.DOWNLOAD_SERVICE);
        if (!(downloadManager instanceof DownloadManager)) {
            Log.e(LOG_TAG, "no http or https url");
            return 0;
        }

        request = new DownloadManager.Request(Uri.parse(downloadUrl));
        request.setDescription(downloadDescription);
        request.setAllowedOverRoaming(isDownloadEnableRoaming);
        request.setAllowedOverMetered(isDownloadEnableMetered);
        request.setAllowedNetworkTypes(downloadNetworkType);
        request.setTitle(downloadTitle);
        request.setMimeType(MimeTypeMap.getSingleton().getMimeTypeFromExtension(
            MimeTypeMap.getFileExtensionFromUrl(downloadUrl)));
        int downloadNotify = isDownloadBackground ? VISIBILITY_VISIBLE_NOTIFY_COMPLETED : VISIBILITY_HIDDEN;
        request.setNotificationVisibility(downloadNotify);
        for (String key : downloadHeader.keySet()) {
            request.addRequestHeader(key, downloadHeader.get(key));
        }

        // set destination save file path
        String fileName = downloadFilePath.substring(downloadFilePath.lastIndexOf("/") + 1);
        File saveFile = new File(downloadFilePath, fileName);
        File parentFile = saveFile.getParentFile();
        request.setDestinationUri(Uri.fromFile(parentFile));

        downloadProgressObj = downloadProgress;
        downloadId = downloadManager.enqueue(request);
        Log.i(LOG_TAG, "Start to download task: " + downloadId);
        if (downloadId != 0) {
            startQueryProgress();
        }
        return downloadId;
    }

    /**
     * get download paused reason
     *
     * @param downloadColumnReason download paused reason
     */
    private void getPausedReason(int downloadColumnReason) {
        Log.i(LOG_TAG, "get download paused reason");
        switch (downloadColumnReason) {
            case DownloadManager.PAUSED_QUEUED_FOR_WIFI:
                Log.i(LOG_TAG, "the download is Waiting for WiFi");
                break;
            case DownloadManager.PAUSED_WAITING_FOR_NETWORK:
                Log.i(LOG_TAG, "the download is Waiting for Network");
                break;
            case DownloadManager.PAUSED_WAITING_TO_RETRY:
                Log.i(LOG_TAG, "the download is Waiting to retry");
                break;
            case DownloadManager.PAUSED_UNKNOWN:
                Log.i(LOG_TAG, "the download is paused for some other reason");
                break;
            default :
                break;
        }
    }

    /**
     * get download failed reason
     *
     * @param downloadColumnReason download failed reason
     */
    private void getFailedReason(int downloadColumnReason) {
        Log.i(LOG_TAG, "get download failed reason");
        switch (downloadColumnReason) {
            case DownloadManager.ERROR_CANNOT_RESUME:
                Log.e(LOG_TAG, "can't resume the download");
                break;
            case DownloadManager.ERROR_DEVICE_NOT_FOUND:
                Log.e(LOG_TAG, "no external storage device was found");
                break;
            case DownloadManager.ERROR_FILE_ALREADY_EXISTS:
                Log.e(LOG_TAG, "the requested destination file already exists");
                break;
            case DownloadManager.ERROR_FILE_ERROR:
                Log.e(LOG_TAG, "it doesn't fit under any other error code");
                break;
            case DownloadManager.ERROR_HTTP_DATA_ERROR:
                Log.e(LOG_TAG, "the error receiving or processing data occurred at the HTTP level");
                break;
            case DownloadManager.ERROR_INSUFFICIENT_SPACE:
                Log.e(LOG_TAG, "there was insufficient storage space.");
                break;
            case DownloadManager.ERROR_TOO_MANY_REDIRECTS:
                Log.e(LOG_TAG, "there were too many redirects");
                break;
            case DownloadManager.ERROR_UNHANDLED_HTTP_CODE:
                Log.e(LOG_TAG, "the HTTP was received that download manager can't handle");
                break;
            case DownloadManager.ERROR_UNKNOWN:
                Log.e(LOG_TAG, "the download has an error that doesn't fit under any other error code");
                break;
            default :
                Log.e(LOG_TAG, "error code: SESSION_UNKNOWN");
                break;
        }
    }

    /**
     * get download failed reason
     *
     * @return download size and total size Array
     */
    private int[] getDownloadBytes() {
        Cursor cursor = null;
        int[] bytes = new int[] {ARRAY_INIT_VAL, ARRAY_INIT_VAL, 0, 0};
        try {
            cursor = downloadManager.query(new DownloadManager.Query().setFilterById(downloadId));
            if (cursor != null && cursor.moveToFirst()) {
                bytes[DOWNLOAD_RECEIVED_SIZE_ARGC] =
                    cursor.getInt(cursor.getColumnIndex(DownloadManager.COLUMN_BYTES_DOWNLOADED_SO_FAR));
                bytes[DOWNLOAD_TOTAL_SIZE_ARGC] =
                    cursor.getInt(cursor.getColumnIndex(DownloadManager.COLUMN_TOTAL_SIZE_BYTES));
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return bytes;
    }

    /**
     * get download bytes callback
     *
     * @param bytesAndStatus the Array of saving download bytes and status
     */
    private void getDownloadBytesCallback(int[] bytesAndStatus) {
        int[] downloadBytes = getDownloadBytes();
        bytesAndStatus[DOWNLOAD_RECEIVED_SIZE_ARGC] = downloadBytes[DOWNLOAD_RECEIVED_SIZE_ARGC];
        bytesAndStatus[DOWNLOAD_TOTAL_SIZE_ARGC] = downloadBytes[DOWNLOAD_TOTAL_SIZE_ARGC];
        onRequestDataCallback(bytesAndStatus, downloadProgressObj);
    }

    /**
     * get download failed reason
     *
     * @param downloadStatus download status
     * @param bytesAndStatus the Array of saving download bytes and status
     */
    private void queryDownloadStatus(int downloadStatus, int[] bytesAndStatus) {
        Log.i(LOG_TAG, "query download status, download status: " + downloadStatus);
        String mimeType = downloadManager.getMimeTypeForDownloadedFile(downloadId);
        switch (downloadStatus) {
            case DownloadManager.STATUS_PAUSED:
                getPausedReason(bytesAndStatus[DOWNLOAD_COLUMN_REASON]);
                getDownloadBytesCallback(bytesAndStatus);
                break;
            case DownloadManager.STATUS_PENDING:
                if (alreadyRetried < MAX_RETRY_TIMES) {
                    alreadyRetried++;
                } else {
                    downloadManager.remove(downloadId);
                }
                break;
            case DownloadManager.STATUS_RUNNING:
                getDownloadBytesCallback(bytesAndStatus);
                onRequestMimeTypeCallback(mimeType, downloadProgressObj);
                break;
            case DownloadManager.STATUS_SUCCESSFUL:
                getDownloadBytesCallback(bytesAndStatus);
                onRequestMimeTypeCallback(mimeType, downloadProgressObj);
                stopQueryProgress();
                break;
            case DownloadManager.STATUS_FAILED:
                getFailedReason(bytesAndStatus[DOWNLOAD_COLUMN_REASON]);
                getDownloadBytesCallback(bytesAndStatus);
                break;
            default :
                break;
        }
    }

    /**
     * query download progress
     *
     * @return the downloadSize and totalSize or download status array
     */
    public int[] queryProgress() {
        int[] bytesAndStatus = new int[] {
            ARRAY_INIT_VAL, ARRAY_INIT_VAL, 0, 0
        };

        // begin to query status
        Cursor cursor = null;
        try {
            cursor = downloadManager.query(new DownloadManager.Query().setFilterById(downloadId));
            if (cursor != null && cursor.moveToFirst()) {
                bytesAndStatus[DOWNLOAD_STATUS] =
                    cursor.getInt(cursor.getColumnIndex(DownloadManager.COLUMN_STATUS));
                bytesAndStatus[DOWNLOAD_COLUMN_REASON] =
                    cursor.getInt(cursor.getColumnIndex(DownloadManager.COLUMN_REASON));
                Log.i(LOG_TAG, "reason: " + bytesAndStatus[DOWNLOAD_STATUS]);
                Log.i(LOG_TAG, "status: " + bytesAndStatus[DOWNLOAD_COLUMN_REASON]);
                queryDownloadStatus(bytesAndStatus[DOWNLOAD_STATUS], bytesAndStatus);
            } else {
                stopQueryProgress();
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return bytesAndStatus;
    }

    /**
     * begin to query progress
     */
    private void startQueryProgress() {
        isDownloading = true;
        handle.post(runnable);
    }

    /**
     * stop query progress
     */
    private void stopQueryProgress() {
        isDownloading = false;
        handle.removeCallbacks(runnable);
        alreadyRetried = 0;
    }

    public boolean isDownload() {
        return isDownloading;
    }

    /**
     * unregister downloadCompleteReceiver
     */
    public void onDestroy() {
        try {
            stopQueryProgress();
            context.unregisterReceiver(downloadCompleteReceiver);
        } catch (IllegalArgumentException error) {
            Log.e(LOG_TAG, "unregisterReceiver error: " + error.getMessage());
        }
    }

    /**
     * remove download and delete file
     */
    public void removeDownload() {
        downloadManager.remove(downloadId);
        DownloadManager.Query query = new DownloadManager.Query();
        query.setFilterById(downloadId);
        alreadyRetried = 0;
    }

    /**
     * judge whether support breakpoint continuation
     *
     * @return true or false
     */
    public boolean isSupportBreakpoint() {
        try {
            URL url = new URL(downloadUrl);
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            int responseCode = conn.getResponseCode();
            Log.i(LOG_TAG, "get network response code: " + responseCode);
            if (responseCode == HttpURLConnection.HTTP_OK) {
                String acceptRanges = conn.getHeaderField("Accept-Ranges");
                if (acceptRanges != null && acceptRanges.equals("bytes")) {
                    Log.i(LOG_TAG, "The url support breakpoint continuation");
                } else {
                    Log.e(LOG_TAG, "The url unsupport breakpoint continuation, download from the start location");
                    return false;
                }
            }
        } catch (IOException error) {
            Log.e(LOG_TAG, "URL connection failed, error: " + error.getMessage());
            return false;
        }
        return true;
    }

    /**
     * pause download
     */
    public void pauseDownload() {
        Log.i(LOG_TAG, "execute pauseDownload, downloadId: " + downloadId);
        ContentResolver contentResolver = context.getContentResolver();
        ContentValues contentValues = new ContentValues();
        int updateRows = 0;
        contentValues.put("control", PAUSE_CONTROL_VALUE); // pause control Value
        contentValues.put("status", PAUSE_BY_USER_STATUS); // PAUSED_BY_USER
        try {
            updateRows = contentResolver.update(ContentUris.withAppendedId(
                Uri.parse("content://downloads/my_downloads"), downloadId), contentValues, null, null);
        } catch (IllegalArgumentException error) {
            Log.e(LOG_TAG, "Failed to update control for downloading");
        }
    }

    /**
     * resume download
     *
     * @return true or false
     * @throws IllegalArgumentException
     */
    public boolean resumeDownload() {
        Log.i(LOG_TAG, "execute resumeDownload");
        if (!isSupportBreakpoint()) {
            startDownload(downloadProgressObj);
            return true;
        }
        ContentResolver contentResolver = context.getContentResolver();
        ContentValues contentValues = new ContentValues();
        int updateRows = 0;
        contentValues.put("control", RESUME_CONTROL_VALUE); // resume control Value
        contentValues.put("status", RESUME_BY_USER_STATUS); // RESUME_BY_USER
        try {
            updateRows = contentResolver.update(ContentUris.withAppendedId(
                Uri.parse("content://downloads/my_downloads"), downloadId), contentValues, null, null);
        } catch (IllegalArgumentException error) {
            Log.e(LOG_TAG, "Failed to update control for downloading");
        }
        return updateRows > 0;
    }

    public boolean canMakeRequest(String urlString) {
        try {
            URL url = new URL(urlString);
            URLConnection connection = url.openConnection();
            connection.setConnectTimeout(5000); // 设置超时时间，单位为ms
            connection.connect(); // 尝试连接
            return true;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
    }

    private void sendFailCallback(int status, int reason) {
        int[] bytesAndStatus = new int[]{
                ARRAY_INIT_VAL, ARRAY_INIT_VAL, 0, 0
        };

        bytesAndStatus[2] = status;
        bytesAndStatus[3] = reason;
        onRequestDataCallback(bytesAndStatus,downloadProgressObj);

    }


    /**
     * Register the initialization method of the plugin for the plugin constructor to call
     */
    protected native void nativeInit();

    /**
     * callback function in order to send download status and download progress to C++
     *
     * @param intArray the array of save download status and download/total size
     * @param downloadProgress download progress object
     */
    public native void onRequestDataCallback(int[] intArray, long downloadProgress);

    /**
     * callback function in order to send mimeType to C++
     *
     * @param mimeType mime type
     * @param downloadProgress download progress object
     */
    public native void onRequestMimeTypeCallback(String mimeType, long downloadProgress);

    /**
     * callback function in order to send mimeType to C++
     *
     * @param networkType network type
     * @param networkObject network type object
     */
    public native void onRequestNetworkTypeCallback(int networkType, long networkObject);
}