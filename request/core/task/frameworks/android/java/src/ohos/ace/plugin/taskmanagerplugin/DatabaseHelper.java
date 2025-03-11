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

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

/**
 * DatabaseHelper class for task manager plugin database operations.
 *
 * @since 2024-05-31
 */
public class DatabaseHelper extends SQLiteOpenHelper {
    private static final String DATABASE_NAME = "task.db";
    private static final int DATABASE_VERSION = 1;

    public DatabaseHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        String createTaskTable = "CREATE TABLE Task (" +
                "tid INTEGER PRIMARY KEY AUTOINCREMENT," +
                " saveas TEXT," +
                " url TEXT," +
                " data TEXT," +
                " title TEXT," +
                " description TEXT," +
                " action1 INTEGER," +
                " mode INTEGER," +
                " mimeType TEXT," +
                " progress TEXT," +
                " ctime INTEGER," +
                " mtime INTEGER," +
                " faults INTEGER," +
                " reason TEXT," +
                " taskStates TEXT," +
                " downloadId TEXT," +
                " token TEXT," +
                " roaming INTEGER," +
                " metered INTEGER," +
                " network INTEGER," +
                " headers TEXT," +
                " version INTEGER," +
                " index1 INTEGER," +
                " begins INTEGER," +
                " ends INTEGER," +
                " priority INTEGER," +
                " overwrite INTEGER," +
                " retry INTEGER," +
                " redirect INTEGER," +
                " gauge INTEGER," +
                " precise INTEGER," +
                " background INTEGER," +
                " method TEXT," +
                " forms TEXT," +
                " files TEXT," +
                " bodyFds TEXT," +
                " bodyFileNames TEXT," +
                " tries INTEGER," +
                " code INTEGER," +
                " withSystem INTEGER," +
                " extras TEXT" +
                ")";
        db.execSQL(createTaskTable);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {

    }
}
