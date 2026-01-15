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

import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

/**
 * PatternDetection
 *
 * @since 1
 */
public class PatternDetection {
    /**
     * patterns
     */
    public static final Map<Integer, Pattern> patterns = new HashMap<>();

    /**
     * URL
     */
    public static final int URL = 0b0001;

    /**
     * NUMBER
     */
    public static final int NUMBER = 0b0010;

    /**
     * EMAIL_ADDRESS
     */
    public static final int EMAIL_ADDRESS = 0b0100;

    static {
        patterns.put(URL,
                Pattern.compile("[a-zA-Z0-9+.-]+://[-a-zA-Z0-9+&@#/%?=~_|!:,.;]*[-a-zA-Z0-9+&@#/%=~_]"));

        patterns.put(NUMBER,
                Pattern.compile("[-+]?[0-9]*\\.?[0-9]+"));

        patterns.put(EMAIL_ADDRESS,
                Pattern.compile("(([a-zA-Z0-9_\\-\\.\\%\\+]+)@(([a-zA-Z0-9\\-]+(?:\\.[a-zA-Z0-9\\-]+)*)|" +
                    "(?:\\[([0-9]{1,3}\\.){3}[0-9]{1,3}\\]))([a-zA-Z]{1,}|[0-9]{1,3}))"));
    }

    /**
     * isMatch
     *
     * @param text text
     * @param pattern pattern
     * @return boolean
     */
    public static boolean isMatch(String text, int pattern) {
        return patterns.get(pattern).matcher(text).find();
    }
}