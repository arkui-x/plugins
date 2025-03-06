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
import android.content.Intent;
import android.net.Uri;
import android.provider.DocumentsContract;
import android.util.Log;

import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.util.List;
import java.util.ArrayList;


import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.util.Map;

import android.content.Context;

import java.net.URLConnection;
import java.util.Objects;

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

    private Object mProxyInstance;

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
            Log.e(LOG_TAG, Objects.requireNonNull(e.getMessage()));
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
     */
    public void select(int maxSelectNumber, String defaultFilePathUri, String[] fileSuffixFilters, int selectMode) {
        Log.i(LOG_TAG, "select enter. selectMode:" + selectMode);
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
                Log.d(LOG_TAG, "selectFile fileSuffixFilters[i]:" + fileSuffixFilters[i] + ", types:" + types);
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
     */
    public void save(String[] newFileNames, String defaultFilePath) {
        Log.i(LOG_TAG, "save enter");
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
        Log.i(LOG_TAG, "onResult enter");

        if (!checkResult(requestCode, resultCode, data)) {
            return;
        }

        List<String> rst = new ArrayList<>();

        Uri uri = data.getData();
        if (uri != null) {
            rst.add(uri.toString());
            onPickerResult(rst, RESULT_OK);
            return;
        }
        if (data.getClipData() != null) {
            int count = data.getClipData().getItemCount();
            for (int i = 0; i < count; i++) {
                rst.add(data.getClipData().getItemAt(i).getUri().toString());
            }
            onPickerResult(rst, RESULT_OK);
        }
    }

    private boolean checkResult(int requestCode, int resultCode, Intent data) {
        String errorMsg = "requestCode:" + requestCode + ".   resultCode:" + resultCode;

        if (requestCode != FILE_PICKER_CODE) {
            Log.d(LOG_TAG, errorMsg);
            return false;
        }

        if (resultCode != Activity.RESULT_OK) {
            Log.i(LOG_TAG, errorMsg);
            onPickerResult(new ArrayList<>(), RESULT_OK);
            return false;
        }

        if (data == null) {
            errorMsg += ".  data is null";
            Log.i(LOG_TAG, errorMsg);
            onPickerResult(new ArrayList<>(), RESULT_OK);
            return false;
        }
        return true;
    }

    private void startPicker(Intent intent) {
        try {
            Objects.requireNonNull(getActivity()).startActivityForResult(intent, FILE_PICKER_CODE);
        } catch (ActivityNotFoundException e) {
            Log.e(LOG_TAG, "ActivityNotFoundException, err:", e);
            onPickerResult(new ArrayList<>(), RESULT_OK);
        } catch (Exception e) {
            Log.e(LOG_TAG, "startActivityForResult unknown error, err:", e);
            onPickerResult(null, RESULT_ERROR);
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
     */
    protected native void onPickerResult(List<String> rst, int errCode);
}
