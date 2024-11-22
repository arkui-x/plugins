/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

package ohos.ace.plugin.photoaccesshelper;

import android.content.ContentQueryMap;
import android.content.ContentValues;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class AlbumValues {
    private ContentQueryMap albumMap;

    private Iterator<Map.Entry<String, ContentValues>> mIterator;

    public AlbumValues(ContentQueryMap albumMap) {
        this.albumMap = albumMap;
        getFirstValue();
    }

    public List<String> getColumns() {
        if (albumMap == null) {
            return null;
        }
        Map<String, ContentValues> rows = albumMap.getRows();
        List<String> columns = new ArrayList<>();
        for (String id : rows.keySet()) {
            ContentValues contentValues = rows.get(id);
            Set<String> contentColumns = contentValues.keySet();
            if (contentColumns != null && !contentColumns.isEmpty()) {
                columns.addAll(contentValues.keySet());
                break;
            }
        }
        return columns;
    }

    public ContentValues getFirstValue() {
        Map<String, ContentValues> rows = albumMap.getRows();
        Set<Map.Entry<String, ContentValues>> entry = rows.entrySet();
        mIterator = entry.iterator();
        if (mIterator == null || !mIterator.hasNext()) {
            return null;
        }
        return mIterator.next().getValue();
    }

    public ContentValues getNextValue() {
        if (mIterator == null || !mIterator.hasNext()) {
            return null;
        }
        return mIterator.next().getValue();
    }

    public boolean IsAtLastRow() {
        if (mIterator == null) {
            return true;
        }
        return !mIterator.hasNext();
    }

    public ContentValues getLastValue() {
        if (mIterator == null) {
            return null;
        }
        ContentValues values = null;
        if (!mIterator.hasNext()) {
            values = getFirstValue();
        }
        while (mIterator.hasNext()) {
            values = mIterator.next().getValue();
        }
        return values;
    }

    public ContentValues gotoRow(int position) {
        if (albumMap == null || albumMap.getRows().size() <= position) {
            return null;
        }
        Map<String, ContentValues> rows = albumMap.getRows();
        Set<Map.Entry<String, ContentValues>> entry = rows.entrySet();
        mIterator = entry.iterator();
        while (position-- > 0) {
            mIterator.next();
        }
        return mIterator.next().getValue();
    }

    public ContentValues move(int offset) {
        if (mIterator == null) {
            getFirstValue();
            offset--;
        }
        while (offset-- > 0) {
            mIterator.next();
        }
        return mIterator.next().getValue();
    }

    public byte[] getAsBoolean(ContentValues contentValues, String key) {
        if (contentValues == null) {
            return null;
        }
        Object value = contentValues.getAsBoolean(key);
        return (byte[]) value;
    }

    public int size() {
        if (albumMap == null || albumMap.getRows() == null) {
            return 0;
        }
        return albumMap.getRows().size();
    }
}
