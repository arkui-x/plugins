/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
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

package ohos.ace.plugin.clipboard;

/**
 * DataRecordDTO
 *
 * @since 1
 */
public class DataRecordDTO {
    private String mimeType = null;
    private String htmlText = null;
    private String jsonWant = null;
    private String plainText = null;
    private String uri = null;

    /**
     * DataRecordDTO
     */
    public DataRecordDTO() {
    }

    /**
     * setMimeType
     *
     * @param mimeType mimeType
     */
    public void setMimeType(String mimeType) {
        this.mimeType = mimeType;
    }

    /**
     * setHtmlText
     *
     * @param htmlText htmlText
     */
    public void setHtmlText(String htmlText) {
        this.htmlText = htmlText;
    }

    /**
     * setJsonWant
     *
     * @param jsonWant jsonWant
     */
    public void setJsonWant(String jsonWant) {
        this.jsonWant = jsonWant;
    }

    /**
     * setPlainText
     *
     * @param plainText plainText
     */
    public void setPlainText(String plainText) {
        this.plainText = plainText;
    }

    /**
     * setUri
     *
     * @param uri uri
     */
    public void setUri(String uri) {
        this.uri = uri;
    }

    /**
     * getMimeType
     *
     * @return mimeType
     */
    public String getMimeType() {
        return this.mimeType;
    }

    /**
     * getHtmlText
     *
     * @return htmlText
     */
    public String getHtmlText() {
        return this.htmlText;
    }

    /**
     * getJsonWant
     *
     * @return jsonWant
     */
    public String getJsonWant() {
        return this.jsonWant;
    }

    /**
     * getPlainText
     *
     * @return plainText
     */
    public String getPlainText() {
        return this.plainText;
    }

    /**
     * getUri
     *
     * @return uri
     */
    public String getUri() {
        return this.uri;
    }
}