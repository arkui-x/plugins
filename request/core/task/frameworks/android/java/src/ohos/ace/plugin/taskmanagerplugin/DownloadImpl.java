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

package ohos.ace.plugin.taskmanagerplugin;

import static ohos.ace.plugin.taskmanagerplugin.IConstant.TAG;

import android.annotation.SuppressLint;
import android.app.DownloadManager;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.graphics.drawable.Icon;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.webkit.MimeTypeMap;

import java.io.File;
import java.io.IOException;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executors;

/**
 * download manager
 *
 * @since 1
 */
@SuppressLint("Range")
public class DownloadImpl {
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
    private static final int NETWORK_INVALID = 0;
    private static final int NETWORK_WIFI = 1;
    private static final int NETWORK_MOBILE = 2;

    private final Context context;
    private DownloadManager.Request request;
    private DownloadManager downloadManager;

    private final List<QueryRunnable> queryRunnables = new ArrayList<>();

    private class QueryRunnable implements Runnable {
        private final TaskInfo taskInfo;
        private boolean isDownloading = false;
        private int retryNum = 0;

        public QueryRunnable(TaskInfo taskInfo) {
            this.taskInfo = taskInfo;
        }

        public void setDownloading(boolean downloading) {
            isDownloading = downloading;
        }

        @Override
        public void run() {
            queryProgress(this);
            if (isDownloading) {
                handle.postDelayed(this, QUERY_PROGRESS_TIME);
            }
        }
    }

    private final Handler handle = new Handler(Looper.getMainLooper());
    private JavaTaskImpl mJavaTaskImpl;

    public DownloadImpl(Context context, JavaTaskImpl javaTask) {
        this.context = context;
        mJavaTaskImpl = javaTask;
        mJavaTaskImpl.jniInit();
    }

    /**
     * get network state is wifi or mobile net
     *
     * @return network type
     */
    @SuppressLint("MissingPermission")
    public int getNetworkState() {
        ConnectivityManager connectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        if (connectivityManager == null) {
            return NETWORK_INVALID;
        }
        NetworkInfo activeNetInfo = connectivityManager.getActiveNetworkInfo();
        if (activeNetInfo == null || !activeNetInfo.isAvailable()) {
            Log.i(TAG, "current network is invalid");
            return NETWORK_INVALID;
        }

        NetworkInfo wifiInfo = connectivityManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        if (wifiInfo != null) {
            NetworkInfo.State state = wifiInfo.getState();
            if (state != null) {
                if (state == NetworkInfo.State.CONNECTED || state == NetworkInfo.State.CONNECTING) {
                    Log.i(TAG, "current network is wifi!");
                    return NETWORK_WIFI;
                }
            }
        }

        NetworkInfo networkInfo = connectivityManager.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
        if (networkInfo != null) {
            NetworkInfo.State state = networkInfo.getState();
            if (state != null) {
                if (state == NetworkInfo.State.CONNECTED || state == NetworkInfo.State.CONNECTING) {
                    Log.i(TAG, "current network is mobile net!");
                    return NETWORK_MOBILE;
                }
            }
        }
        return NETWORK_INVALID;
    }

    /**
     * Start download with url
     *
     * @return the download id
     */
    public void startDownload(TaskInfo taskInfo) {
        CompletableFuture<Config> configFuture = CompletableFuture.supplyAsync(() -> TaskDao.queryConfig(context, taskInfo.getTid()));
        Config config = null;
        try {
            config = configFuture.get();
        } catch (Exception e) {
            Log.e(TAG, "startDownload: error", e);
        }
        if (config == null) {
            Log.e(TAG, "startDownload: config is null");
            sendFailCallback(taskInfo, Reason.CONNECT_ERROR);
            return;
        }
        Log.i(TAG, "startDownload config: :" + JsonUtil.configToJson(config));
        int networkState = getNetworkState();
        Log.i(TAG, "networkState :" + networkState);
        if (networkState == NETWORK_INVALID) {
            Log.e(TAG, "no network");
            sendFailCallback(taskInfo, Reason.NETWORK_OFFLINE);
            return;
        }
        Config finalConfig = config;
        CompletableFuture<Boolean> future = CompletableFuture.supplyAsync(() -> canMakeRequest(finalConfig.getUrl()));
        boolean canMakeRequest = false;
        try {
            canMakeRequest = future.get();
        } catch (ExecutionException | InterruptedException e) {
            e.printStackTrace();
        }
        if (!canMakeRequest) {
            Log.d(TAG, "can not make request");
            sendFailCallback(taskInfo, Reason.CONNECT_ERROR);
            return;
        }
        downloadManager = (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);
        if (downloadManager == null) {
            Log.e(TAG, "no http or https url");
            return;
        }
        request = new DownloadManager.Request(Uri.parse(config.getUrl()));
        request.setDescription(config.getDescription());
        request.setAllowedOverRoaming(config.isRoaming());
        if (networkState == NETWORK_WIFI) {
            if (config.getNetwork() != Network.WIFI && config.getNetwork() != Network.ANY) {
                sendFailCallback(taskInfo, Reason.UNSUPPORTED_NETWORK_TYPE);
                return;
            }
            request.setAllowedOverMetered(true);
        } else if (networkState == NETWORK_MOBILE) {
            if (config.getNetwork() != Network.CELLULAR && config.getNetwork() != Network.ANY) {
                sendFailCallback(taskInfo, Reason.UNSUPPORTED_NETWORK_TYPE);
                return;
            }
            request.setAllowedOverMetered(config.isMetered());
        }
        request.setAllowedNetworkTypes(DownloadManager.Request.NETWORK_MOBILE | DownloadManager.Request.NETWORK_WIFI);
        request.setTitle(config.getTitle());
        String mimeType = MimeTypeMap.getSingleton().getMimeTypeFromExtension(MimeTypeMap.getFileExtensionFromUrl(config.getUrl()));
        request.setMimeType(mimeType);
        if (config.getMode() == Mode.FOREGROUND) {
            request.setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE);
        } else {
            request.setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED);
        }
        if (config.getHeaders() != null) {
            for (String key : config.getHeaders().keySet()) {
                request.addRequestHeader(key, config.getHeaders().get(key));
            }
        }
        // set destination save file path
        String downloadFilePath = config.getSaveas();
        if (downloadFilePath == null) {
            Log.e(TAG, "startDownload: saveas is null");
            sendFailCallback(taskInfo, Reason.USER_OPERATION);
            return;
        }
        String fileName = downloadFilePath.substring(downloadFilePath.lastIndexOf("/") + 1);
        File saveFile = new File(downloadFilePath, fileName);
        File parentFile = saveFile.getParentFile();
        if (parentFile != null) {
            Log.i(TAG, "createDownload: savePath:" + parentFile.getAbsolutePath());
            request.setDestinationUri(Uri.fromFile(parentFile));
            taskInfo.setMimeType(mimeType);
        } else {
            Log.e(TAG, "createDownload: file is null");
            sendFailCallback(taskInfo, Reason.IO_ERROR);
            return;
        }
        Log.i(TAG, "startDownload,savePath: " + taskInfo.getSaveas() + ",downloadUrl:" + taskInfo.getUrl());
        long downloadId = downloadManager.enqueue(request);
        Log.i(TAG, "Start to download task: " + downloadId);
        Progress progress = taskInfo.getProgress();
        if (progress == null) {
            progress = new Progress();
        }
        progress.setState(State.RUNNING);
        taskInfo.setProgress(progress);
        if (downloadId != 0) {
            taskInfo.setDownloadId(downloadId);
            startQueryProgress(taskInfo);
        }
        TaskDao.update(context, taskInfo, true);
    }

    /**
     * get download paused reason
     *
     * @param downloadColumnReason download paused reason
     */
    private String getPausedReason(int downloadColumnReason) {
        Log.i(TAG, "get download paused reason");
        switch (downloadColumnReason) {
            case DownloadManager.PAUSED_QUEUED_FOR_WIFI:
                Log.i(TAG, "the download is Waiting for WiFi");
                return Reason.UNSUPPORTED_NETWORK_TYPE;
            case DownloadManager.PAUSED_WAITING_FOR_NETWORK:
                Log.i(TAG, "the download is Waiting for Network");
                return Reason.WAITTING_NETWORK_ONE_DAY;
            case DownloadManager.PAUSED_WAITING_TO_RETRY:
                Log.i(TAG, "the download is Waiting to retry");
                return Reason.REQUEST_ERROR;
            case DownloadManager.PAUSED_UNKNOWN:
                Log.i(TAG, "the download is paused for some other reason");
                return Reason.OTHERS_ERROR;
            default:
                return Reason.OTHERS_ERROR;
        }
    }

    /**
     * get download failed reason
     *
     * @param downloadColumnReason download failed reason
     */
    private String getFailedReason(int downloadColumnReason) {
        Log.i(TAG, "get download failed reason");
        switch (downloadColumnReason) {
            case DownloadManager.ERROR_CANNOT_RESUME:
                Log.e(TAG, "can't resume the download");
                return Reason.REQUEST_ERROR;
            case DownloadManager.ERROR_DEVICE_NOT_FOUND:
                Log.e(TAG, "no external storage device was found");
                return Reason.BUILD_CLIENT_FAILED;
            case DownloadManager.ERROR_FILE_ALREADY_EXISTS:
                Log.e(TAG, "the requested destination file already exists");
                return Reason.IO_ERROR;
            case DownloadManager.ERROR_FILE_ERROR:
                Log.e(TAG, "it doesn't fit under any other error code");
                return Reason.IO_ERROR;
            case DownloadManager.ERROR_HTTP_DATA_ERROR:
                Log.e(TAG, "the error receiving or processing data occurred at the HTTP level");
                return Reason.PROTOCOL_ERROR;
            case DownloadManager.ERROR_INSUFFICIENT_SPACE:
                Log.e(TAG, "there was insufficient storage space.");
                return Reason.UNSUPPORT_RANGE_REQUEST;
            case DownloadManager.ERROR_TOO_MANY_REDIRECTS:
                Log.e(TAG, "there were too many redirects");
                return Reason.REDIRECT_ERROR;
            case DownloadManager.ERROR_UNHANDLED_HTTP_CODE:
                Log.e(TAG, "the HTTP was received that download manager can't handle");
                return Reason.CONNECT_ERROR;
            case DownloadManager.ERROR_UNKNOWN:
                Log.e(TAG, "the download has an error that doesn't fit under any other error code");
                return Reason.OTHERS_ERROR;
            default:
                Log.e(TAG, "error code: SESSION_UNKNOWN");
                return Reason.OTHERS_ERROR;
        }
    }

    /**
     * get download failed reason
     *
     * @return download size and total size Array
     */
    private int[] getDownloadBytes(long downloadId) {
        Cursor cursor = null;
        int[] bytes = new int[]{ARRAY_INIT_VAL, ARRAY_INIT_VAL, 0, 0};
        try {
            cursor = downloadManager.query(new DownloadManager.Query().setFilterById(downloadId));
            if (cursor != null && cursor.moveToFirst()) {
                bytes[DOWNLOAD_RECEIVED_SIZE_ARGC] = cursor.getInt(cursor.getColumnIndex(DownloadManager.COLUMN_BYTES_DOWNLOADED_SO_FAR));
                bytes[DOWNLOAD_TOTAL_SIZE_ARGC] = cursor.getInt(cursor.getColumnIndex(DownloadManager.COLUMN_TOTAL_SIZE_BYTES));
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return bytes;
    }

    /**
     * get download failed reason
     *
     * @param downloadStatus download status
     * @param bytesAndStatus the Array of saving download bytes and status
     */
    private void queryDownloadStatus(QueryRunnable queryRunnable, int downloadStatus, int[] bytesAndStatus) {
        Log.i(TAG, "query download status, download status: " + downloadStatus);
        Progress progress = queryRunnable.taskInfo.getProgress();
        switch (downloadStatus) {
            case DownloadManager.STATUS_PAUSED:
                Log.i(TAG, "query download status, download STATUS_PAUSED: " + DownloadManager.STATUS_PAUSED);
                if (progress.getState() != State.PAUSED && !statusIsFinish(progress.getState())) {
                    queryRunnable.taskInfo.setReason(getPausedReason(bytesAndStatus[DOWNLOAD_COLUMN_REASON]));
                    queryRunnable.taskInfo.setCode(getReasonCodeByReason(queryRunnable.taskInfo.getReason()));
                    String eventType = EventType.PAUSE;
                    if (bytesAndStatus[DOWNLOAD_COLUMN_REASON] == DownloadManager.PAUSED_WAITING_FOR_NETWORK &&
                            queryRunnable.taskInfo.getVersion() == Version.API10) {
                        progress.setState(State.FAILED);
                        eventType = EventType.FAILED;
                    } else {
                        progress.setState(State.PAUSED);
                        eventType = EventType.PAUSE;
                    }
                    int[] downloadBytes = getDownloadBytes(queryRunnable.taskInfo.getDownloadId());
                    progress.setProcessed(downloadBytes[DOWNLOAD_RECEIVED_SIZE_ARGC]);
                    List<Long> sizes = new ArrayList<>();
                    sizes.add((long) downloadBytes[DOWNLOAD_TOTAL_SIZE_ARGC]);
                    progress.setSizes(sizes);
                    mJavaTaskImpl.jniOnRequestCallback(queryRunnable.taskInfo.getTid(), eventType,
                            JsonUtil.convertTaskInfoToJson(queryRunnable.taskInfo));
                    TaskDao.update(context, queryRunnable.taskInfo, true);
                    if (progress.getState() == State.FAILED) {
                        removeDownload(queryRunnable.taskInfo);
                        stopQueryProgress(queryRunnable);
                    }
                }
                break;
            case DownloadManager.STATUS_PENDING:
                Log.i(TAG, "queryDownloadStatus: STATUS_PENDING,retryNum:" + queryRunnable.retryNum);
                if (queryRunnable.retryNum < 3) {
                    queryRunnable.retryNum++;
                } else {
                    stopQueryProgress(queryRunnable);
                    removeDownload(queryRunnable.taskInfo);
                    sendFailCallback(queryRunnable.taskInfo, Reason.OTHERS_ERROR);
                }
                break;
            case DownloadManager.STATUS_RUNNING:
                if (!statusIsFinish(progress.getState())) {
                    progress.setState(State.RUNNING);
                    int[] downloadBytes = getDownloadBytes(queryRunnable.taskInfo.getDownloadId());
                    progress.setProcessed(downloadBytes[DOWNLOAD_RECEIVED_SIZE_ARGC]);
                    List<Long> sizes = new ArrayList<>();
                    sizes.add((long) downloadBytes[DOWNLOAD_TOTAL_SIZE_ARGC]);
                    progress.setSizes(sizes);
                    mJavaTaskImpl.jniOnRequestCallback(queryRunnable.taskInfo.getTid(), EventType.PROGRESS, JsonUtil.convertTaskInfoToJson(queryRunnable.taskInfo));
                    TaskDao.update(context, queryRunnable.taskInfo, true);
                }
                break;
            case DownloadManager.STATUS_SUCCESSFUL:
                if (!statusIsFinish(progress.getState())) {
                    progress.setState(State.COMPLETED);
                    int[] downloadBytes = getDownloadBytes(queryRunnable.taskInfo.getDownloadId());
                    progress.setProcessed(downloadBytes[DOWNLOAD_RECEIVED_SIZE_ARGC]);
                    List<Long> sizes = new ArrayList<>();
                    sizes.add((long) downloadBytes[DOWNLOAD_TOTAL_SIZE_ARGC]);
                    progress.setSizes(sizes);
                    mJavaTaskImpl.jniOnRequestCallback(queryRunnable.taskInfo.getTid(), EventType.PROGRESS, JsonUtil.convertTaskInfoToJson(queryRunnable.taskInfo));
                    mJavaTaskImpl.jniOnRequestCallback(queryRunnable.taskInfo.getTid(), EventType.COMPLETED, JsonUtil.convertTaskInfoToJson(queryRunnable.taskInfo));
                    TaskDao.update(context, queryRunnable.taskInfo, true);
                }
                stopQueryProgress(queryRunnable);
                break;
            case DownloadManager.STATUS_FAILED:
                if (!statusIsFinish(progress.getState()) && progress.getState() != State.FAILED) {
                    progress.setState(State.FAILED);
                    queryRunnable.taskInfo.setReason(getFailedReason(bytesAndStatus[DOWNLOAD_COLUMN_REASON]));
                    queryRunnable.taskInfo.setCode(getReasonCodeByReason(queryRunnable.taskInfo.getReason()));
                    int[] downloadBytes = getDownloadBytes(queryRunnable.taskInfo.getDownloadId());
                    progress.setProcessed(downloadBytes[DOWNLOAD_RECEIVED_SIZE_ARGC]);
                    List<Long> sizes = new ArrayList<>();
                    sizes.add((long) downloadBytes[DOWNLOAD_TOTAL_SIZE_ARGC]);
                    progress.setSizes(sizes);
                    mJavaTaskImpl.jniOnRequestCallback(queryRunnable.taskInfo.getTid(), EventType.FAILED, JsonUtil.convertTaskInfoToJson(queryRunnable.taskInfo));
                    TaskDao.update(context, queryRunnable.taskInfo, true);
                }
                stopQueryProgress(queryRunnable);
                break;
            default:
                break;
        }

        Log.i(TAG, "queryDownloadStatus taskInfo:" + JsonUtil.convertTaskInfoToJson(queryRunnable.taskInfo));
    }

    /**
     * query download progress
     */
    public void queryProgress(QueryRunnable queryRunnable) {
        Log.i(TAG, "queryProgress: tid:" + queryRunnable.taskInfo.getTid() + ",downloadId:" + queryRunnable.taskInfo.getDownloadId());
        int[] bytesAndStatus = new int[]{ARRAY_INIT_VAL, ARRAY_INIT_VAL, 0, 0};

        // begin to query status
        try (Cursor cursor = downloadManager.query(new DownloadManager.Query().setFilterById(queryRunnable.taskInfo.getDownloadId()))) {
            if (cursor != null && cursor.moveToFirst()) {
                bytesAndStatus[DOWNLOAD_STATUS] = cursor.getInt(cursor.getColumnIndex(DownloadManager.COLUMN_STATUS));
                bytesAndStatus[DOWNLOAD_COLUMN_REASON] = cursor.getInt(cursor.getColumnIndex(DownloadManager.COLUMN_REASON));
                Log.i(TAG, "queryProgress status: " + bytesAndStatus[DOWNLOAD_STATUS]);
                queryDownloadStatus(queryRunnable, bytesAndStatus[DOWNLOAD_STATUS], bytesAndStatus);
            } else {
                stopQueryProgress(queryRunnable);
            }
        }
    }

    /**
     * begin to query progress
     */
    private void startQueryProgress(TaskInfo taskInfo) {
        QueryRunnable queryRunnable = new QueryRunnable(taskInfo);
        queryRunnable.setDownloading(true);
        queryRunnables.add(queryRunnable);
        handle.post(queryRunnable);
    }

    /**
     * stop query progress
     */
    public void stopQueryProgress(QueryRunnable queryRunnable) {
        queryRunnable.setDownloading(false);
        handle.removeCallbacks(queryRunnable);
        queryRunnables.remove(queryRunnable);
    }

    public void stopQueryProgress(TaskInfo taskInfo) {
        for (QueryRunnable qr :
                queryRunnables) {
            if (qr.taskInfo.getTid() == taskInfo.getTid()) {
                stopQueryProgress(qr);
                break;
            }
        }
    }

    public void postQueryProgressByTid(long tid) {
        Log.i(TAG, "postQueryProgressByTid: tid:" + tid);
        for (QueryRunnable item : queryRunnables) {
            if (item.taskInfo.getTid() == tid) {
                handle.removeCallbacks(item);
                handle.post(item);
                return;
            }
        }
    }

    public void postQueryProgress() {
        Log.i(TAG, "postQueryProgress");
        for (QueryRunnable item : queryRunnables) {
            handle.removeCallbacks(item);
            handle.post(item);
        }
    }

    /**
     * remove download and delete file
     */
    public void removeDownload(TaskInfo taskInfo) {
        Log.i(TAG, "removeDownload: " + taskInfo.getTid());
        if (downloadManager != null) {
            downloadManager.remove(taskInfo.getDownloadId());
            DownloadManager.Query query = new DownloadManager.Query();
            query.setFilterById(taskInfo.getDownloadId());
        }
    }

    /**
     * judge whether support breakpoint continuation
     *
     * @return true or false
     */
    public boolean isSupportBreakpoint(String downloadUrl) {
        try {
            URL url = new URL(downloadUrl);
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            int responseCode = conn.getResponseCode();
            Log.i(TAG, "get network response code: " + responseCode);
            if (responseCode == HttpURLConnection.HTTP_OK) {
                String acceptRanges = conn.getHeaderField("Accept-Ranges");
                if (acceptRanges != null && acceptRanges.equals("bytes")) {
                    Log.i(TAG, "The url support breakpoint continuation");
                } else {
                    Log.e(TAG, "The url unsupport breakpoint continuation, download from the start location");
                    return false;
                }
            }
        } catch (IOException error) {
            Log.e(TAG, "URL connection failed, error: " + error.getMessage());
            return false;
        }
        return true;
    }

    /**
     * pause download
     */
    public void pauseDownload(TaskInfo taskInfo) {
        Log.i(TAG, "execute pauseDownload, taskId: " + taskInfo.getTid() + ",downloadId:" + taskInfo.getDownloadId());
        stopQueryProgress(taskInfo);
        ContentResolver contentResolver = context.getContentResolver();
        ContentValues contentValues = new ContentValues();
        int updateRows = 0;
        contentValues.put("control", PAUSE_CONTROL_VALUE); // pause control Value
        contentValues.put("status", PAUSE_BY_USER_STATUS); // PAUSED_BY_USER
        try {
            updateRows = contentResolver.update(ContentUris.withAppendedId(Uri.parse("content://downloads/my_downloads"), taskInfo.getDownloadId()), contentValues, null, null);
            Log.i(TAG, "pauseDownload: updateRows:" + updateRows);
        } catch (IllegalArgumentException error) {
            Log.e(TAG, "Failed to update control for downloading");
        }
        startQueryProgress(taskInfo);
    }

    /**
     * resume download
     *
     * @return true or false
     * @throws IllegalArgumentException
     */
    public void resumeDownload(TaskInfo taskInfo) {
        boolean result = false;
        Log.i(TAG, "execute resumeDownload,tid:" + taskInfo.getTid() + ",downloadId:" + taskInfo.getDownloadId());
        if (!isSupportBreakpoint(taskInfo.getUrl())) {
            stopQueryProgress(taskInfo);
            startDownload(taskInfo);
            result = true;
        } else {
            ContentResolver contentResolver = context.getContentResolver();
            ContentValues contentValues = new ContentValues();
            int updateRows = 0;
            contentValues.put("control", RESUME_CONTROL_VALUE); // resume control Value
            contentValues.put("status", RESUME_BY_USER_STATUS); // RESUME_BY_USER
            try {
                updateRows = contentResolver.update(ContentUris.withAppendedId(Uri.parse("content://downloads/my_downloads"), taskInfo.getDownloadId()), contentValues, null, null);
            } catch (IllegalArgumentException error) {
                Log.e(TAG, "Failed to update control for downloading");
            }
            result = updateRows > 0;
        }
        if (result) {
            sendResumeCallback(taskInfo);
        }
    }

    public boolean canMakeRequest(String urlString) {
        Log.i(TAG, "Download: start download manager service, downloadUrl: " + urlString);
        try {
            URL url = new URL(urlString);
            URLConnection connection = url.openConnection();
            HttpURLConnection httpConnection = (HttpURLConnection) connection;
            httpConnection.setConnectTimeout(5000);
            httpConnection.connect();

            int responseCode = httpConnection.getResponseCode();
            Log.i(TAG, "Response Code: " + responseCode);

            if (responseCode == HttpURLConnection.HTTP_OK) {
                Log.i(TAG, "canMakeRequest success");
                return true;
            } else {
                Log.i(TAG, "canMakeRequest failed with response code: " + responseCode);
                return false;
            }
        } catch (Exception e) {
            e.printStackTrace();
            Log.i(TAG, "canMakeRequest failed due to exception");
            return false;
        }
    }

    private void sendFailCallback(TaskInfo taskInfo, String reason) {
        Progress progress = taskInfo.getProgress();
        if (progress.getState() != State.FAILED && !statusIsFinish(progress.getState())) {
            progress.setState(State.FAILED);
            List<Long> sizes = new ArrayList<>();
            sizes.add(-1L);
            progress.setSizes(sizes);
            taskInfo.setReason(reason);
            taskInfo.setCode(getReasonCodeByReason(taskInfo.getReason()));
            TaskDao.update(context, taskInfo, true);
            if (taskInfo.getVersion() == Version.API9) {
                Executors.newCachedThreadPool().submit(() -> {
                    try {
                        Thread.sleep(200);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                    mJavaTaskImpl.jniOnRequestCallback(taskInfo.getTid(), EventType.FAILED, JsonUtil.convertTaskInfoToJson(taskInfo));
                });
            } else {
                mJavaTaskImpl.jniOnRequestCallback(taskInfo.getTid(), EventType.FAILED, JsonUtil.convertTaskInfoToJson(taskInfo));
            }
        }
    }

    public void sendStopCallback(TaskInfo taskInfo) {
        Progress progress = taskInfo.getProgress();
        if (progress.getState() != State.STOPPED && !statusIsFinish(progress.getState())) {
            progress.setState(State.STOPPED);
            taskInfo.setReason(Reason.USER_OPERATION);
            taskInfo.setCode(getReasonCodeByReason(taskInfo.getReason()));
            TaskDao.update(context, taskInfo, true);
        }
        stopQueryProgress(taskInfo);
    }

    private void sendResumeCallback(TaskInfo taskInfo) {
        Progress progress = taskInfo.getProgress();
        if (progress.getState() != State.RUNNING && !statusIsFinish(progress.getState())) {
            progress.setState(State.RUNNING);
            mJavaTaskImpl.jniOnRequestCallback(taskInfo.getTid(), EventType.RESUME, JsonUtil.convertTaskInfoToJson(taskInfo));
            TaskDao.update(context, taskInfo, true);
        }
    }

    public void sendRemoveCallback(TaskInfo taskInfo) {
        Progress progress = taskInfo.getProgress();
        progress.setState(State.REMOVED);
        TaskDao.update(context,taskInfo,true);
        mJavaTaskImpl.jniOnRequestCallback(taskInfo.getTid(), EventType.REMOVE, JsonUtil.convertTaskInfoToJson(taskInfo));
        stopQueryProgress(taskInfo);
    }

    private boolean statusIsFinish(int status) {
        return status == State.COMPLETED || status == State.FAILED || status == State.STOPPED || status == State.REMOVED;
    }

    public String getMimeType(long downloadId) {
        return downloadManager.getMimeTypeForDownloadedFile(downloadId);
    }

    public int getReasonCodeByReason(String reason) {
        switch (reason) {
            case Reason.TASK_SURVIVAL_ONE_MONTH:
                return Reason.TASK_SURVIVAL_ONE_MONTH_CODE;
            case Reason.WAITTING_NETWORK_ONE_DAY:
                return Reason.WAITTING_NETWORK_ONE_DAY_CODE;
            case Reason.STOPPED_NEW_FRONT_TASK:
                return Reason.STOPPED_NEW_FRONT_TASK_CODE;
            case Reason.RUNNING_TASK_MEET_LIMITS:
                return Reason.RUNNING_TASK_MEET_LIMITS_CODE;
            case Reason.USER_OPERATION:
                return Reason.USER_OPERATION_CODE;
            case Reason.APP_BACKGROUND_OR_TERMINATE:
                return Reason.APP_BACKGROUND_OR_TERMINATE_CODE;
            case Reason.NETWORK_OFFLINE:
                return Reason.NETWORK_OFFLINE_CODE;
            case Reason.UNSUPPORTED_NETWORK_TYPE:
                return Reason.UNSUPPORTED_NETWORK_TYPE_CODE;
            case Reason.BUILD_CLIENT_FAILED:
                return Reason.BUILD_CLIENT_FAILED_CODE;
            case Reason.BUILD_REQUEST_FAILED:
                return Reason.BUILD_REQUEST_FAILED_CODE;
            case Reason.GET_FILESIZE_FAILED:
                return Reason.GET_FILESIZE_FAILED_CODE;
            case Reason.CONTINUOUS_TASK_TIMEOUT:
                return Reason.CONTINUOUS_TASK_TIMEOUT_CODE;
            case Reason.CONNECT_ERROR:
                return Reason.CONNECT_ERROR_CODE;
            case Reason.REQUEST_ERROR:
                return Reason.REQUEST_ERROR_CODE;
            case Reason.UPLOAD_FILE_ERROR:
                return Reason.UPLOAD_FILE_ERROR_CODE;
            case Reason.REDIRECT_ERROR:
                return Reason.REDIRECT_ERROR_CODE;
            case Reason.PROTOCOL_ERROR:
                return Reason.PROTOCOL_ERROR_CODE;
            case Reason.IO_ERROR:
                return Reason.IO_ERROR_CODE;
            case Reason.UNSUPPORT_RANGE_REQUEST:
                return Reason.UNSUPPORT_RANGE_REQUEST_CODE;
            case Reason.OTHERS_ERROR:
                return Reason.OTHERS_ERROR_CODE;
            default:
                return Reason.REASON_OK_CODE;
        }
    }
}