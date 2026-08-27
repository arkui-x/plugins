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

package ohos.ace.plugin.file.fs.picker.filepicker;

import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.provider.DocumentsContract;
import android.provider.OpenableColumns;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.net.URLConnection;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import ohos.ace.adapter.ALog;

/**
 * FilePicker class for file picker plugin implementation in Java.
 *
 * @since 2024-06-24
 */
public class FilePicker {
    private static final String LOG_TAG = "FilePicker";
    private static final int FILE_PICKER_CODE = 82;
    private static final int RESULT_ERROR = 13900042;
    private static final int RESULT_OK = 0;

    private static final int COPY_BUF_SIZE = 64 * 1024;
    private static final String PICKER_CACHE_DIR = "picker_cache";
    private static final long CACHE_MAX_AGE_MS = 24 * 60 * 60 * 1000L;

    private final ExecutorService copyExecutor = Executors.newSingleThreadExecutor(
        r -> {
            Thread t = new Thread(r, "FilePicker-Copy");
            t.setPriority(Thread.NORM_PRIORITY - 1);
            return t;
        });

    private Object mProxyInstance;
    private int currentRequestId = 0;
    private boolean isSaveMode = false;
    private Activity cachedActivity = null;

    public FilePicker(Context context) {
        nativeInit();
        setCallback();
    }

    private void setCallback() {
        try {
            Class<?> delegate = Class.forName("ohos.stage.ability.adapter.StageActivityDelegate");
            Class<?> callbackInterface =
                Class.forName("ohos.stage.ability.adapter.StageActivityDelegate$INTENTCALLBACK");
            Method addIntentMethod = delegate.getMethod("addIntentCallback", callbackInterface);
            if (mProxyInstance == null) {
                mProxyInstance = Proxy.newProxyInstance(
                        callbackInterface.getClassLoader(),
                        new Class<?>[]{callbackInterface},
                        (proxy, method, args) -> {
                            if (!"onResult".equals(method.getName())) {
                                return null;
                            }
                            int requestCode = (int) args[0];
                            int resultCode = (int) args[1];
                            Intent data = (Intent) args[2];
                            onResult(requestCode, resultCode, data);
                            return null;
                        });
                addIntentMethod.invoke(null, mProxyInstance);
            }
        } catch (NoSuchMethodException | ClassNotFoundException | InvocationTargetException |
                 IllegalAccessException e) {
            ALog.e(LOG_TAG, Objects.requireNonNull(e.getMessage()));
        }
    }


    enum DocumentSelectMode {
        FILE(0), FOLDER(1), MIXED(2);
        private final int code;

        DocumentSelectMode(int code) {
            this.code = code;
        }
    }

    /**
     * select file or folder
     *
     * @param maxSelectNumber max select number
     * @param defaultFilePathUri default file path uri
     * @param fileSuffixFilters file suffix filters
     * @param selectMode select mode
     * @param requestId request id
     */
    public void select(int maxSelectNumber, String defaultFilePathUri,
        String[] fileSuffixFilters, int selectMode, int requestId) {
        this.currentRequestId = requestId;
        this.isSaveMode = false;
        ALog.i(LOG_TAG, "select enter. selectMode:" + selectMode);
        if (selectMode == DocumentSelectMode.FOLDER.code) {
            this.selectFolder();
            return;
        }
        this.selectFile(maxSelectNumber, defaultFilePathUri, fileSuffixFilters);
    }

    /**
     * select file
     *
     * @param maxSelectNumber max select number
     * @param defaultFilePathUri default file path uri
     * @param fileSuffixFilters file suffix filters
     */
    public void selectFile(int maxSelectNumber, String defaultFilePathUri, String[] fileSuffixFilters) {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);

        int SINGLE_CHOICE_MODE = 1;
        intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, maxSelectNumber != SINGLE_CHOICE_MODE);
        intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, Uri.parse(defaultFilePathUri));
        intent.setType("*/*");

        if (fileSuffixFilters.length > 0) {
            String[] fileSuffixTypes = new String[fileSuffixFilters.length];
            for (int i = 0; i < fileSuffixFilters.length; i++) {
                String types = URLConnection.guessContentTypeFromName(fileSuffixFilters[i]);
                ALog.d(LOG_TAG, "selectFile fileSuffixFilters[i]:" + fileSuffixFilters[i] + ", types:" + types);
                fileSuffixTypes[i] = types;
            }
            intent.putExtra(Intent.EXTRA_MIME_TYPES, fileSuffixTypes);
        }

        startPicker(intent);
    }

    /**
     * select folder
     */
    public void selectFolder() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
        startPicker(intent);
    }

    /**
     * save file
     *
     * @param newFileNames new file names
     * @param defaultFilePath default file path
     * @param requestId request id
     */
    public void save(String[] newFileNames, String defaultFilePath, int requestId) {
        ALog.i(LOG_TAG, "save enter");
        this.currentRequestId = requestId;
        this.isSaveMode = true;
        Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*");

        if (newFileNames.length > 0) {
            intent.putExtra(Intent.EXTRA_TITLE, newFileNames[0]);
        }

        if (!Objects.equals(defaultFilePath, "")) {
            intent.putExtra(DocumentsContract.EXTRA_INITIAL_URI, Uri.parse(defaultFilePath));
        }

        startPicker(intent);
    }

    /**
     * start picker activity and wait for result
     *
     * @param requestCode requestCode for picker activity
     * @param resultCode resultCode for picker activity
     * @param data data for picker activity
     */
    public void onResult(int requestCode, int resultCode, Intent data) {
        ALog.i(LOG_TAG, "onResult enter");

        if (!checkResult(requestCode, resultCode, data)) {
            return;
        }

        List<Uri> uris = new ArrayList<>();
        Uri uri = data.getData();
        if (uri != null) {
            uris.add(uri);
        } else if (data.getClipData() != null) {
            int count = data.getClipData().getItemCount();
            for (int i = 0; i < count; i++) {
                Uri clipUri = data.getClipData().getItemAt(i).getUri();
                if (clipUri != null) {
                    uris.add(clipUri);
                }
            }
        }

        if (!isSaveMode) {
            copyUrisToSandboxAsync(uris);
        } else {
            List<String> rst = new ArrayList<>();
            for (Uri resultUri : uris) {
                rst.add(resultUri.toString());
            }
            onPickerResult(rst, RESULT_OK, currentRequestId);
        }
    }

    private void copyUrisToSandboxAsync(List<Uri> uris) {
        int savedRequestId = currentRequestId;
        copyExecutor.execute(() -> {
            cleanExpiredCache();
            List<String> rst = new ArrayList<>();
            for (Uri resultUri : uris) {
                if (!"content".equals(resultUri.getScheme())) {
                    rst.add(resultUri.toString());
                    continue;
                }
                if (isTreeUri(resultUri)) {
                    rst.add(resultUri.toString());
                    continue;
                }
                String sandboxPath = copyToSandbox(resultUri);
                if (sandboxPath != null) {
                    rst.add("file://" + sandboxPath);
                } else {
                    rst.add(resultUri.toString());
                }
            }
            onPickerResult(rst, RESULT_OK, savedRequestId);
        });
    }

    private boolean isTreeUri(Uri uri) {
        String path = uri.getPath();
        return path != null && path.contains("/tree/");
    }

    private String copyToSandbox(Uri contentUri) {
        Activity activity = cachedActivity;
        if (activity == null) {
            activity = getActivity();
        }
        if (activity == null) {
            ALog.e(LOG_TAG, "copyToSandbox: activity is null");
            return null;
        }
        File sandboxDir = new File(activity.getFilesDir(), PICKER_CACHE_DIR);
        if (!sandboxDir.exists() && !sandboxDir.mkdirs()) {
            ALog.e(LOG_TAG, "copyToSandbox: mkdirs failed");
            return null;
        }

        String fileName = queryFileName(activity, contentUri);
        if (fileName == null || fileName.isEmpty()) {
            fileName = "picked_" + System.currentTimeMillis();
        }
        fileName = fileName.replaceAll("[:/\\\\|?*<>\"]", "_");

        File destFile = resolveUniqueFile(sandboxDir, fileName);
        try (InputStream is = activity.getContentResolver().openInputStream(contentUri);
             FileOutputStream fos = new FileOutputStream(destFile)) {
            byte[] buf = new byte[COPY_BUF_SIZE];
            int len;
            while ((len = is.read(buf)) > 0) {
                fos.write(buf, 0, len);
            }
            ALog.i(LOG_TAG, "copyToSandbox: copied to " + destFile.getAbsolutePath());
            return destFile.getAbsolutePath();
        } catch (SecurityException e) {
            ALog.e(LOG_TAG, "copyToSandbox security: " + e.getMessage());
            return null;
        } catch (Exception e) {
            ALog.e(LOG_TAG, "copyToSandbox failed: " + e.getMessage());
            return null;
        }
    }

    private File resolveUniqueFile(File dir, String fileName) {
        File candidate = new File(dir, fileName);
        if (!candidate.exists()) {
            return candidate;
        }
        String baseName;
        String ext;
        int dotIndex = fileName.lastIndexOf('.');
        if (dotIndex > 0) {
            baseName = fileName.substring(0, dotIndex);
            ext = fileName.substring(dotIndex);
        } else {
            baseName = fileName;
            ext = "";
        }
        int seq = 1;
        while (candidate.exists()) {
            candidate = new File(dir, baseName + "_" + seq + ext);
            seq++;
        }
        return candidate;
    }

    private void cleanExpiredCache() {
        Activity activity = cachedActivity;
        if (activity == null) {
            activity = getActivity();
        }
        if (activity == null) {
            return;
        }
        File sandboxDir = new File(activity.getFilesDir(), PICKER_CACHE_DIR);
        if (!sandboxDir.exists()) {
            return;
        }
        long now = System.currentTimeMillis();
        File[] files = sandboxDir.listFiles();
        if (files == null) {
            return;
        }
        for (File f : files) {
            if (f.isFile() && (now - f.lastModified()) > CACHE_MAX_AGE_MS) {
                if (f.delete()) {
                    ALog.i(LOG_TAG, "cleanExpiredCache: deleted " + f.getName());
                }
            }
        }
    }

    private String queryFileName(Context context, Uri uri) {
        String name = null;
        Cursor cursor = null;
        try {
            cursor = context.getContentResolver().query(uri, null, null, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                int nameIndex = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                if (nameIndex >= 0) {
                    name = cursor.getString(nameIndex);
                }
            }
        } catch (Exception e) {
            ALog.w(LOG_TAG, "queryFileName failed: " + e.getMessage());
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        if (name == null) {
            String lastSegment = uri.getLastPathSegment();
            if (lastSegment != null) {
                name = lastSegment.replaceAll("[:/]", "_");
            }
        }
        return name;
    }

    private boolean checkResult(int requestCode, int resultCode, Intent data) {
        String errorMsg = "requestCode:" + requestCode + ".   resultCode:" + resultCode;

        if (requestCode != FILE_PICKER_CODE) {
            ALog.d(LOG_TAG, errorMsg);
            return false;
        }

        if (resultCode != Activity.RESULT_OK) {
            ALog.i(LOG_TAG, errorMsg);
            onPickerResult(new ArrayList<>(), RESULT_OK, currentRequestId);
            return false;
        }

        if (data == null) {
            errorMsg += ".  data is null";
            ALog.i(LOG_TAG, errorMsg);
            onPickerResult(new ArrayList<>(), RESULT_ERROR, currentRequestId);
            return false;
        }
        return true;
    }

    private void startPicker(Intent intent) {
        try {
            Activity activity = Objects.requireNonNull(getActivity());
            this.cachedActivity = activity;
            activity.startActivityForResult(intent, FILE_PICKER_CODE);
        } catch (ActivityNotFoundException e) {
            ALog.e(LOG_TAG, "ActivityNotFoundException, err:" + e);
            onPickerResult(new ArrayList<>(), RESULT_ERROR, currentRequestId);
        } catch (Exception e) {
            ALog.e(LOG_TAG, "startActivityForResult unknown error, err:" + e);
            onPickerResult(new ArrayList<>(), RESULT_ERROR, currentRequestId);
        }
    }

    private Activity getActivity() {
        try {
            Class activityThreadClass = Class.forName("android.app.ActivityThread");
            Object activityThread = activityThreadClass.getMethod("currentActivityThread").invoke(null);
            Field mActivities = activityThreadClass.getDeclaredField("mActivities");
            mActivities.setAccessible(true);
            Map activitiesMap = (Map) mActivities.get(activityThread);
            for (Object activityClientRecord : activitiesMap.values()) {
                Class activityClientRecordClass = activityClientRecord.getClass();
                Field paused = activityClientRecordClass.getDeclaredField("paused");
                paused.setAccessible(true);
                if (!paused.getBoolean(activityClientRecord)) {
                    Field activityField = activityClientRecordClass.getDeclaredField("activity");
                    activityField.setAccessible(true);
                    Activity activity = (Activity) activityField.get(activityClientRecord);
                    return activity;
                }
            }
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        } catch (NoSuchFieldException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     * start file picker
     */
    protected native void nativeInit();

    /**
     * on picker result
     *
     * @param rst result
     * @param errCode error code
     * @param requestId request id
     */
    protected native void onPickerResult(List<String> rst, int errCode, int requestId);
}
