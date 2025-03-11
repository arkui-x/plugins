/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

package ohos.ace.plugin.utilplugin;

import android.content.Context;
import android.text.TextUtils;
import android.util.Log;

import java.io.UnsupportedEncodingException;

/**
 * Util android plugin module
 *
 * @since 2023-05
 */
public class UtilPlugin {
    private static final String TAG = "UtilPlugin";

    /**
     * UtilPlugin
     *
     * @param context context of the application
     */
    public UtilPlugin(Context context) {
        nativeInit();
    }

    /**
     * Encode to chinese
     *
     * @param input input string
     * @param encoding encoding string
     * @return byte array string
     */
    public String encodeIntoChinese(String input, String encoding) {
        byte[] bytes = null;
        StringBuilder hexString = new StringBuilder();
        if (TextUtils.isEmpty(input) || TextUtils.isEmpty(encoding)) {
            Log.e(TAG, "Input or encoding is empty");
            return "";
        }
        try {
            bytes = input.getBytes(encoding);
            for (int i = 0; i < bytes.length; i++) {
                String hex = Integer.toHexString(bytes[i] & 0xFF);
                if (hex.length() < 2) {
                    hexString.append(0);
                }
                hexString.append(hex);
            }
        } catch (UnsupportedEncodingException e) {
            Log.e(TAG, "Encoding to chinese failed, unsupported encode type");
            e.printStackTrace();
        }

        return hexString.toString();
    }

    /**
     * Decode with chinese encoding type
     *
     * @param input input string
     * @param encoding encoding string
     * @return utf-8 string
     */
    public String decode(String input, String encoding) {
        String result = "";
        if (TextUtils.isEmpty(input) || TextUtils.isEmpty(encoding)) {
            Log.e(TAG, "Input or encoding is empty");
            return "";
        }
        try {
            // 1.latin1 str -> latin1 bytes
            byte[] latinBytes = input.getBytes("latin1");
            // 2.latin1 bytes -> decoding type str
            String decodeStr = new String(latinBytes, encoding);
            // 3.encoding type str ->  utf8 bytes
            byte[] utf8Bytes = decodeStr.getBytes("utf-8");
            // 4. utf-8 bytes -> utf-8 str
            result = new String(utf8Bytes, "utf-8");
        } catch (UnsupportedEncodingException e) {
            Log.e(TAG, "Decode failed, unsupported encode type");
            e.printStackTrace();
        }
        return result;
    }

    /**
     * nativeInit
     */
    protected native void nativeInit();
}
