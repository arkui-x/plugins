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

import java.util.HashMap;

/**
 * Config class for task manager plugin.
 *
 * @since 2024-05-31
 */
public class Config {
    private long taskId;

    /**
     * The task action, upload or download.
     */
    private int action;
    private String url;
    private String title;
    private String description;

    /**
     * The default is background.
     */
    private int mode;
    private boolean overwrite;
    private String method;
    private HashMap<String, String> headers;
    private String data;
    private String saveas;
    private String proxy;
    private int network;
    private boolean metered;
    private boolean roaming;
    private boolean redirect;
    private int index;
    private int begins;
    private int ends;
    private boolean gauge;
    private boolean precise;
    private String token;
    private String extras;

    private long cTime;
    private String mimeType;
    private int priority;
    private boolean retry;
    private boolean background;
    private String forms;
    private String files;
    private String bodyFds;
    private String bodyFileNames;
    private int version;

    public long getTaskId() {
        return taskId;
    }

    public void setTaskId(long taskId) {
        this.taskId = taskId;
    }

    public int getAction() {
        return action;
    }

    public void setAction(int action) {
        this.action = action;
    }

    public String getUrl() {
        return url;
    }

    public void setUrl(String url) {
        this.url = url;
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

    public int getMode() {
        return mode;
    }

    public void setMode(int mode) {
        this.mode = mode;
    }

    public boolean isOverwrite() {
        return overwrite;
    }

    public void setOverwrite(boolean overwrite) {
        this.overwrite = overwrite;
    }

    public String getMethod() {
        return method;
    }

    public void setMethod(String method) {
        this.method = method;
    }

    public HashMap<String, String> getHeaders() {
        return headers;
    }

    public void setHeaders(HashMap<String, String> headers) {
        this.headers = headers;
    }

    public String getData() {
        return data;
    }

    public void setData(String data) {
        this.data = data;
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


    public int getNetwork() {
        return network;
    }

    public void setNetwork(int network) {
        this.network = network;
    }

    public boolean isMetered() {
        return metered;
    }

    public void setMetered(boolean metered) {
        this.metered = metered;
    }

    public boolean isRoaming() {
        return roaming;
    }

    public void setRoaming(boolean roaming) {
        this.roaming = roaming;
    }

    public boolean isRedirect() {
        return redirect;
    }

    public void setRedirect(boolean redirect) {
        this.redirect = redirect;
    }

    public int getIndex() {
        return index;
    }

    public void setIndex(int index) {
        this.index = index;
    }

    public int getBegins() {
        return begins;
    }

    public void setBegins(int begins) {
        this.begins = begins;
    }

    public int getEnds() {
        return ends;
    }

    public void setEnds(int ends) {
        this.ends = ends;
    }

    public boolean isGauge() {
        return gauge;
    }

    public void setGauge(boolean gauge) {
        this.gauge = gauge;
    }

    public boolean isPrecise() {
        return precise;
    }

    public void setPrecise(boolean precise) {
        this.precise = precise;
    }

    public String getToken() {
        return token;
    }

    public void setToken(String token) {
        this.token = token;
    }

    public String getExtras() {
        return extras;
    }

    public void setExtras(String extras) {
        this.extras = extras;
    }

    public long getcTime() {
        return cTime;
    }

    public void setcTime(long cTime) {
        this.cTime = cTime;
    }

    public String getMimeType() {
        return mimeType;
    }

    public void setMimeType(String mimeType) {
        this.mimeType = mimeType;
    }

    public int getPriority() {
        return priority;
    }

    public void setPriority(int priority) {
        this.priority = priority;
    }

    public boolean isRetry() {
        return retry;
    }

    public void setRetry(boolean retry) {
        this.retry = retry;
    }

    public boolean isBackground() {
        return background;
    }

    public void setBackground(boolean background) {
        this.background = background;
    }

    public String getForms() {
        return forms;
    }

    public void setForms(String forms) {
        this.forms = forms;
    }

    public String getFiles() {
        return files;
    }

    public void setFiles(String files) {
        this.files = files;
    }

    public String getBodyFds() {
        return bodyFds;
    }

    public void setBodyFds(String bodyFds) {
        this.bodyFds = bodyFds;
    }

    public String getBodyFileNames() {
        return bodyFileNames;
    }

    public void setBodyFileNames(String bodyFileNames) {
        this.bodyFileNames = bodyFileNames;
    }

    public int getVersion() {
        return version;
    }

    public void setVersion(int version) {
        this.version = version;
    }
}
