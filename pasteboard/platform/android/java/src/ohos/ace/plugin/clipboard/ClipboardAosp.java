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

import android.content.ClipData;
import android.content.ClipData.Item;
import android.content.ClipDescription;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.FileUriExposedException;
import android.os.Handler;
import android.text.Html;
import android.text.Spanned;
import android.util.Log;

import ohos.ace.plugin.clipboard.PasteboardConstants.PasteboardError;

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * ClipboardAosp
 *
 * @since 1
 */
public class ClipboardAosp {
    private static final String LOG_TAG = "ClipboardPlugin";
    private static final long CLIPBOARD_CHANGE_DELAY_MS = 50L;
    private static final int VERSION_CODE_P = 28;
    private static final String MULTI_TYPE_DATA_MIME = "arkuix/multipletypedata";
    private static final String ARKUI_X_DATA_FLAG = "arkuix/pastedataflag";
    private static final String MIME_OH_TEXT_PLAIN = "text/plain";
    private static final String MIME_OH_TEXT_HTML = "text/html";
    private static final String MIME_OH_TEXT_URI = "text/uri";
    private static final String MIME_OH_TEXT_WANT = "text/want";
    private static final String MIME_PLATFORM_TEXT_PLAIN = "text/plain";
    private static final String MIME_PLATFORM_TEXT_HTML = "text/html";
    private static final String MIME_PLATFORM_TEXT_URI_LIST = "text/uri-list";
    private static final String MIME_PLATFORM_TEXT_INTENT = "text/intent";
    private static final Map<String, String> MIME_TYPE_MAPPING;

    static {
        MIME_TYPE_MAPPING = new HashMap<String, String>();
        MIME_TYPE_MAPPING.put(MIME_OH_TEXT_PLAIN, MIME_PLATFORM_TEXT_PLAIN);
        MIME_TYPE_MAPPING.put(MIME_OH_TEXT_HTML, MIME_PLATFORM_TEXT_HTML);
        MIME_TYPE_MAPPING.put(MIME_OH_TEXT_URI, MIME_PLATFORM_TEXT_URI_LIST);
        MIME_TYPE_MAPPING.put(MIME_OH_TEXT_WANT, MIME_PLATFORM_TEXT_INTENT);
    }

    long localTimeStamp = System.currentTimeMillis();

    private long timeStamp = 0L;
    private final AtomicInteger subscribeCount = new AtomicInteger(0);
    private ClipboardManager.OnPrimaryClipChangedListener listener = null;
    private ClipboardManager clipManager;
    private Runnable pasteboardChangeRunnable = () -> {
        onPasteboardChanged();
        timeStamp = 0;
    };
    private Handler mainHandler;
    private Context context;

    /**
     * ClipboardPlugin on AOSP platform
     *
     * @param context context of the application
     */
    public ClipboardAosp(Context context) {
        this.context = context;
        mainHandler = new Handler(context.getMainLooper());
        nativeInit();
    }

    private void initClipManager() {
        if (context != null) {
            Object service = context.getSystemService(Context.CLIPBOARD_SERVICE);
            if (service instanceof ClipboardManager) {
                this.clipManager = (ClipboardManager) service;
            } else {
                Log.e(LOG_TAG, "unable to get clipboard service");
                this.clipManager = null;
            }
        } else {
            Log.e(LOG_TAG, "context is null");
            this.clipManager = null;
        }
    }

    private boolean ensureClipManagerAvailable() {
        if (clipManager == null) {
            initClipManager();
        }
        if (clipManager == null) {
            Log.e(LOG_TAG, "clipboard manager is not available");
            return false;
        }
        return true;
    }

    private String convertToPlatformMimeType(String ohosMimeType) {
        if (ohosMimeType == null) {
            return null;
        }
        return MIME_TYPE_MAPPING.getOrDefault(ohosMimeType, ohosMimeType);
    }

    private String convertToOHMimeType(String platformMimeType) {
        if (platformMimeType == null) {
            return null;
        }
        for (Map.Entry<String, String> entry : MIME_TYPE_MAPPING.entrySet()) {
            if (entry.getValue().equals(platformMimeType)) {
                return entry.getKey();
            }
        }
        return platformMimeType;
    }

    private ClipData buildClipData(ClipDescription clipDescription, DataRecordDTO[] records) {
        List<ClipData.Item> itemList = new ArrayList<>();
        if (records != null && records.length > 0) {
            for (int i = 0; i < records.length; i++) {
                if (records[i] == null) {
                    continue;
                }
                ClipData.Item item = recordToClipItem(records[i]);
                if (item == null) {
                    return null;
                }
                itemList.add(item);
            }
        }
        if (itemList.isEmpty()) {
            return null;
        }

        try {
            ClipData clipData = new ClipData(clipDescription, itemList.get(0));
            for (int i = 1; i < itemList.size(); i++) {
                clipData.addItem(itemList.get(i));
            }
            return clipData;
        } catch (IllegalArgumentException e) {
            Log.e(LOG_TAG, "buildClipData: create ClipData failed");
            return null;
        }
    }

    private ClipData dtoToClipData(PasteDataDTO pasteDataDTO) {
        if (pasteDataDTO == null) {
            return null;
        }
        PropertyDTO propertyDTO = pasteDataDTO.getPropertyDTO();
        if (propertyDTO == null) {
            return null;
        }
        String[] dtoMimeTypes = propertyDTO.getMimeTypes();
        if (dtoMimeTypes == null) {
            return null;
        }
        String[] mimeTypes = new String[dtoMimeTypes.length + 1];
        for (int i = 0; i < dtoMimeTypes.length; i++) {
            if (dtoMimeTypes[i] != null) {
                mimeTypes[i] = convertToPlatformMimeType(dtoMimeTypes[i]);
            }
        }
        mimeTypes[dtoMimeTypes.length] = ARKUI_X_DATA_FLAG;
        String tag = propertyDTO.getTag() != null ? propertyDTO.getTag() : "";
        ClipDescription clipDescription = new ClipDescription(tag, mimeTypes);
        String jsonAdditions = propertyDTO.getJsonAdditions();
        if (jsonAdditions != null) {
            Bundle additionBundle = WantParamsUtils.jsonToExtras(jsonAdditions);
            WantParamsUtils.setExtrasToClipDescription(clipDescription, additionBundle);
        }
        DataRecordDTO[] records = pasteDataDTO.getRecords();
        if (records == null) {
            return null;
        }
        return buildClipData(clipDescription, records);
    }

    private ClipData.Item recordToClipItem(DataRecordDTO record) {
        String uriStr = record.getUri();
        Uri uri = null;
        if (uriStr != null) {
            try {
                uri = Uri.parse(uriStr);
            } catch (IllegalArgumentException e) {
                Log.e(LOG_TAG, "recordToClipItem: Invalid URI");
            }
        }
        String jsonWant = record.getJsonWant();
        Intent intent = new Intent();
        Boolean hasChanged = false;
        if (jsonWant != null && !jsonWant.isEmpty()) {
            Boolean isSuccess = WantParamsUtils.putJsonToIntent(intent, jsonWant);
            if (!isSuccess) {
                return null;
            }
            hasChanged = true;
        }
        if (!hasChanged) {
            intent = null;
        }
        String plainText = record.getPlainText();
        String htmlText = record.getHtmlText();
        if (plainText == null && htmlText != null) {
            Spanned htmlSpanned = Html.fromHtml(htmlText, Html.FROM_HTML_MODE_COMPACT);
            plainText = htmlSpanned.toString();
        }
        try {
            return new ClipData.Item(plainText, htmlText, intent, uri);
        } catch (FileUriExposedException e) {
            Log.e(LOG_TAG, "recordToClipItem new Item failed");
            return null;
        }
    }

    private DataRecordDTO buildDataRecordDTO(ClipData.Item item) {
        if (item == null) {
            Log.e(LOG_TAG, "buildDataRecordDTO: item is null");
            return null;
        }
        DataRecordDTO dataRecordDTO = new DataRecordDTO();
        String htmlText = item.getHtmlText();
        dataRecordDTO.setHtmlText(htmlText);
        Intent intent = item.getIntent();
        String jsonWant = null;
        if (intent != null) {
            jsonWant = WantParamsUtils.getJsonFromIntent(intent);
        }
        dataRecordDTO.setJsonWant(jsonWant);
        CharSequence plainText = item.getText();
        dataRecordDTO.setPlainText(plainText != null ? plainText.toString() : null);
        Uri uriObj = item.getUri();
        String uri = uriObj != null ? uriObj.toString() : null;
        dataRecordDTO.setUri(uri);
        return dataRecordDTO;
    }

    private void buildDataRecordsDTO(DataRecordDTO[] dataRecordsDTO, ClipData clipData) {
        if (dataRecordsDTO == null || clipData == null) {
            Log.e(LOG_TAG, "buildDataRecordsDTO: null params");
            return;
        }
        int itemCount = clipData.getItemCount();
        for (int i = 0; i < itemCount; i++) {
            dataRecordsDTO[i] = buildDataRecordDTO(clipData.getItemAt(i));
        }
    }

    private void buildPropertyDTO(PropertyDTO propertyDTO, ClipDescription clipDescription) {
        int mimeCount = clipDescription.getMimeTypeCount();
        String[] mimeTypes = getDescriptionMimeTypes();
        propertyDTO.setMimeTypes(mimeTypes);

        CharSequence label = clipDescription.getLabel();
        propertyDTO.setTag(label != null ? label.toString() : null);

        propertyDTO.setTimestamp(clipDescription.getTimestamp());
        propertyDTO.setJsonAdditions(WantParamsUtils.getJsonFromClipDescription(clipDescription));
    }

    private PasteDataDTO clipDataToDTO(ClipData clipData) {
        if (clipData == null) {
            return null;
        }
        PropertyDTO propertyDTO = new PropertyDTO();
        ClipDescription clipDescription = clipData.getDescription();
        if (propertyDTO == null || clipDescription == null) {
            Log.e(LOG_TAG, "clipDataToDTO: null propertyDTO or clipDescription");
            return null;
        }
        buildPropertyDTO(propertyDTO, clipDescription);
        int itemCount = clipData.getItemCount();
        if (itemCount == 0) {
            Log.e(LOG_TAG, "clipDataToDTO: clipData has no item");
            return null;
        }
        DataRecordDTO[] dataRecordsDTO = new DataRecordDTO[itemCount];
        buildDataRecordsDTO(dataRecordsDTO, clipData);

        PasteDataDTO pasteDataDTO = new PasteDataDTO();
        pasteDataDTO.setPropertyDTO(propertyDTO);
        pasteDataDTO.setRecords(dataRecordsDTO);
        return pasteDataDTO;
    }

    /**
     * Gets the data from the clipboard.
     *
     * @return The data from the clipboard, or null if no data is available.
     */
    public PasteDataDTO getData() {
        if (!ensureClipManagerAvailable()) {
            return null;
        }
        return clipDataToDTO(clipManager.getPrimaryClip());
    }

    /**
     * Sets the data to the clipboard.
     *
     * @param pasteDataDTO The data to be set.
     * @return 0 if the operation is successful, or an error code.
     */
    public int setData(PasteDataDTO pasteDataDTO) {
        if (!ensureClipManagerAvailable()) {
            return PasteboardError.INVALID_RETURN_VALUE_ERROR.getValue();
        }
        ClipData clipData = dtoToClipData(pasteDataDTO);
        if (clipData == null) {
            return PasteboardError.NO_DATA_ERROR.getValue();
        }

        try {
            clipManager.setPrimaryClip(clipData);
            return PasteboardError.E_OK.getValue();
        } catch (SecurityException e) {
            Log.e(LOG_TAG, "permission denied");
            return PasteboardError.PERMISSION_VERIFICATION_ERROR.getValue();
        } catch (IllegalStateException e) {
            Log.e(LOG_TAG, "clipboard service status exception");
            return PasteboardError.INVALID_PARAM_ERROR.getValue();
        } catch (RuntimeException e) {
            Log.e(LOG_TAG, "clipData size exceed system limit");
            return PasteboardError.OTHER_ERROR.getValue();
        }
    }

    /**
     * Checks whether the clipboard contains data.
     *
     * @return true if the clipboard contains data, false otherwise.
     */
    public boolean hasData() {
        if (!ensureClipManagerAvailable()) {
            return false;
        }
        try {
            return clipManager.hasPrimaryClip();
        } catch (SecurityException e) {
            Log.e(LOG_TAG, "permission denied");
            return false;
        } catch (IllegalStateException e) {
            Log.e(LOG_TAG, "clipboard service status exception");
            return false;
        }
    }

    /**
     * Clears the clipboard.
     *
     * @return true if the clipboard was cleared successfully, false otherwise.
     */
    public boolean clear() {
        if (!ensureClipManagerAvailable()) {
            return false;
        }
        try {
            // For API level 28 and above, use clearPrimaryClip directly
            if (android.os.Build.VERSION.SDK_INT >= VERSION_CODE_P) {
                // Use reflection to call clearPrimaryClip() method
                try {
                    java.lang.reflect.Method clearMethod = clipManager.getClass().getMethod("clearPrimaryClip");
                    clearMethod.invoke(clipManager);
                } catch (NoSuchMethodException | IllegalAccessException | InvocationTargetException e) {
                    Log.e(LOG_TAG, "reflection error when calling clearPrimaryClip");
                    return false;
                }
            } else {
                // For API level below 28, set an empty clip data to clear clipboard
                ClipData emptyClip = ClipData.newPlainText("", "");
                clipManager.setPrimaryClip(emptyClip);
            }
        } catch (SecurityException e) {
            Log.e(LOG_TAG, "permission denied");
            return false;
        } catch (IllegalStateException e) {
            Log.e(LOG_TAG, "clipboard service status exception");
            return false;
        }
        return true;
    }

    private String[] getDescriptionMimeTypes() {
        if (!ensureClipManagerAvailable()) {
            return null;
        }
        ClipData clipData = clipManager.getPrimaryClip();
        if (clipData != null) {
            ClipDescription clipDescription = clipData.getDescription();
            if (clipDescription == null) {
                return null;
            }
            boolean isMultiTypeData = clipDescription.hasMimeType(MULTI_TYPE_DATA_MIME);
            boolean isArkUIXData = clipDescription.hasMimeType(ARKUI_X_DATA_FLAG);
            int mimeCount = clipDescription.getMimeTypeCount();
            ArrayList<String> mimeTypes = new ArrayList<>();
            for (int i = 0; i < mimeCount; i++) {
                String mimeType = clipDescription.getMimeType(i);
                if (mimeType == null || MULTI_TYPE_DATA_MIME.equals(mimeType) || ARKUI_X_DATA_FLAG.equals(mimeType)) {
                    continue;
                }
                mimeType = convertToOHMimeType(mimeType);
                boolean isWantType = MIME_OH_TEXT_WANT.equals(mimeType);
                if (!((isWantType && isMultiTypeData) || (isWantType && !isArkUIXData))) {
                    mimeTypes.add(mimeType);
                }
            }
            return mimeTypes.toArray(new String[0]);
        }
        return null;
    }

    /**
     * Gets the MIME types of the data on the clipboard.
     *
     * @return An array of MIME types, or null if no data is available.
     */
    public String[] getMimeTypes() {
        if (!ensureClipManagerAvailable()) {
            return null;
        }
        ClipData clipData = clipManager.getPrimaryClip();
        if (clipData == null) {
            return null;
        }
        ClipDescription clipDescription = clipData.getDescription();
        if (clipDescription == null) {
            return null;
        }
        boolean isMultiTypeData = clipDescription.hasMimeType(MULTI_TYPE_DATA_MIME);
        boolean isArkUIXData = clipDescription.hasMimeType(ARKUI_X_DATA_FLAG);
        if (clipData != null) {
            ArrayList<String> mimeTypes = new ArrayList<>();
            int itemCount = clipData.getItemCount();
            for (int i = 0; i < itemCount; i++) {
                ClipData.Item item = clipData.getItemAt(i);
                if (item == null) {
                    continue;
                }
                if (item.getText() != null) {
                    mimeTypes.add(convertToOHMimeType(MIME_PLATFORM_TEXT_PLAIN));
                }
                if (item.getHtmlText() != null) {
                    mimeTypes.add(convertToOHMimeType(MIME_PLATFORM_TEXT_HTML));
                }
                if (item.getUri() != null) {
                    mimeTypes.add(convertToOHMimeType(MIME_PLATFORM_TEXT_URI_LIST));
                }
                if (item.getIntent() != null && isArkUIXData && !isMultiTypeData) {
                    mimeTypes.add(convertToOHMimeType(MIME_PLATFORM_TEXT_INTENT));
                }
            }
            return mimeTypes.toArray(new String[0]);
        }
        return null;
    }

    /**
     * Check whether the clipboard contains data of the specified mime type.
     *
     * @param mimeType Mime type of the data to check.
     * @return True if the clipboard contains data of the specified mime type, false otherwise.
     */
    public boolean hasDataType(String mimeType) {
        if (!ensureClipManagerAvailable()) {
            return false;
        }
        if (mimeType == null || mimeType.isEmpty()) {
            return false;
        }
        String tmpMimeType = convertToPlatformMimeType(mimeType);
        ClipData clipData = clipManager.getPrimaryClip();
        if (clipData != null) {
            ClipDescription clipDescription = clipData.getDescription();
            if (clipDescription != null) {
                boolean isMultiTypeData = clipDescription.hasMimeType(MULTI_TYPE_DATA_MIME);
                boolean isArkUIXData = clipDescription.hasMimeType(ARKUI_X_DATA_FLAG);
                boolean isWantType = MIME_OH_TEXT_WANT.equals(mimeType);
                if ((isMultiTypeData && isWantType) || (!isArkUIXData && isWantType)) {
                    return false;
                }
                return clipDescription.hasMimeType(tmpMimeType);
            }
        }
        return false;
    }

    /**
     * subscribePasteboardChange
     *
     * @return true if success, false otherwise
     */
    public boolean subscribePasteboardChange() {
        if (!ensureClipManagerAvailable()) {
            return false;
        }

        if (subscribeCount.getAndIncrement() > 0) {
            return true;
        }
        if (listener == null) {
            listener = new ClipboardManager.OnPrimaryClipChangedListener() {
                @Override
                public void onPrimaryClipChanged() {
                    if (clipManager.getPrimaryClipDescription() == null) {
                        mainHandler.post(pasteboardChangeRunnable);
                        return;
                    }
                    long stampl = clipManager.getPrimaryClipDescription().getTimestamp();

                    if (timeStamp == 0) {
                        timeStamp = stampl;
                    }
                    long temp = System.currentTimeMillis();
                    if ((timeStamp == stampl
                            && (temp - localTimeStamp) > CLIPBOARD_CHANGE_DELAY_MS) || (timeStamp != stampl)) {
                        localTimeStamp = temp;
                        mainHandler.removeCallbacks(pasteboardChangeRunnable);
                        mainHandler.post(pasteboardChangeRunnable);
                    }
                }
            };
        }
        clipManager.addPrimaryClipChangedListener(this.listener);
        return true;
    }

    /**
     * unsubscribePasteboardChange
     *
     * @return true if success, false otherwise
     */
    public boolean unsubscribePasteboardChange() {
        if (!ensureClipManagerAvailable()) {
            return false;
        }
        if (subscribeCount.decrementAndGet() > 0) {
            return true;
        }
        try {
            clipManager.removePrimaryClipChangedListener(this.listener);
        } catch (IllegalArgumentException e) {
            Log.e(LOG_TAG, "remove listener failed: listener not registered");
        }
        return true;
    }

    private ArrayList<String> getAllTextAndHtmlText() {
        if (!ensureClipManagerAvailable()) {
            return new ArrayList<>();
        }
        ClipData clipData = clipManager.getPrimaryClip();
        if (clipData == null) {
            return new ArrayList<>();
        }
        ArrayList<String> texts = new ArrayList<>();
        for (int i = 0; i < clipData.getItemCount(); i++) {
            ClipData.Item item = clipData.getItemAt(i);
            CharSequence text = item.getText();
            CharSequence htmlText = item.getHtmlText();
            if (text != null) {
                texts.add(text.toString());
            }
            if (htmlText != null) {
                texts.add(htmlText.toString());
            }
        }
        return texts;
    }

    private int checkPattern(String[] texts, int patternType, int result, int patterns) {
        int res = result;
        if ((patterns & patternType) != 0) {
            for (String text : texts) {
                if (PatternDetection.isMatch(text, patternType)) {
                    res = result | patternType;
                    break;
                }
            }
        }
        return res;
    }

    /**
     * detectPatterns
     * 0b0001: URL
     * 0b0010: NUMBER
     * 0b0100: EMAIL_ADDRESS
     * 0b0111: ALL
     * 0b0101: URL and EMAIL_ADDRESS
     * 0b0011: URL and NUMBER
     * 0b0000: NONE
     *
     * @param patterns bitmask of PatternDetection.Pattern
     * @return bitmask of PatternDetection.Pattern
     */
    public int detectPatterns(int patterns) {
        if (patterns == 0) {
            return 0;
        }
        Integer result = 0;
        String[] texts = getAllTextAndHtmlText().toArray(new String[0]);
        if (texts.length == 0) {
            return result;
        }
        result = checkPattern(texts, PatternDetection.URL, result, patterns);
        result = checkPattern(texts, PatternDetection.NUMBER, result, patterns);
        result = checkPattern(texts, PatternDetection.EMAIL_ADDRESS, result, patterns);
        return result;
    }

    /**
     * Initialize clipboard plugin
     */
    protected native void nativeInit();

    /**
     * onPasteboardChanged
     */
    protected native void onPasteboardChanged();
}