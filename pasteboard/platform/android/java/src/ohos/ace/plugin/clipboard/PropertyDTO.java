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
 * PropertyDTO
 *
 * @since 1
 */
public class PropertyDTO {
    private String[] mimeTypes = null;
    private String tag = null;
    private long timeStamp = 0L;
    private String jsonAdditions = null;

    /**
     * Constructor
     */
    public PropertyDTO() {
    }

    /**
     * setMimeTypes
     *
     * @param mimeTypes mimeTypes
     */
    public void setMimeTypes(String[] mimeTypes) {
        this.mimeTypes = mimeTypes;
    }

    /**
     * setTag
     *
     * @param tag tag
     */
    public void setTag(String tag) {
        this.tag = tag;
    }

    /**
     * setTimestamp
     *
     * @param timeStamp timeStamp
     */
    public void setTimestamp(long timeStamp) {
        this.timeStamp = timeStamp;
    }

    /**
     * setJsonAddtions
     *
     * @param jsonAdditions jsonAdditions
     */
    public void setJsonAdditions(String jsonAdditions) {
        this.jsonAdditions = jsonAdditions;
    }

    /**
     * getMimeTypes
     *
     * @return mimeTypes
     */
    public String[] getMimeTypes() {
        return this.mimeTypes;
    }

    /**
     * getTag
     *
     * @return tag
     */
    public String getTag() {
        return this.tag;
    }

    /**
     * getTimestamp
     *
     * @return timeStamp
     */
    public long getTimestamp() {
        return this.timeStamp;
    }

    /**
     * getJsonAdditions
     *
     * @return jsonAdditions
     */
    public String getJsonAdditions() {
        return this.jsonAdditions;
    }
}