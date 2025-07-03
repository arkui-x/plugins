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

import java.util.List;

/**
 * TaskInfo class
 *
 * @since 2024-05-31
 */
public class TaskInfo {
    private long tid;
    private String saveas;
    private String proxy;
    private String url;
    private String data;
    private String title;
    private String description;
    private int action;
    private int mode;
    private String mimeType;
    private Progress progress;
    private long ctime;
    private long mtime;
    private int faults;
    private String reason;
    private long downloadId;
    private String token;
    private TaskState taskStates;
    private int version;
    private List<FileSpec> files;
    private List<FormItem> forms;
    private boolean gauge;
    private boolean retry;
    private int tries;
    private int code;
    private boolean withSystem;
    private int priority;
    private String extras;
    private Response response;

    public Response getResponse() {
        return response;
    }

    public void setResponse(Response response) {
        this.response = response;
    }

    public String getSaveas() {
        return saveas;
    }

    public void setSaveas(String saveas) {
        this.saveas = saveas;
    }

    public String getProxy() {
        return proxy;
    }

    public void setProxy(String proxy) {
        this.proxy = proxy;
    }

    public String getUrl() {
        return url;
    }

    public void setUrl(String url) {
        this.url = url;
    }

    public String getData() {
        return data;
    }

    public void setData(String data) {
        this.data = data;
    }

    public long getTid() {
        return tid;
    }

    public void setTid(long tid) {
        this.tid = tid;
    }

    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public int getAction() {
        return action;
    }

    public void setAction(int action) {
        this.action = action;
    }

    public int getMode() {
        return mode;
    }

    public void setMode(int mode) {
        this.mode = mode;
    }

    public String getMimeType() {
        return mimeType;
    }

    public void setMimeType(String mimeType) {
        this.mimeType = mimeType;
    }

    public Progress getProgress() {
        return progress;
    }

    public void setProgress(Progress progress) {
        this.progress = progress;
    }

    public long getCtime() {
        return ctime;
    }

    public void setCtime(long ctime) {
        this.ctime = ctime;
    }

    public long getMtime() {
        return mtime;
    }

    public void setMtime(long mtime) {
        this.mtime = mtime;
    }

    public int getFaults() {
        return faults;
    }

    public void setFaults(int faults) {
        this.faults = faults;
    }

    public String getReason() {
        return reason;
    }

    public void setReason(String reason) {
        this.reason = reason;
    }

    public long getDownloadId() {
        return downloadId;
    }

    public void setDownloadId(long downloadId) {
        this.downloadId = downloadId;
    }

    public String getToken() {
        return token;
    }

    public void setToken(String token) {
        this.token = token;
    }

    public TaskState getTaskStates() {
        return taskStates;
    }

    public void setTaskStates(TaskState taskStates) {
        this.taskStates = taskStates;
    }

    public int getVersion() {
        return version;
    }

    public void setVersion(int version) {
        this.version = version;
    }

    public List<FileSpec> getFiles() {
        return files;
    }

    public void setFiles(List<FileSpec> files) {
        this.files = files;
    }

    public List<FormItem> getForms() {
        return forms;
    }

    public void setForms(List<FormItem> forms) {
        this.forms = forms;
    }

    public boolean isGauge() {
        return gauge;
    }

    public void setGauge(boolean gauge) {
        this.gauge = gauge;
    }

    public boolean isRetry() {
        return retry;
    }

    public void setRetry(boolean retry) {
        this.retry = retry;
    }

    public int getTries() {
        return tries;
    }

    public void setTries(int tries) {
        this.tries = tries;
    }

    public int getCode() {
        return code;
    }

    public void setCode(int code) {
        this.code = code;
    }

    public boolean isWithSystem() {
        return withSystem;
    }

    public void setWithSystem(boolean withSystem) {
        this.withSystem = withSystem;
    }

    public int getPriority() {
        return priority;
    }

    public void setPriority(int priority) {
        this.priority = priority;
    }

    public String getExtras() {
        return extras;
    }

    public void setExtras(String extras) {
        this.extras = extras;
    }
}
