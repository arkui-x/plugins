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

 #import "DBManager.h"
 #import <sqlite3.h>

@interface DBManager () {
    sqlite3 *db_;
    NSString *dbPath_;
}
@end

@implementation DBManager

static DBManager *instance;
+ (DBManager *)shareManager {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[DBManager alloc] init];
    });
    return instance;
}

- (void)dealloc {
    NSLog(@"DBManager dealloc");
    sqlite3_close(db_);
    instance = nil;
}

- (BOOL)initDB {
    if (db_) {
        return YES;
    }
    // 获得沙盒中的数据库文件名
    dbPath_ = [[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject]
        stringByAppendingPathComponent:@"task.db"];

    sqlite3_shutdown();
    if (sqlite3_config(SQLITE_CONFIG_SERIALIZED) != SQLITE_OK) {
        NSLog(@"failed to sqlite3_config serialized");
    }
    sqlite3_initialize();

    if (![self openDB]) {
        return NO;
    }
    const char *sql = "CREATE TABLE IF NOT EXISTS Task (tid INTEGER PRIMARY KEY AUTOINCREMENT,"
        " saveas TEXT,"
        " url TEXT,"
        " data TEXT,"
        " title TEXT,"
        " description TEXT,"
        " action1 INTEGER,"
        " mode INTEGER,"
        " mimeType TEXT,"
        " progress TEXT,"
        " ctime INTEGER,"
        " mtime INTEGER,"
        " faults INTEGER,"
        " reason TEXT,"
        " taskStates TEXT,"
        " downloadId TEXT,"
        " token TEXT,"
        " roaming INTEGER,"
        " metered INTEGER,"
        " network INTEGER,"
        " headers TEXT,"
        " version INTEGER,"
        " index1 INTEGER,"
        " begins INTEGER,"
        " ends INTEGER,"
        " overwrite INTEGER,"
        " retry INTEGER,"
        " redirect INTEGER,"
        " gauge INTEGER,"
        " precise INTEGER,"
        " background INTEGER,"
        " method TEXT,"
        " forms TEXT,"
        " files TEXT,"
        " bodyFds TEXT,"
        " bodyFileNames TEXT,"
        " tries INTEGER,"
        " code INTEGER,"
        " withSystem INTEGER,"
        " extras TEXT);";
    char *error = NULL;
    if (sqlite3_exec(db_, sql, NULL, NULL, &error) != SQLITE_OK) {
        NSLog(@"failed to create table Task, %s", error);
        sqlite3_close(db_);
        return NO;
    }
    NSLog(@"init db ok");
    return YES;
}

- (int64_t)insert:(IosTaskInfo *)taskInfo {
    NSLog(@"insert db enter");
    if (![self openDB]) {
        return -1;
    }
    const char *sql = "INSERT INTO Task (saveas, url, data, title, description, action1, mode, mimeType, "
        "progress, ctime, mtime, faults, reason, taskStates, downloadId, token, "
        "roaming, metered, network, headers, version, index1, begins, ends, "
        "overwrite, retry, redirect, gauge, precise, background, method, "
        "forms, files, bodyFds, bodyFileNames, tries, code, withSystem, extras) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
        "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, NULL) != SQLITE_OK) {
        NSLog(@"failed to execute sqlite3_prepare_v2");
        [self releaseStmt:stmt];
        return -1;
    }
    sqlite3_bind_text(stmt, 1, taskInfo.saveas.UTF8String, -1, NULL);
    sqlite3_bind_text(stmt, 2, taskInfo.url.UTF8String, -1, NULL);
    sqlite3_bind_text(stmt, 3, taskInfo.data.UTF8String, -1, NULL);
    sqlite3_bind_text(stmt, 4, taskInfo.title.UTF8String, -1, NULL);
    sqlite3_bind_text(stmt, 5, taskInfo.desc.UTF8String, -1, NULL);
    sqlite3_bind_int(stmt, 6, taskInfo.action);
    sqlite3_bind_int(stmt, 7, taskInfo.mode);
    sqlite3_bind_text(stmt, 8, taskInfo.mimeType.UTF8String, -1, NULL);
    sqlite3_bind_text(stmt, 9, taskInfo.progress.UTF8String, -1, NULL);
    sqlite3_bind_int64(stmt, 10, taskInfo.ctime);
    sqlite3_bind_int64(stmt, 11, taskInfo.mtime);
    sqlite3_bind_int(stmt, 12, taskInfo.faults);
    sqlite3_bind_text(stmt, 13, taskInfo.reason.UTF8String, -1, NULL);
    sqlite3_bind_text(stmt, 14, taskInfo.taskStates.UTF8String, -1, NULL);
    sqlite3_bind_int(stmt, 15, taskInfo.downloadId);
    sqlite3_bind_text(stmt, 16, taskInfo.token.UTF8String, -1, NULL);
    sqlite3_bind_int(stmt, 17, taskInfo.roaming);
    sqlite3_bind_int(stmt, 18, taskInfo.metered);
    sqlite3_bind_int(stmt, 19, taskInfo.network);
    sqlite3_bind_text(stmt, 20, taskInfo.headers.UTF8String, -1, NULL);
    sqlite3_bind_int(stmt, 21, taskInfo.version);
    sqlite3_bind_int(stmt, 22, taskInfo.index);
    sqlite3_bind_int64(stmt, 23, taskInfo.begins);
    sqlite3_bind_int64(stmt, 24, taskInfo.ends);
    sqlite3_bind_int(stmt, 25, taskInfo.overwrite);
    sqlite3_bind_int(stmt, 26, taskInfo.retry);
    sqlite3_bind_int(stmt, 27, taskInfo.redirect);
    sqlite3_bind_int(stmt, 28, taskInfo.gauge);
    sqlite3_bind_int(stmt, 29, taskInfo.precise);
    sqlite3_bind_int(stmt, 30, taskInfo.background);
    sqlite3_bind_text(stmt, 31, taskInfo.method.UTF8String, -1, NULL);
    sqlite3_bind_text(stmt, 32, taskInfo.forms.UTF8String, -1, NULL);
    sqlite3_bind_text(stmt, 33, taskInfo.files.UTF8String, -1, NULL);
    sqlite3_bind_text(stmt, 34, taskInfo.bodyFds.UTF8String, -1, NULL);
    sqlite3_bind_text(stmt, 35, taskInfo.bodyFileNames.UTF8String, -1, NULL);
    sqlite3_bind_int(stmt, 36, taskInfo.tries);
    sqlite3_bind_int(stmt, 37, taskInfo.code);
    sqlite3_bind_int(stmt, 38, taskInfo.withSystem);
    sqlite3_bind_text(stmt, 39, taskInfo.extras.UTF8String, -1, NULL);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        NSLog(@"failed to insert record: %s", sqlite3_errmsg(db_));
        [self releaseStmt:stmt];
        return -1;
    }
    int64_t tid = sqlite3_last_insert_rowid(db_); // 获取最后插入的记录的主键值
    [self releaseStmt:stmt];
    NSLog(@"insert db ok, tid:%lld", tid);
    return tid;
}

- (NSArray *)queryAll {
    NSLog(@"queryAll enter");

    if (![self openDB]) {
        return nil;
    }
    const char *sql = "SELECT * FROM Task;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, NULL) != SQLITE_OK) {
        NSLog(@"failed to execute sqlite3_prepare_v2");
        [self releaseStmt:stmt];
        return nil;
    }
    NSMutableArray *tasks = [self getQueryResult:stmt];
    [self releaseStmt:stmt];
    NSLog(@"queryAll ok");
    return tasks;
}

- (IosTaskInfo *)queryWithTaskId:(int64_t)taskId {
    NSLog(@"queryWithTaskId, taskId:%lld", taskId);

    if (![self openDB]) {
        return nil;
    }
    const char *sql = "SELECT * FROM Task WHERE tid=?;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, NULL) != SQLITE_OK) {
        NSLog(@"failed to execute sqlite3_prepare_v2");
        [self releaseStmt:stmt];
        return nil;
    }
    sqlite3_bind_int64(stmt, 1, taskId);

    NSMutableArray *tasks = [self getQueryResult:stmt];
    [self releaseStmt:stmt];
    if (tasks.count > 0) {
        NSLog(@"queryWithTaskId ok, tasks.count:%ld", tasks.count);
        return [tasks firstObject];;
    }
    return nil;
}

- (IosTaskInfo *)queryWithToken:(NSString *)token taskId:(int64_t)taskId {
    NSLog(@"queryWithToken, taskId:%lld, token:%@", taskId, token);

    if (![self openDB]) {
        return nil;
    }
    const char *sql = "SELECT * FROM Task WHERE tid=? AND token=?;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, NULL) != SQLITE_OK) {
        NSLog(@"failed to execute sqlite3_prepare_v2");
        [self releaseStmt:stmt];
        return nil;
    }
    sqlite3_bind_int64(stmt, 1, taskId);
    sqlite3_bind_text(stmt, 2, token.UTF8String, -1, NULL);

    NSMutableArray *tasks = [self getQueryResult:stmt];
    [self releaseStmt:stmt];
    if (tasks.count > 0) {
        NSLog(@"queryWithToken ok, tasks.count:%ld", tasks.count);
        return [tasks firstObject];
    }
    NSLog(@"queryWithToken failed");
    return nil;
}

- (NSArray *)queryWithFilter:(IosTaskFilter *)filter {
    NSLog(@"queryWithFilter enter");
    if (!filter) {
        return nil;
    }
    if (![self openDB]) {
        return nil;
    }

    NSString *strFilter = @"";
    if (filter.before > 0) {
        strFilter = [strFilter stringByAppendingFormat:@"ctime<%lld ", filter.before];
    }
    if (filter.after > 0) {
        if (strFilter.length > 0) {
            strFilter = [strFilter stringByAppendingString:@" AND "];
        }
        strFilter = [strFilter stringByAppendingFormat:@"ctime>%lld ", filter.after];
    }
    if (filter.action == 0 || filter.action == 1) {
        if (strFilter.length > 0) {
            strFilter = [strFilter stringByAppendingString:@" AND "];
        }
        strFilter = [strFilter stringByAppendingFormat:@"action1=%d ", filter.action];
    }
    if (filter.mode == 1) {
        if (strFilter.length > 0) {
            strFilter = [strFilter stringByAppendingString:@" AND "];
        }
        strFilter = [strFilter stringByAppendingFormat:@"mode=%d ", filter.mode];
    }
    NSString *strSql = @"SELECT tid FROM Task";
    NSLog(@"queryWithFilter, strFilter:%@", strFilter);
    if (strFilter.length > 0) {
        strSql = [strSql stringByAppendingFormat:@" WHERE %@", strFilter];
    }
    const char *sql = strSql.UTF8String;
    NSLog(@"queryWithFilter, sql:%s", sql);
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, NULL) != SQLITE_OK) {
        NSLog(@"failed to execute sqlite3_prepare_v2");
        [self releaseStmt:stmt];
        return nil;
    }

    NSMutableArray *taskIds = [NSMutableArray array];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int64_t taskId = sqlite3_column_int64(stmt, 0);
        [taskIds addObject:[NSNumber numberWithLongLong:taskId]];
    }
    [self releaseStmt:stmt];
    NSLog(@"queryWithFilter end, taskIds.count:%ld", taskIds.count);

    return taskIds;
}

- (BOOL)update:(IosTaskInfo *)taskInfo {
    NSLog(@"update db enter, tid:%lld", taskInfo.tid);
    if (!taskInfo) {
        return NO;
    }

    if (![self openDB]) {
        return NO;
    }

    const char *sql = "UPDATE Task SET url=?, data=?, title=?, description=?, "
        "action1=?, mode=?, mimeType=?, progress=?, ctime=?, mtime=?, faults=?, "
        "reason=?, downloadId=?, taskStates=?, version=?, files=?, code=? WHERE tid =?;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, NULL) != SQLITE_OK) {
        NSLog(@"failed to execute sqlite3_prepare_v2");
        [self releaseStmt:stmt];
        return NO;
    }
    sqlite3_bind_text(stmt, 1, taskInfo.url.UTF8String, -1, NULL);
    sqlite3_bind_text(stmt, 2, taskInfo.data.UTF8String, -1, NULL);
    sqlite3_bind_text(stmt, 3, taskInfo.title.UTF8String, -1, NULL);
    sqlite3_bind_text(stmt, 4, taskInfo.desc.UTF8String, -1, NULL);
    sqlite3_bind_int(stmt, 5, taskInfo.action);
    sqlite3_bind_int(stmt, 6, taskInfo.mode);
    sqlite3_bind_text(stmt, 7, taskInfo.mimeType.UTF8String, -1, NULL);
    sqlite3_bind_text(stmt, 8, taskInfo.progress.UTF8String, -1, NULL);
    sqlite3_bind_int64(stmt, 9, taskInfo.ctime);
    sqlite3_bind_int64(stmt, 10, taskInfo.mtime);
    sqlite3_bind_int(stmt, 11, taskInfo.faults);
    sqlite3_bind_text(stmt, 12, taskInfo.reason.UTF8String, -1, NULL);
    sqlite3_bind_int(stmt, 13, taskInfo.downloadId);
    sqlite3_bind_text(stmt, 14, taskInfo.taskStates.UTF8String, -1, NULL);
    sqlite3_bind_int(stmt, 15, taskInfo.version);
    sqlite3_bind_text(stmt, 16, taskInfo.files.UTF8String, -1, NULL);
    sqlite3_bind_int(stmt, 17, taskInfo.code);
    sqlite3_bind_int64(stmt, 18, taskInfo.tid);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        NSLog(@"failed to update database");
        [self releaseStmt:stmt];
        return NO;
    }
    [self releaseStmt:stmt];
    NSLog(@"update db ok");
    return YES;
}

- (BOOL)remove:(int64_t)taskId {
    NSLog(@"remove, taskId:%lld", taskId);

    if (![self openDB]) {
        return NO;
    }
    const char *sql = "DELETE FROM Task WHERE tid=?;";
    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, NULL) != SQLITE_OK) {
        NSLog(@"failed to execute sqlite3_prepare_v2");
        [self releaseStmt:stmt];
        return NO;
    }
    sqlite3_bind_int64(stmt, 1, taskId);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        NSLog(@"failed to delete, taskId:%lld", taskId);
        [self releaseStmt:stmt];
        return NO;
    }
    [self releaseStmt:stmt];
    NSLog(@"remove ok");
    return YES;
}

#pragma mark - Private Methods

- (BOOL)openDB {
    if (db_) {
        return YES;
    }
    if (sqlite3_open(dbPath_.UTF8String, &db_) != SQLITE_OK) {
        NSLog(@"failed to open database, %@", sqlite3_errmsg(db_));
        sqlite3_close(db_);
        return NO;
    }
    return YES;
}

- (NSMutableArray *)getQueryResult:(sqlite3_stmt *)stmt  {
    if (!stmt) {
        return nil;
    }
    NSMutableArray *tasks = [NSMutableArray array];
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        IosTaskInfo *taskInfo = [[IosTaskInfo alloc] init];
        taskInfo.tid = sqlite3_column_int64(stmt, 0);
        taskInfo.saveas = [NSString stringWithUTF8String:(const char *)sqlite3_column_text(stmt, 1)];
        taskInfo.url = [NSString stringWithUTF8String:(const char *)sqlite3_column_text(stmt, 2)];
        taskInfo.data = [NSString stringWithUTF8String:(const char *)sqlite3_column_text(stmt, 3)];
        taskInfo.title = [NSString stringWithUTF8String:(const char *)sqlite3_column_text(stmt, 4)];
        taskInfo.desc = [NSString stringWithUTF8String:(const char *)sqlite3_column_text(stmt, 5)];
        taskInfo.action = sqlite3_column_int(stmt, 6);
        taskInfo.mode = sqlite3_column_int(stmt, 7);
        taskInfo.mimeType = [NSString stringWithUTF8String:(const char *)sqlite3_column_text(stmt, 8)];
        taskInfo.progress = [NSString stringWithUTF8String:(const char *)sqlite3_column_text(stmt, 9)];
        taskInfo.ctime = sqlite3_column_int64(stmt, 10);
        taskInfo.mtime = sqlite3_column_int64(stmt, 11);
        taskInfo.faults = sqlite3_column_int(stmt, 12);
        taskInfo.reason = [NSString stringWithUTF8String:(const char *)sqlite3_column_text(stmt, 13)];
        taskInfo.taskStates = [NSString stringWithUTF8String:(const char *)sqlite3_column_text(stmt, 14)];
        taskInfo.downloadId = sqlite3_column_int(stmt, 15);
        taskInfo.token = [NSString stringWithUTF8String:(const char *)sqlite3_column_text(stmt, 16)];
        taskInfo.roaming = sqlite3_column_int(stmt, 17);
        taskInfo.metered = sqlite3_column_int(stmt, 18);
        taskInfo.network = sqlite3_column_int(stmt, 19);
        taskInfo.headers = [NSString stringWithUTF8String:(const char *)sqlite3_column_text(stmt, 20)];
        taskInfo.version = sqlite3_column_int(stmt, 21);
        taskInfo.index = sqlite3_column_int(stmt, 22);
        taskInfo.begins = sqlite3_column_int(stmt, 23);
        taskInfo.ends = sqlite3_column_int(stmt, 24);
        taskInfo.overwrite = sqlite3_column_int(stmt, 25);
        taskInfo.retry = sqlite3_column_int(stmt, 26);
        taskInfo.redirect = sqlite3_column_int(stmt, 27);
        taskInfo.gauge = sqlite3_column_int(stmt, 28);
        taskInfo.precise = sqlite3_column_int(stmt, 29);
        taskInfo.background = sqlite3_column_int(stmt, 30);
        taskInfo.method = [NSString stringWithUTF8String:(const char *)sqlite3_column_text(stmt, 31)];
        taskInfo.forms = [NSString stringWithUTF8String:(const char *)sqlite3_column_text(stmt, 32)];
        taskInfo.files = [NSString stringWithUTF8String:(const char *)sqlite3_column_text(stmt, 33)];
        taskInfo.bodyFds = [NSString stringWithUTF8String:(const char *)sqlite3_column_text(stmt, 34)];
        taskInfo.bodyFileNames = [NSString stringWithUTF8String:(const char *)sqlite3_column_text(stmt, 35)];
        taskInfo.tries = sqlite3_column_int(stmt, 36);
        taskInfo.code = sqlite3_column_int(stmt, 37);
        taskInfo.withSystem = sqlite3_column_int(stmt, 38);
        taskInfo.extras = [NSString stringWithUTF8String:(const char *)sqlite3_column_text(stmt, 39)];
        [tasks addObject:taskInfo];
    }
    NSLog(@"getQueryResult ok");
    return tasks;
}

- (void)releaseStmt:(sqlite3_stmt *)stmt {
    sqlite3_finalize(stmt);
    NSLog(@"releaseStmt ok");
}

@end
