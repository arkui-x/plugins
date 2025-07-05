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

import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

/**
 * JsonUtil class for json to config and config to json convert.
 *
 * @since 2024-05-31
 */
public class JsonUtil {
    /**
     * Json转Config
     *
     * @param configJson json
     * @return Config
     */
    public static Config jsonToConfig(String configJson) {
        if (configJson == null || configJson.isEmpty()) {
            return null;
        }
        Config config = null;
        try {
            JSONObject jsonObject = new JSONObject(configJson);
            config = new Config();
            config.setTaskId(jsonObject.optLong("taskId"));
            config.setAction(jsonObject.optInt("action"));
            config.setUrl(jsonObject.optString("url"));
            config.setTitle(jsonObject.optString("title"));
            config.setDescription(jsonObject.optString("description"));
            config.setMode(jsonObject.optInt("mode"));
            config.setOverwrite(jsonObject.optBoolean("overwrite"));
            config.setMethod(jsonObject.optString("method"));
            JSONObject headersObject = jsonObject.optJSONObject("headers");
            if (headersObject != null) {
                HashMap<String, String> headers = new HashMap<>();
                Iterator<String> iterator = headersObject.keys();
                while (iterator.hasNext()) {
                    String key = iterator.next();
                    headers.put(key, headersObject.optString(key));
                }
                config.setHeaders(headers);
            }
            config.setData(jsonObject.optString("data"));
            config.setSaveas(jsonObject.optString("saveas"));
            config.setNetwork(jsonObject.optInt("network"));
            config.setMetered(jsonObject.optBoolean("metered"));
            config.setRoaming(jsonObject.optBoolean("roaming"));
            config.setRedirect(jsonObject.optBoolean("redirect"));
            config.setIndex(jsonObject.optInt("index"));
            config.setBegins(jsonObject.optInt("begins"));
            config.setEnds(jsonObject.optInt("ends"));
            config.setGauge(jsonObject.optBoolean("gauge"));
            config.setPrecise(jsonObject.optBoolean("precise"));
            config.setToken(jsonObject.optString("token"));
            config.setExtras(jsonObject.optString("extras"));
            config.setcTime(jsonObject.optLong("cTime"));

            config.setPriority(jsonObject.optInt("priority"));
            config.setRetry(jsonObject.optBoolean("retry"));
            config.setBackground(jsonObject.optBoolean("background"));
            config.setForms(jsonObject.optString("forms"));
            config.setFiles(jsonObject.optString("files"));
            config.setBodyFds(jsonObject.optString("bodyFds"));
            config.setBodyFileNames(jsonObject.optString("bodyFileNames"));
            config.setVersion(jsonObject.optInt("version"));
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return config;
    }

    /**
     * config to json
     *
     * @param config config
     * @return json
     */
    public static String configToJson(Config config) {
        if (config == null) {
            return "";
        }
        try {
            JSONObject jsonConfig = new JSONObject();
            jsonConfig.put("taskId", config.getTaskId());
            jsonConfig.put("action", config.getAction());
            jsonConfig.put("url", config.getUrl());
            jsonConfig.put("title", config.getTitle());
            jsonConfig.put("description", config.getDescription());
            jsonConfig.put("mode", config.getMode());
            jsonConfig.put("overwrite", config.isOverwrite());
            jsonConfig.put("method", config.getMethod());
            if (config.getHeaders() != null) {
                jsonConfig.put("headers", new JSONObject(config.getHeaders()));
            }
            jsonConfig.put("data", config.getData());
            jsonConfig.put("saveas", config.getSaveas());
            jsonConfig.put("network", config.getNetwork());
            jsonConfig.put("metered", config.isMetered());
            jsonConfig.put("roaming", config.isRoaming());
            jsonConfig.put("redirect", config.isRedirect());
            jsonConfig.put("index", config.getIndex());
            jsonConfig.put("begins", config.getBegins());
            jsonConfig.put("ends", config.getEnds());
            jsonConfig.put("gauge", config.isGauge());
            jsonConfig.put("precise", config.isPrecise());
            jsonConfig.put("token", config.getToken());
            jsonConfig.put("cTime", config.getcTime());
            jsonConfig.put("mimeType", config.getMimeType());
            jsonConfig.put("version", config.getVersion());
            jsonConfig.put("priority", config.getPriority());
            jsonConfig.put("retry", config.isRetry());
            jsonConfig.put("background", config.isBackground());
            jsonConfig.put("forms", config.getForms());
            jsonConfig.put("files", config.getFiles());
            jsonConfig.put("bodyFds", config.getBodyFds());
            jsonConfig.put("bodyFileNames", config.getBodyFileNames());
            jsonConfig.put("extras", config.getExtras());
            return jsonConfig.toString();
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return "";
    }

    /**
     * Progress转成Json
     *
     * @param progress progress
     * @return Json
     */
    public static String convertProgressToJson(Progress progress) {
        if (progress == null) {
            return "";
        }
        try {
            JSONObject json = new JSONObject();
            json.put("state", progress.getState());
            json.put("index", progress.getIndex());
            json.put("processed", progress.getProcessed());
            json.put("lastProcessed", progress.getLastProcessed());
            if (progress.getSizes() != null) {
                JSONArray sizeArray = new JSONArray();
                for (long size : progress.getSizes()) {
                    sizeArray.put(size);
                }
                json.put("sizes", sizeArray);
            }
            if (progress.getExtras() != null) {
                JSONObject extrasJSON = new JSONObject(progress.getExtras());
                json.put("extras", extrasJSON);
            }
            return json.toString();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return "";
    }

    /**
     * json to Config
     *
     * @param progressJson progressJson
     * @return Config
     */
    public static Progress convertJsonToProgress(String progressJson) {
        if (progressJson == null) {
            return null;
        }
        try {
            JSONObject json = new JSONObject(progressJson);
            // progress
            Progress progress = new Progress();
            progress.setState(json.optInt("state"));
            progress.setIndex(json.optInt("index"));
            progress.setProcessed(json.optLong("processed"));
            progress.setLastProcessed(json.optLong("lastProcessed"));
            JSONArray sizesArray = json.optJSONArray("sizes");
            if (sizesArray != null) {
                List<Long> sizes = new ArrayList<>();
                for (int i = 0; i < sizesArray.length(); i++) {
                    sizes.add(sizesArray.optLong(i));
                }
                progress.setSizes(sizes);
            }
            // extras
            JSONObject extrasJSON = json.optJSONObject("extras");
            if (extrasJSON != null) {
                Iterator<String> keys = extrasJSON.keys();
                Map<String, String> extras = new HashMap<>();
                while (keys.hasNext()) {
                    String key = keys.next();
                    String value = extrasJSON.optString(key);
                    extras.put(key, value);
                }
                progress.setExtras(extras);
            }
            return progress;
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     * json to TaskState
     *
     * @param jsonTaskState jsonTaskState
     * @return TaskState
     */
    public static TaskState convertJsonToTaskState(String jsonTaskState) {
        if (jsonTaskState == null || jsonTaskState.isEmpty()) {
            return null;
        }
        try {
            JSONObject json = new JSONObject(jsonTaskState);
            TaskState taskState = new TaskState();
            taskState.setPath(json.optString("path"));
            taskState.setResponseCode(json.optInt("responseCode"));
            taskState.setMessage(json.optString("message"));
            return taskState;
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     * TaskState to Json
     *
     * @param taskState TaskState object
     * @return Json string
     */
    public static String convertTaskStateToJson(TaskState taskState) {
        if (taskState == null) {
            return "";
        }
        try {
            JSONObject json = new JSONObject();
            json.put("path", taskState.getPath());
            json.put("responseCode", taskState.getResponseCode());
            json.put("message", taskState.getMessage());
            return json.toString();
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return "";
    }

    /**
     * Convert TaskInfo to Json
     *
     * @param taskInfo TaskInfo object
     * @return Json string
     */
    public static String convertTaskInfoToJson(TaskInfo taskInfo) {
        if (taskInfo == null) {
            return "";
        }
        try {
            JSONObject jsonObj = new JSONObject();
            jsonObj.put("tid", String.valueOf(taskInfo.getTid()));
            jsonObj.put("saveas", taskInfo.getSaveas());
            jsonObj.put("url", taskInfo.getUrl());
            jsonObj.put("data", taskInfo.getData());
            jsonObj.put("title", taskInfo.getTitle());
            jsonObj.put("description", taskInfo.getDescription());
            jsonObj.put("action", taskInfo.getAction());
            jsonObj.put("mode", taskInfo.getMode());
            jsonObj.put("mimeType", taskInfo.getMimeType());
            jsonObj.put("ctime", taskInfo.getCtime());
            jsonObj.put("mtime", taskInfo.getMtime());
            jsonObj.put("faults", taskInfo.getFaults());
            jsonObj.put("reason", taskInfo.getReason());
            jsonObj.put("downloadId", taskInfo.getDownloadId());
            jsonObj.put("token", taskInfo.getToken());
            jsonObj.put("version", taskInfo.getVersion());
            List<FileSpec> fileSpecList = taskInfo.getFiles();
            if (fileSpecList != null) {
                JSONArray filesArray = new JSONArray();
                for (FileSpec item : fileSpecList) {
                    JSONObject jsonFileSpec = new JSONObject();
                    jsonFileSpec.put("name", item.getName());
                    jsonFileSpec.put("uri", item.getUri());
                    jsonFileSpec.put("filename", item.getFilename());
                    jsonFileSpec.put("type", item.getType());
                    jsonFileSpec.put("fd", item.getFd());
                    filesArray.put(jsonFileSpec);
                }
                jsonObj.put("files", filesArray);
            }
            List<FormItem> formItemList = taskInfo.getForms();
            if (formItemList != null) {
                JSONArray formsArray = new JSONArray();
                for (FormItem item : formItemList) {
                    JSONObject jsonFormsItem = new JSONObject();
                    jsonFormsItem.put("name", item.getName());
                    jsonFormsItem.put("value", item.getValue());
                    formsArray.put(jsonFormsItem);
                }
                jsonObj.put("forms", formsArray);
            }
            jsonObj.put("gauge", taskInfo.isGauge());
            jsonObj.put("retry", taskInfo.isRetry());
            jsonObj.put("tries", taskInfo.getTries());
            jsonObj.put("code", taskInfo.getCode());
            jsonObj.put("withSystem", taskInfo.isWithSystem());
            jsonObj.put("extras", taskInfo.getExtras());
            // Progress
            Progress progress = taskInfo.getProgress();
            if (progress != null) {
                JSONObject progressObj = new JSONObject();
                progressObj.put("state", progress.getState());
                progressObj.put("index", progress.getIndex());
                progressObj.put("processed", progress.getProcessed());
                progressObj.put("lastProcessed", progress.getLastProcessed());
                if (progress.getSizes() != null) {
                    JSONArray sizeArray = new JSONArray();
                    for (long size : progress.getSizes()) {
                        sizeArray.put(size);
                    }
                    progressObj.put("sizes", sizeArray);
                }
                progressObj.put("extras", progress.getExtras());
                jsonObj.put("progress", progressObj);
            }

            TaskState taskState = taskInfo.getTaskStates();
            if (taskState != null) {
                JSONObject jsonTaskState = new JSONObject();
                jsonTaskState.put("path", taskState.getPath());
                jsonTaskState.put("responseCode", taskState.getResponseCode());
                jsonTaskState.put("message", taskState.getMessage());
                jsonObj.put("taskStates", jsonTaskState);
            }

            Response response = taskInfo.getResponse();
            if (response != null) {
                JSONObject responseObj = new JSONObject();
                responseObj.put("version", response.getVersion());
                responseObj.put("statusCode", response.getStatusCode());
                responseObj.put("reason", response.getReason());
                if (response.getHeaders() != null) {
                    JSONObject headersObj = new JSONObject();
                    for (Map.Entry<String, List<String>> entry : response.getHeaders().entrySet()) {
                        JSONArray valuesArray = new JSONArray(entry.getValue());
                        headersObj.put(entry.getKey(), valuesArray);
                    }
                    responseObj.put("headers", headersObj);
                }
                jsonObj.put("response", responseObj);
            }
            return jsonObj.toString();
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return "";
    }

    /**
     * json to Filter
     *
     * @param filterJson filter json
     * @return Filter
     */
    public static Filter jsonToFilter(String filterJson) {
        if (filterJson == null) {
            return null;
        }
        try {
            JSONObject jsonObject = new JSONObject(filterJson);
            Filter filter = new Filter();
            filter.setBundle(jsonObject.optString("bundle"));
            filter.setBefore(jsonObject.optLong("before", -1));
            filter.setAfter(jsonObject.optLong("after", -1));
            filter.setState(jsonObject.optInt("state", -1));
            filter.setAction(jsonObject.optInt("action", -1));
            filter.setMode(jsonObject.optInt("mode", -1));
            return filter;
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     * json to TaskInfo
     *
     * @param taskInfoJson taskInfo json
     * @return TaskInfo
     */
    public static TaskInfo jsonToTaskInfo(String taskInfoJson) {
        if (taskInfoJson == null) {
            return null;
        }
        try {
            TaskInfo taskInfo = new TaskInfo();
            JSONObject json = new JSONObject(taskInfoJson);
            taskInfo.setTid(json.optLong("tid"));
            taskInfo.setSaveas(json.optString("saveas"));
            taskInfo.setUrl(json.optString("url"));
            taskInfo.setData(json.optString("data"));
            taskInfo.setTitle(json.optString("title"));
            taskInfo.setDescription(json.optString("description"));
            taskInfo.setAction(json.optInt("action"));
            taskInfo.setMode(json.optInt("mode"));
            taskInfo.setMimeType(json.optString("mimeType"));
            taskInfo.setVersion(json.optInt("version"));
            JSONArray filesArray = json.optJSONArray("files");
            if (filesArray != null) {
                List<FileSpec> fileSpecList = new ArrayList<>();
                for (int i = 0; i < filesArray.length(); i++) {
                    JSONObject jsonFileSpec = filesArray.getJSONObject(i);
                    FileSpec fileSpec = new FileSpec();
                    fileSpec.setName(jsonFileSpec.optString("name"));
                    fileSpec.setUri(jsonFileSpec.optString("uri"));
                    fileSpec.setFilename(jsonFileSpec.optString("filename"));
                    fileSpec.setType(jsonFileSpec.optString("type"));
                    fileSpec.setFd(jsonFileSpec.optInt("fd"));
                    fileSpecList.add(fileSpec);
                }
                taskInfo.setFiles(fileSpecList);
            }
            JSONArray formsArray = json.optJSONArray("forms");
            if (formsArray != null) {
                List<FormItem> formItemList = new ArrayList<>();
                for (int i = 0; i < formsArray.length(); i++) {
                    JSONObject jsonFormItem = formsArray.optJSONObject(i);
                    FormItem formItem = new FormItem();
                    formItem.setName(jsonFormItem.optString("name"));
                    formItem.setValue(jsonFormItem.optString("value"));
                    formItemList.add(formItem);
                }
                taskInfo.setForms(formItemList);
            }
            taskInfo.setGauge(json.optBoolean("gauge"));
            taskInfo.setRetry(json.optBoolean("retry"));
            taskInfo.setTries(json.optInt("tries"));
            taskInfo.setCode(json.optInt("code"));
            taskInfo.setWithSystem(json.optBoolean("withSystem"));
            taskInfo.setPriority(json.optInt("priority"));
            taskInfo.setExtras(json.optString("extras"));
            Progress progress = new Progress();
            JSONObject progressObj = json.optJSONObject("progress");
            if (progressObj != null) {
                progress.setState(progressObj.optInt("state"));
                progress.setIndex(progressObj.optInt("index"));
                progress.setProcessed(progressObj.optLong("processed"));
                progress.setLastProcessed(progressObj.optLong("lastProcessed"));
                JSONArray sizesArray = progressObj.optJSONArray("sizes");
                if (sizesArray != null) {
                    List<Long> sizes = new ArrayList<>();
                    for (int i = 0; i < sizesArray.length(); i++) {
                        sizes.add(sizesArray.optLong(i));
                    }
                    progress.setSizes(sizes);
                }
                JSONObject extrasJSON = progressObj.optJSONObject("extras");
                if (extrasJSON != null) {
                    Map<String, String> extras = new HashMap<>();
                    Iterator<String> keys = extrasJSON.keys();
                    while (keys.hasNext()) {
                        String key = keys.next();
                        String value = extrasJSON.optString(key);
                        extras.put(key, value);
                    }
                    progress.setExtras(extras);
                }
            }
            taskInfo.setProgress(progress);
            TaskState taskState = new TaskState();
            JSONObject jsonTaskState = json.optJSONObject("taskStates");
            if (jsonTaskState != null) {
                taskState.setPath(jsonTaskState.optString("path"));
                taskState.setResponseCode(jsonTaskState.optInt("responseCode"));
                taskState.setMessage(jsonTaskState.optString("message"));
                taskInfo.setTaskStates(taskState);
            }
            taskInfo.setCtime(json.optLong("ctime"));
            taskInfo.setMtime(json.optLong("mtime"));
            taskInfo.setFaults(json.optInt("faults"));
            taskInfo.setReason(json.optString("reason"));
            taskInfo.setDownloadId(json.optLong("downloadId"));
            taskInfo.setToken(json.optString("token"));
            return taskInfo;
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     * Converts a map to a JSON string.
     *
     * @param map the map to be converted
     * @return the JSON string representation of the map
     */
    public static String mapStringToJson(Map<String, String> map) {
        JSONObject jsonObject = new JSONObject(map);
        return jsonObject.toString();
    }

    /**
     * Converts a JSON string to a map of strings.
     *
     * @param json the JSON string to be converted
     * @return the map of strings representation of the JSON string
     */
    public static HashMap<String, String> jsonToMapString(String json) {
        HashMap<String, String> map = new HashMap<>();
        if (json == null) {
            return map;
        }
        try {
            JSONObject jsonObject = new JSONObject(json);
            Iterator<String> keys = jsonObject.keys();
            while (keys.hasNext()) {
                String key = keys.next();
                String value = jsonObject.optString(key);
                map.put(key, value);
            }
        } catch (JSONException e) {
            Log.e(TAG, "jsonToMapString: error", e);
        }
        return map;
    }

    /**
     * Convert FileSpec list to json string
     *
     * @param fileSpecList FileSpec list
     * @return json string
     */
    public static String convertFileSpecToJson(List<FileSpec> fileSpecList) {
        if (fileSpecList == null) {
            return "";
        }
        try {
            JSONArray jsonArray = new JSONArray();
            for (FileSpec fileSpec : fileSpecList) {
                JSONObject jsonFileSpec = new JSONObject();
                jsonFileSpec.put("name", fileSpec.getName());
                jsonFileSpec.put("uri", fileSpec.getUri());
                jsonFileSpec.put("filename", fileSpec.getFilename());
                jsonFileSpec.put("type", fileSpec.getType());
                jsonFileSpec.put("fd", fileSpec.getFd());
                jsonArray.put(jsonFileSpec);
            }
            return jsonArray.toString();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return "";
    }

    /**
     * Convert json string to FileSpec list
     *
     * @param json json string
     * @return FileSpec list
     */
    public static List<FileSpec> jsonToFileSpecList(String json) {
        List<FileSpec> fileSpecList = new ArrayList<>();
        if (json == null) {
            return fileSpecList;
        }
        try {
            JSONArray jsonArray = new JSONArray(json);
            for (int i = 0; i < jsonArray.length(); i++) {
                JSONObject jsonFileSpec = jsonArray.getJSONObject(i);
                FileSpec fileSpec = new FileSpec();
                fileSpec.setName(jsonFileSpec.optString("name"));
                fileSpec.setUri(jsonFileSpec.optString("uri"));
                fileSpec.setFilename(jsonFileSpec.optString("filename"));
                fileSpec.setType(jsonFileSpec.optString("type"));
                fileSpec.setFd(jsonFileSpec.optInt("fd"));
                fileSpecList.add(fileSpec);
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return fileSpecList;
    }

    /**
     * Convert FormItem list to json string
     *
     * @param formItems FormItem list
     * @return json string
     */
    public static String formsListToJson(List<FormItem> formItems) {
        if (formItems == null) {
            return "";
        }
        try {
            JSONArray jsonArray = new JSONArray();
            for (FormItem formItem : formItems) {
                JSONObject jsonFormItem = new JSONObject();
                jsonFormItem.put("name", formItem.getName());
                jsonFormItem.put("value", formItem.getValue());
                jsonArray.put(formItem);
            }
            return jsonArray.toString();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return "";
    }

    /**
     * Convert json string to FormItem list
     *
     * @param json json string
     * @return FormItem list
     */
    public static List<FormItem> jsonToFormList(String json) {
        List<FormItem> formItems = new ArrayList<>();
        if (json == null || json.isEmpty()) {
            return formItems;
        }
        try {
            JSONArray jsonArray = new JSONArray(json);
            for (int i = 0; i < jsonArray.length(); i++) {
                JSONObject jsonFormItem = jsonArray.getJSONObject(i);
                FormItem formItem = new FormItem();
                formItem.setName(jsonFormItem.optString("name"));
                formItem.setValue(jsonFormItem.optString("value"));
                formItems.add(formItem);
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return formItems;
    }
}
