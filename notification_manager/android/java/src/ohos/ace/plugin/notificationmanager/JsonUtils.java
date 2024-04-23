package ohos.ace.plugin.notificationmanager;

import android.app.Notification;
import android.text.TextUtils;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.Iterator;

public class JsonUtils {
    private static final String TAG = JsonUtils.class.getSimpleName();

    public static NotificationRequest getNotificationRequest(String jsonString) {
        NotificationRequest request = new NotificationRequest();
        if (TextUtils.isEmpty(jsonString)) {
            return request;
        }
        try {
            JSONObject object = new JSONObject(jsonString);
            request.id = object.optInt("id");
            request.label = object.optString("label");
            request.deliveryTime = object.optLong("deliveryTime");
            request.autoDeletedTime = object.optLong("autoDeletedTime");

            request.isAlertOnce = object.optBoolean("isAlertOnce");
            request.tapDismissed = object.optBoolean("tapDismissed");
            request.isStopwatch = object.optBoolean("isStopwatch");
            request.isCountdown = object.optBoolean("isCountdown");
            request.isOngoing = object.optBoolean("isOngoing");
            request.showDeliveryTime = object.optBoolean("showDeliveryTime");

            request.smallIcon = object.optString("smallIcon");
            request.largeIcon = object.optString("largeIcon");
            request.badgeNumber = object.optInt("badgeNumber");
            request.notificationContentType = object.optString("notificationContentType");
            request.progressMax = object.optInt("progressMax");
            request.progressValue = object.optInt("progressValue");

            request.content = getContent(object.getJSONObject("content"));
        } catch (JSONException ex) {
            Log.e(TAG, "getNotificationRequest error");
        }
        return request;
    }

    private static NotificationRequest.NotificationContent getContent(JSONObject contentObject) {
        NotificationRequest.NotificationContent content = new NotificationRequest.NotificationContent();

        try {
            content.contentType = contentObject.optInt("contentType");
            JSONObject object = contentObject.getJSONObject("content");
            content.content = getBasicContent(content.contentType, object);
        } catch (JSONException ex) {
            Log.e(TAG, "getStringArray error");
        }
        return content;
    }

    private static NotificationRequest.NotificationBasicContent getBasicContent(int contentType,
            JSONObject contentObject) {
        if (contentType == NotificationTools.ContentType.LONG_TEXT.getValue()) {
            NotificationRequest.NotificationLongTextContent longTextContent = new NotificationRequest.NotificationLongTextContent();
            longTextContent.title = contentObject.optString("title");
            longTextContent.text = contentObject.optString("text");
            longTextContent.additionalText = contentObject.optString("additionalText");

            longTextContent.briefText = contentObject.optString("briefText");
            longTextContent.expandedTitle = contentObject.optString("expandedTitle");
            longTextContent.longText = contentObject.optString("longText");

            return longTextContent;
        }
        if (contentType == NotificationTools.ContentType.MULTILINE.getValue()) {
            NotificationRequest.NotificationMultiLineContent linesTextContent = new NotificationRequest.NotificationMultiLineContent();
            linesTextContent.title = contentObject.optString("title");
            linesTextContent.text = contentObject.optString("text");
            linesTextContent.additionalText = contentObject.optString("additionalText");

            linesTextContent.briefText = contentObject.optString("briefText");
            linesTextContent.expandedTitle = contentObject.optString("expandedTitle");

            JSONArray jsonArray = contentObject.optJSONArray("allLines");
            linesTextContent.allLines = getStringArray(jsonArray);
            return linesTextContent;
        }
        if (contentType == NotificationTools.ContentType.PICTURE.getValue()) {
            NotificationRequest.NotificationPictureContent pictureContent = new NotificationRequest.NotificationPictureContent();
            pictureContent.title = contentObject.optString("title");
            pictureContent.text = contentObject.optString("text");
            pictureContent.additionalText = contentObject.optString("additionalText");

            pictureContent.briefText = contentObject.optString("briefText");
            pictureContent.expandedTitle = contentObject.optString("expandedTitle");
            pictureContent.picture = contentObject.optString("picture");

            return pictureContent;
        }

        NotificationRequest.NotificationBasicContent content = new NotificationRequest.NotificationBasicContent();
        content.title = contentObject.optString("title");
        content.text = contentObject.optString("text");
        content.additionalText = contentObject.optString("additionalText");
        return content;
    }

    private static String[] getStringArray(JSONArray jsonArray) {
        if (jsonArray == null) {
            return null;
        }
        try {
            String[] stringArray = new String[jsonArray.length()];
            for (int i = 0; i < jsonArray.length(); i++) {
                stringArray[i] = jsonArray.getString(i);
            }
            return stringArray;
        } catch (JSONException ex) {
            Log.e(TAG, "getStringArray error");
        }
        return null;
    }
}
