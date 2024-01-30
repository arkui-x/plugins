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

import android.content.Context;
import android.text.TextUtils;
import android.util.Log;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executors;

public class JavaTaskImpl {
    private DownloadImpl mDownloadImpl;
    public static Context mContext;

    public JavaTaskImpl(Context context) {
        mContext = context;
        mDownloadImpl = new DownloadImpl(mContext, this);
        reset();
    }

    public long create(String configJson) {
        Log.i(TAG, "create: configJson:" + configJson);
        Config config = JsonUtil.jsonToConfig(configJson);
        if (config == null) {
            Log.e(TAG, "create: config is null");
            return IConstant.FAILED_VALUE;
        }
        Log.i(TAG, "create: save taskInfo to database");
        CompletableFuture<Long> future = CompletableFuture.supplyAsync(() -> TaskDao.insert(mContext, config));
        try {
            long tid = future.get();
            Log.i(TAG, "create success:" + tid);
            return tid;
        } catch (Exception e) {
            Log.e(TAG, "create: error:", e);
        }
        return IConstant.FAILED_VALUE;
    }

    public void start(long taskId) {
        Log.i(TAG, "start: taskId:" + taskId);
        CompletableFuture<TaskInfo> future = CompletableFuture.supplyAsync(() -> TaskDao.query(mContext, taskId));
        TaskInfo taskInfo = null;
        try {
            taskInfo = future.get();
        } catch (Exception e) {
            Log.d(TAG, "start query error: " + e);
        }
        if (taskInfo == null) {
            Log.e(TAG, "start: task info is null");
            return;
        }
        if (taskInfo.getAction() == Action.DOWNLOAD) {
            mDownloadImpl.startDownload(taskInfo);
        } else {
            Log.e(TAG, "start: action is not download:" + taskInfo.getAction());
        }
    }

    public void resume(long taskId) {
        Log.i(TAG, "resume: " + taskId);
        CompletableFuture<TaskInfo> future = CompletableFuture.supplyAsync(() -> TaskDao.query(mContext, taskId));
        TaskInfo taskInfo = null;
        try {
            taskInfo = future.get();
        } catch (Exception e) {
            Log.d(TAG, "resume query error: " + e);
        }
        if (taskInfo == null) {
            Log.e(TAG, "resume: task info is null");
            return;
        }
        if (taskInfo.getAction() == Action.DOWNLOAD) {
            mDownloadImpl.resumeDownload(taskInfo);
        } else {
            Log.e(TAG, "resume: action is not download:" + taskInfo.getAction());
        }
    }

    public void pause(long taskId) {
        Log.i(TAG, "pause: " + taskId);
        CompletableFuture<TaskInfo> future = CompletableFuture.supplyAsync(() -> TaskDao.query(mContext, taskId));
        TaskInfo taskInfo = null;
        try {
            taskInfo = future.get();
        } catch (Exception e) {
            Log.d(TAG, "pause query error: " + e);
        }
        if (taskInfo == null) {
            Log.e(TAG, "pause: task info is null");
            return;
        }
        if (taskInfo.getAction() == Action.DOWNLOAD) {
            mDownloadImpl.pauseDownload(taskInfo);
        } else {
            Log.e(TAG, "pause: action is not download:" + taskInfo.getAction());
        }
    }

    public void stop(long taskId) {
        Log.i(TAG, "stop: " + taskId);
        CompletableFuture<TaskInfo> future = CompletableFuture.supplyAsync(() -> TaskDao.query(mContext, taskId));
        TaskInfo taskInfo = null;
        try {
            taskInfo = future.get();
        } catch (Exception e) {
            Log.d(TAG, "stop query error: " + e);
        }
        if (taskInfo == null) {
            Log.e(TAG, "stop: task info is null");
            return;
        }
        if (taskInfo.getAction() == Action.DOWNLOAD) {
            mDownloadImpl.removeDownload(taskInfo);
            mDownloadImpl.sendStopCallback(taskInfo);
        } else {
            Log.e(TAG, "stop: action is not download:" + taskInfo.getAction());
        }
    }

    public long remove(long taskId) {
        Log.i(TAG, "remove: " + taskId);
        CompletableFuture<TaskInfo> future = CompletableFuture.supplyAsync(() -> TaskDao.query(mContext, taskId));
        TaskInfo taskInfo = null;
        try {
            taskInfo = future.get();
        } catch (Exception e) {
            Log.d(TAG, "remove query error: " + e);
        }
        if (taskInfo == null) {
            Log.e(TAG, "remove: task info is null");
            return -1;
        }
        if (taskInfo.getAction() == Action.DOWNLOAD) {
            mDownloadImpl.removeDownload(taskInfo);
        } else {
            Log.e(TAG, "remove: action is not download:" + taskInfo.getAction());
        }
        mDownloadImpl.sendRemoveCallback(taskInfo);
        return 0;
    }

    public String getMimeType(long taskId) {
        Log.i(TAG, "getMimeType: " + taskId);
        CompletableFuture<TaskInfo> future = CompletableFuture.supplyAsync(() -> TaskDao.query(mContext, taskId));
        TaskInfo taskInfo = null;
        try {
            taskInfo = future.get();
        } catch (Exception e) {
            Log.d(TAG, "getMimeType query error: " + e);
        }
        if (taskInfo == null) {
            Log.e(TAG, "getMimeType: task info is null");
            return "";
        }
        return mDownloadImpl.getMimeType(taskInfo.getDownloadId());
    }

    public void reset() {
        //set status stopped which is not complete when launching app
        Executors.newCachedThreadPool().submit(() -> {
            List<TaskInfo> taskInfoList = TaskDao.queryAll(mContext);
            for (TaskInfo item : taskInfoList) {
                if (item.getProgress().getState() != State.STOPPED && item.getProgress().getState() != State.COMPLETED && item.getProgress().getState() != State.FAILED && item.getProgress().getState() != State.REMOVED) {
                    item.getProgress().setState(State.STOPPED);
                    mDownloadImpl.sendStopCallback(item);
                }
            }
        });
    }

    public void reportTaskInfo(String taskInfoJson) {
        Log.i(TAG, "reportTaskInfo: " + taskInfoJson);
        TaskInfo taskInfo = JsonUtil.jsonToTaskInfo(taskInfoJson);
        if (taskInfo == null) {
            Log.i(TAG, "reportTaskInfo: task info is null");
            return;
        }
        TaskDao.update(mContext, taskInfo, false);
    }

    public String show(long taskId) {
        Log.i(TAG, "show: " + taskId);
        mDownloadImpl.postQueryProgressByTid(taskId);
        CompletableFuture<TaskInfo> future = CompletableFuture.supplyAsync(() -> TaskDao.query(mContext, taskId));
        TaskInfo taskInfo = null;
        try {
            taskInfo = future.get();
        } catch (ExecutionException | InterruptedException e) {
            e.printStackTrace();
            Log.e(TAG, "show: query error", e);
        }
        if (taskInfo == null) {
            Log.e(TAG, "show: task is null");
            return "";
        }
        if (!TextUtils.isEmpty(taskInfo.getToken()) && !taskInfo.getToken().equals("null")) {
            Log.e(TAG, "show: token is not null");
            return "";
        }
        String result = JsonUtil.convertTaskInfoToJson(taskInfo);
        Log.d(TAG, "show: result:" + result);
        return result;
    }

    public String touch(long taskId, String token) {
        Log.i(TAG, "touch: " + taskId);
        mDownloadImpl.postQueryProgressByTid(taskId);
        CompletableFuture<TaskInfo> future = CompletableFuture.supplyAsync(() -> TaskDao.queryByToken(mContext, taskId, token));
        TaskInfo taskInfo = null;
        try {
            taskInfo = future.get();
        } catch (ExecutionException | InterruptedException e) {
            e.printStackTrace();
            Log.e(TAG, "touch: query error", e);
        }
        if (taskInfo == null) {
            Log.e(TAG, "touch: task is null");
            return "";
        }
        String result = JsonUtil.convertTaskInfoToJson(taskInfo);
        Log.d(TAG, "touch: result:" + result);
        return result;
    }

    public long[] search(String filterJson) {
        Log.i(TAG, "search: " + filterJson);
        mDownloadImpl.postQueryProgress();
        List<Long> taskIdList = new ArrayList<>();
        Filter filter = JsonUtil.jsonToFilter(filterJson);
        CompletableFuture<List<Long>> future = CompletableFuture.supplyAsync(() -> TaskDao.queryByFilter(mContext, filter));
        try {
            taskIdList = future.get();
        } catch (ExecutionException | InterruptedException e) {
            e.printStackTrace();
            Log.e(TAG, "search: error", e);
        }
        long[] taskIdArray = new long[taskIdList.size()];
        for (int i = 0; i < taskIdList.size(); i++) {
            taskIdArray[i] = taskIdList.get(i);
        }
        Log.i(TAG, "search result: " + Arrays.toString(taskIdArray));
        return taskIdArray;
    }

    public String getDefaultStoragePath() {
        File file = mContext.getCacheDir();
        if (file != null) {
            return file.getAbsolutePath();
        }
        return "";
    }

    public void jniInit() {
        Log.i(TAG, "jniInit: ");
        if (!IConstant.isAndroidDebug) {
            nativeInit();
        }
    }

    public void jniOnRequestCallback(long taskId, String eventType, String taskInfoJson) {
        Log.i(TAG, "jniOnRequestCallback: taskId:" + taskId + ",eventType:" + eventType + ",taskInfoJson:" + taskInfoJson);
        if (!IConstant.isAndroidDebug) {
            onRequestCallback(taskId, eventType, taskInfoJson);
        }
    }

    /**
     * Register the initialization method of the plugin for the plugin constructor to call
     */
    private native void nativeInit();

    private native void onRequestCallback(long taskId, String eventType, String taskInfoJson);
}
