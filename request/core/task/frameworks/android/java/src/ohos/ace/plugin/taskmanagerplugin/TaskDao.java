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
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.text.TextUtils;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * TaskDao class is used to operate database
 *
 * @since 2024-05-31
 */
public class TaskDao {
    private static DatabaseHelper mDatabaseHelper;
    private static SQLiteDatabase mSQLiteDatabase;

    /**
     * init database
     *
     * @param context context
     * @param config config
     * @return long
     */
    public static long insert(Context context, Config config) {
        Log.i(TAG, "insert:" + JsonUtil.configToJson(config));
        initDb(context);
        ContentValues values = new ContentValues();
        values.put("saveas", config.getSaveas());
        values.put("url", config.getUrl());
        values.put("data", config.getData());
        values.put("title", config.getTitle());
        values.put("description", config.getDescription());
        values.put("action1", config.getAction());
        values.put("mode", config.getMode());
        values.put("mimeType", config.getMimeType());
        Progress progress = new Progress();
        progress.setState(State.INITIALIZED);
        values.put("progress", JsonUtil.convertProgressToJson(progress));
        values.put("ctime", System.currentTimeMillis());
        values.put("mtime", System.currentTimeMillis());
        values.put("faults", Faults.OTHERS);
        values.put("reason", "");
        values.put("downloadId", "");
        values.put("token", config.getToken());
        values.put("taskStates", "");
        // config's filed
        values.put("roaming", config.isRoaming() ? 1 : 0);
        values.put("metered", config.isMetered() ? 1 : 0);
        values.put("network", config.getNetwork());
        values.put("headers", JsonUtil.mapStringToJson(config.getHeaders()));
        values.put("index1", config.getIndex());
        values.put("begins", config.getBegins());
        values.put("ends", config.getEnds());
        values.put("priority", config.getPriority());
        values.put("overwrite", config.isOverwrite() ? 1 : 0);
        values.put("retry", config.isRetry() ? 1 : 0);
        values.put("redirect", config.isRedirect() ? 1 : 0);
        values.put("gauge", config.isGauge() ? 1 : 0);
        values.put("precise", config.isPrecise() ? 1 : 0);
        values.put("background", config.isBackground() ? 1 : 0);
        values.put("method", config.getMethod());
        values.put("forms", config.getForms());
        values.put("files", config.getFiles());
        values.put("bodyFds", config.getBodyFds());
        values.put("bodyFileNames", config.getBodyFileNames());
        values.put("extras", config.getExtras());
        values.put("version", config.getVersion());
        values.put("tries", 0);
        values.put("code", 0);
        values.put("withSystem", 0);
        try {
            long insertResult = mSQLiteDatabase.insert("Task", null, values);
            Log.i(TAG, "insert: " + insertResult);
            return insertResult;
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "insert: error", e);
        }
        return IConstant.FAILED_VALUE;
    }

    /**
     * query config by taskId
     *
     * @param context context
     * @param taskId  taskId
     * @return config
     */
    @SuppressLint("Range")
    public static Config queryConfig(Context context, long taskId) {
        Log.i(TAG, "queryConfig: " + taskId);
        initDb(context);
        String[] selectionArgs = {String.valueOf(taskId)};
        try (Cursor cursor = mSQLiteDatabase.query("Task", null, "tid=?", selectionArgs, null, null, null)) {
            if (cursor != null && cursor.moveToFirst()) {
                String url = cursor.getString(cursor.getColumnIndex("url"));
                String description = cursor.getString(cursor.getColumnIndex("description"));
                boolean roaming = cursor.getInt(cursor.getColumnIndex("roaming")) == 1;
                boolean metered = cursor.getInt(cursor.getColumnIndex("metered")) == 1;
                int network = cursor.getInt(cursor.getColumnIndex("network"));
                int mode = cursor.getInt(cursor.getColumnIndex("mode"));
                String title = cursor.getString(cursor.getColumnIndex("title"));
                String saveas = cursor.getString(cursor.getColumnIndex("saveas"));
                String headersJson = cursor.getString(cursor.getColumnIndex("headers"));
                HashMap<String, String> headers = JsonUtil.jsonToMapString(headersJson);
                Config config = new Config();
                config.setTaskId(taskId);
                config.setUrl(url);
                config.setDescription(description);
                config.setRoaming(roaming);
                config.setMetered(metered);
                config.setNetwork(network);
                config.setMode(mode);
                config.setTitle(title);
                config.setSaveas(saveas);
                config.setHeaders(headers);
                boolean background = cursor.getInt(cursor.getColumnIndex("background")) == 1;
                config.setBackground(background);
                return config;
            }
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "query: error", e);
        }
        return null;
    }

    /**
     * query task by taskId
     *
     * @param context context
     * @param taskId  taskId
     * @return task
     */
    @SuppressLint("Range")
    public static TaskInfo query(Context context, long taskId) {
        Log.i(TAG, "query: " + taskId);
        initDb(context);
        String[] selectionArgs = {String.valueOf(taskId)};
        List<TaskInfo> taskInfos = new ArrayList<>();
        Cursor cursor = null;
        try {
            cursor = mSQLiteDatabase.query("Task", null, "tid=?", selectionArgs, null, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                do {
                    TaskInfo taskInfo = getTaskInfoByCursor(cursor);
                    taskInfos.add(taskInfo);
                } while (cursor.moveToNext());
            }
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "query: error", e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        if (!taskInfos.isEmpty()) {
            return taskInfos.get(0);
        }
        return null;
    }

    /**
     * query all tasks
     *
     * @param context context
     * @return tasks
     */
    @SuppressLint("Range")
    public static List<TaskInfo> queryAll(Context context) {
        Log.i(TAG, "queryAll");
        initDb(context);
        List<TaskInfo> taskInfos = new ArrayList<>();
        Cursor cursor = null;
        try {
            cursor = mSQLiteDatabase.query("Task", null, null, null, null, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                do {
                    TaskInfo taskInfo = getTaskInfoByCursor(cursor);
                    taskInfos.add(taskInfo);
                } while (cursor.moveToNext());
            }
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "query: error", e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return taskInfos;
    }

    /**
     * query task by taskId and token
     *
     * @param context context
     * @param taskId  taskId
     * @param token   token
     * @return task
     */
    @SuppressLint("Range")
    public static TaskInfo queryByToken(Context context, long taskId, String token) {
        Log.i(TAG, "queryByToken: " + taskId + ",token:" + token);
        initDb(context);
        String[] selectionArgs = {String.valueOf(taskId), token};
        List<TaskInfo> taskInfos = new ArrayList<>();
        try (Cursor cursor = mSQLiteDatabase.query("Task", null, "tid=? and token=?", selectionArgs, null, null,
                null)) {
            if (cursor != null && cursor.moveToFirst()) {
                do {
                    TaskInfo taskInfo = getTaskInfoByCursor(cursor);
                    taskInfos.add(taskInfo);
                } while (cursor.moveToNext());
            }
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "queryByToken: error", e);
        }
        if (!taskInfos.isEmpty()) {
            return taskInfos.get(0);
        }
        return null;
    }

    @SuppressLint("Range")
    private static TaskInfo getTaskInfoByCursor(Cursor cursor) {
        long tid = cursor.getLong(cursor.getColumnIndex("tid"));
        String saveas = cursor.getString(cursor.getColumnIndex("saveas"));
        String url = cursor.getString(cursor.getColumnIndex("url"));
        String data = cursor.getString(cursor.getColumnIndex("data"));
        String title = cursor.getString(cursor.getColumnIndex("title"));
        String description = cursor.getString(cursor.getColumnIndex("description"));
        int action = cursor.getInt(cursor.getColumnIndex("action1"));
        int mode = cursor.getInt(cursor.getColumnIndex("mode"));
        String mimeType = cursor.getString(cursor.getColumnIndex("mimeType"));
        String progress = cursor.getString(cursor.getColumnIndex("progress"));
        String reason = cursor.getString(cursor.getColumnIndex("reason"));
        String token1 = cursor.getString(cursor.getColumnIndex("token"));
        String taskStates = cursor.getString(cursor.getColumnIndex("taskStates"));
        long ctime = cursor.getLong(cursor.getColumnIndex("ctime"));
        long mtime = cursor.getLong(cursor.getColumnIndex("mtime"));
        long downloadId = cursor.getLong(cursor.getColumnIndex("downloadId"));
        int faults = cursor.getInt(cursor.getColumnIndex("faults"));
        int version = cursor.getInt(cursor.getColumnIndex("version"));
        String files = cursor.getString(cursor.getColumnIndex("files"));
        String forms = cursor.getString(cursor.getColumnIndex("forms"));
        boolean gauge = cursor.getInt(cursor.getColumnIndex("gauge")) == 1;
        boolean retry = cursor.getInt(cursor.getColumnIndex("retry")) == 1;
        boolean withSystem = cursor.getInt(cursor.getColumnIndex("withSystem")) == 1;
        int priority = cursor.getInt(cursor.getColumnIndex("priority"));
        int tries = cursor.getInt(cursor.getColumnIndex("tries"));
        int code = cursor.getInt(cursor.getColumnIndex("code"));
        String extras = cursor.getString(cursor.getColumnIndex("extras"));
        TaskInfo taskInfo = new TaskInfo();
        taskInfo.setTid(tid);
        taskInfo.setSaveas(saveas);
        taskInfo.setUrl(url);
        taskInfo.setData(data);
        taskInfo.setTitle(title);
        taskInfo.setDescription(description);
        taskInfo.setAction(action);
        taskInfo.setMode(mode);
        taskInfo.setMimeType(mimeType);
        taskInfo.setProgress(JsonUtil.convertJsonToProgress(progress));
        taskInfo.setReason(reason);
        taskInfo.setCtime(ctime);
        taskInfo.setMtime(mtime);
        taskInfo.setDownloadId(downloadId);
        taskInfo.setFaults(faults);
        taskInfo.setToken(token1);
        taskInfo.setTaskStates(JsonUtil.convertJsonToTaskState(taskStates));
        taskInfo.setVersion(version);
        taskInfo.setFiles(JsonUtil.jsonToFileSpecList(files));
        taskInfo.setForms(JsonUtil.jsonToFormList(forms));
        taskInfo.setGauge(gauge);
        taskInfo.setRetry(retry);
        taskInfo.setTries(tries);
        taskInfo.setCode(code);
        taskInfo.setWithSystem(withSystem);
        taskInfo.setPriority(priority);
        taskInfo.setExtras(extras);
        return taskInfo;
    }

    /**
     * query task by filter
     *
     * @param context context
     * @param filter filter
     * @return List<Long>
     */
    @SuppressLint("Range")
    public static List<Long> queryByFilter(Context context, Filter filter) {
        Log.i(TAG, "queryByFilter");
        initDb(context);
        List<String> argsList = new ArrayList<>();
        StringBuffer sbSelection = new StringBuffer();
        if (filter != null) {
            if (filter.getBefore() > 0) {
                argsList.add(String.valueOf(filter.getBefore()));
                if (!TextUtils.isEmpty(sbSelection.toString())) {
                    sbSelection.append("and ");
                }
                sbSelection.append("ctime<? ");
            }
            if (filter.getAfter() > 0) {
                argsList.add(String.valueOf(filter.getAfter()));
                if (!TextUtils.isEmpty(sbSelection.toString())) {
                    sbSelection.append("and ");
                }
                sbSelection.append("ctime>? ");
            }
            if (filter.getAction() == Action.DOWNLOAD || filter.getAction() == Action.UPLOAD) {
                argsList.add(String.valueOf(filter.getAction()));
                if (!TextUtils.isEmpty(sbSelection.toString())) {
                    sbSelection.append("and ");
                }
                sbSelection.append("action1=? ");
            }
            if (filter.getMode() == Mode.BACKGROUND || filter.getMode() == Mode.FOREGROUND) {
                argsList.add(String.valueOf(filter.getMode()));
                if (!TextUtils.isEmpty(sbSelection.toString())) {
                    sbSelection.append("and ");
                }
                sbSelection.append("mode=? ");
            }
        }
        String selection = sbSelection.toString();
        Log.i(TAG, "queryByFilter,selection:" + selection);
        String[] selectionArgs = argsList.toArray(new String[0]);

        List<Long> taskIds = new ArrayList<>();
        Cursor cursor = null;
        try {
            cursor = mSQLiteDatabase.query("Task", null, selection, selectionArgs, null, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                do {
                    long tid = cursor.getLong(cursor.getColumnIndex("tid"));
                    Log.i(TAG, "queryByFilter,tid:" + tid);
                    String progressJson = cursor.getString(cursor.getColumnIndex("progress"));
                    Progress progress = JsonUtil.convertJsonToProgress(progressJson);
                    if (progress != null && filter != null) {
                        if (filter.getState() != State.INITIALIZED &&
                                filter.getState() != State.WAITING &&
                                filter.getState() != State.RUNNING &&
                                filter.getState() != State.RETRYING &&
                                filter.getState() != State.PAUSED &&
                                filter.getState() != State.STOPPED &&
                                filter.getState() != State.COMPLETED &&
                                filter.getState() != State.FAILED &&
                                filter.getState() != State.REMOVED &&
                                filter.getState() != State.DEFAULT
                        ) {
                            taskIds.add(tid);
                        } else if (progress.getState() == filter.getState()) {
                            taskIds.add(tid);
                        }
                    } else {
                        taskIds.add(tid);
                    }
                } while (cursor.moveToNext());
            }
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "queryByFilter: error", e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return taskIds;
    }

    /**
     * Update task info
     *
     * @param context context
     * @param taskInfo task info
     * @param isSaveToken is save token
     */
    public static void update(Context context, TaskInfo taskInfo, boolean isSaveToken) {
        Log.i(TAG, "update: " + JsonUtil.convertTaskInfoToJson(taskInfo));
        initDb(context);
        ContentValues values = new ContentValues();
        values.put("saveas", taskInfo.getSaveas());
        values.put("url", taskInfo.getUrl());
        values.put("data", taskInfo.getData());
        values.put("title", taskInfo.getTitle());
        values.put("description", taskInfo.getDescription());
        values.put("action1", taskInfo.getAction());
        values.put("mode", taskInfo.getMode());
        values.put("mimeType", taskInfo.getMimeType());
        values.put("progress", JsonUtil.convertProgressToJson(taskInfo.getProgress()));
        values.put("ctime", taskInfo.getCtime());
        values.put("mtime", taskInfo.getMtime());
        values.put("faults", taskInfo.getFaults());
        values.put("reason", taskInfo.getReason());
        values.put("downloadId", taskInfo.getDownloadId());
        if (isSaveToken) {
            values.put("token", taskInfo.getToken());
        }
        values.put("taskStates", JsonUtil.convertTaskStateToJson(taskInfo.getTaskStates()));
        values.put("version", taskInfo.getVersion());
        values.put("files", JsonUtil.convertFileSpecToJson(taskInfo.getFiles()));
        values.put("forms", JsonUtil.formsListToJson(taskInfo.getForms()));
        values.put("gauge", taskInfo.isGauge() ? 1 : 0);
        values.put("retry", taskInfo.isRetry() ? 1 : 0);
        values.put("tries", taskInfo.getTries());
        values.put("code", taskInfo.getCode());
        values.put("withSystem", taskInfo.isWithSystem() ? 1 : 0);
        try {
            String whereClause = "tid= ?";
            String[] whereArgs = {String.valueOf(taskInfo.getTid())};
            mSQLiteDatabase.update("Task", values, whereClause, whereArgs);
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "insert: error", e);
        }
    }

    /**
     * delete task by taskId
     *
     * @param context context
     * @param taskId  taskId
     */
    public static void delete(Context context, long taskId) {
        Log.i(TAG, "delete: " + taskId);
        initDb(context);
        String[] whereArgs = {String.valueOf(taskId)};
        try {
            mSQLiteDatabase.delete("Task", "tid=?", whereArgs);
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "delete error ", e);
        }
    }

    private static void initDb(Context context) {
        if (mDatabaseHelper == null) {
            mDatabaseHelper = new DatabaseHelper(context);
        }
        if (mSQLiteDatabase == null) {
            mSQLiteDatabase = mDatabaseHelper.getWritableDatabase();
        }
    }
}
