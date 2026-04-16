/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Test Activity that overrides startActivityForResult to capture intents.
 */
package ohos.ace.plugin.test;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

public class IntentCaptorActivity extends Activity {
    private Intent lastStartedIntent;
    private int lastRequestCode = -1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    public void startActivityForResult(Intent intent, int requestCode) {
        this.lastStartedIntent = intent;
        this.lastRequestCode = requestCode;
    }

    public Intent getLastStartedIntent() {
        return lastStartedIntent;
    }

    public int getLastRequestCode() {
        return lastRequestCode;
    }

    public void reset() {
        lastStartedIntent = null;
        lastRequestCode = -1;
    }
}
