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

#import <Security/Security.h>
#import "http_ios_constant.h"
#import "http_ios_request.h"

@implementation http_ios_param
@end

@interface QueryStringPair : NSObject
@property (readwrite, nonatomic, strong) id field;
@property (readwrite, nonatomic, strong) id value;

- (instancetype)initWithField:(id)field value:(id)value;
- (NSString*)URLEncodedStringValue;
@end

@implementation QueryStringPair

- (instancetype)initWithField:(id)field value:(id)value
{
    self = [super init];
    if (!self) {
        return nil;
    }

    self.field = field;
    self.value = value;

    return self;
}

- (NSString*)URLEncodedStringValue
{
    if (!self.value || [self.value isEqual:[NSNull null]]) {
        return PercentEscapedStringFromString([self.field description]);
    } else {
        NSString* key = PercentEscapedStringFromString([self.field description]);
        NSString* value = PercentEscapedStringFromString([self.value description]);
        return [NSString stringWithFormat:@"%@=%@", key, value];
    }
}

NSString* PercentEscapedStringFromString(NSString* string) {
    static NSString* const kCharactersGeneralDelimitersToEncode = @":#[]@";
    static NSString* const kCharactersSubToEncode = @"!$&'()*+,;=";

    NSMutableCharacterSet* allowedCharacterSet = [[NSCharacterSet URLQueryAllowedCharacterSet] mutableCopy];
    NSString* encodeString = [kCharactersGeneralDelimitersToEncode stringByAppendingString:kCharactersSubToEncode];
    [allowedCharacterSet removeCharactersInString:encodeString];

    static NSUInteger const batchSize = 50;

    NSUInteger index = 0;
    NSMutableString* escaped = @"".mutableCopy;

    while (index < string.length) {
        NSUInteger length = MIN(string.length - index, batchSize);
        NSRange range = NSMakeRange(index, length);

        range = [string rangeOfComposedCharacterSequencesForRange:range];

        NSString* substring = [string substringWithRange:range];
        NSString* encoded = [substring stringByAddingPercentEncodingWithAllowedCharacters:allowedCharacterSet];
        [escaped appendString:encoded];

        index += range.length;
    }

    return escaped;
}

@end

@interface http_ios_request () <NSURLSessionDataDelegate>
@property (nonatomic, strong) NSMutableData* mutableData;
@property (nonatomic) void* context;
@property (nonatomic, strong) http_ios_param* requestParam;
@property (nonatomic, strong) NSURLSession* urlSession;
@property (nonatomic, strong) NSMutableURLRequest* request;
@end
@implementation http_ios_request

+ (NSString *)getBaseCachePath 
{
    NSString *cachePath = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES).firstObject;
    NSString * path = [NSString stringWithFormat:@"%@/http",cachePath];

    if ([self isExistFileForPath:path]) {
        return path;
    }
    if ([self createDirectoryForPath:path]) {
        NSString * filePath = [NSString stringWithFormat:@"%@/cache.json",path];
        if ([self isExistFileForPath:filePath]) {
            return filePath;
        }
        NSFileManager *fileManager = [NSFileManager defaultManager];
        [fileManager createFileAtPath:filePath contents:nil attributes:nil];
        return filePath;
    }
    return @"";
}

- (NSURLSessionDataTask*)sendRequestWith:(http_ios_param*)requestParam
{  
    _mutableData = [NSMutableData data];
    _requestParam = requestParam;

    self.request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:requestParam.urlPath]];
    self.request.HTTPMethod = requestParam.method;

    if (requestParam.headerJson) {
        for (NSString* key in requestParam.headerJson) {
            [self.request setValue:requestParam.headerJson[key] forHTTPHeaderField:key];
        }
    }

    self.context = requestParam.context;

    NSString* acceptEncoding = @"gzip";
    [self.request setValue:acceptEncoding forHTTPHeaderField:@"Accept-Encoding"];

    NSURLSessionConfiguration* sessionConfig = [NSURLSessionConfiguration defaultSessionConfiguration];
    sessionConfig.timeoutIntervalForRequest = requestParam.connectTimeout;
    sessionConfig.timeoutIntervalForResource = requestParam.readTimeout;
    sessionConfig.HTTPShouldUsePipelining = true;

    if (requestParam.usingHttpProxyType) {
        if (requestParam.proxyhost && requestParam.proxyhost.length != 0 ) {
            sessionConfig.connectionProxyDictionary = 
                [self setproxyId:requestParam.proxyhost proxyPort:requestParam.proxyport];
        }
    }

    self.urlSession = [NSURLSession sessionWithConfiguration:sessionConfig delegate:self delegateQueue:nil];

    [self serializingQueryParams];

    NSURLSessionDataTask* task = [self.urlSession dataTaskWithRequest:self.request];
    task.priority = requestParam.priority;
    [task resume];

    return task;
}

#pragma mark - delegate

- (void)URLSession:(NSURLSession*)session task:(NSURLSessionTask*)task didCompleteWithError:(NSError*)error
{
    NSHTTPURLResponse* response = (NSHTTPURLResponse*)task.response;
    if (error) {
        if (self.failBlock) {
            NSInteger errorCode;
            if ([ErrorCodeMap.allKeys containsObject:@(error.code)]) {
                NSNumber* num = ErrorCodeMap[@(error.code)];
                errorCode = num.integerValue;
            } else {
                errorCode = error.code;
            }
            self.failBlock(errorCode, error.localizedDescription, self.context);
        }
    } else {
        if (self.responseBlock) {
            self.responseBlock(task, response, self.mutableData, self.context);
        }
    }
}

- (void)URLSession:(NSURLSession*)session dataTask:(NSURLSessionDataTask*)dataTask didReceiveData:(NSData*)data
{
    [self.mutableData appendData:data];
    if (self.downloadProgress) {
        self.downloadProgress(self.context, dataTask.countOfBytesReceived, dataTask.countOfBytesExpectedToReceive);
    }
    if (self.memoryBodyBlock) {
        self.memoryBodyBlock(data, self.context);
    }
}

- (void)URLSession:(NSURLSession*)session dataTask:(NSURLSessionDataTask*)dataTask
    didReceiveResponse:(nonnull NSURLResponse*)response
    completionHandler:(nonnull void (^)(NSURLSessionResponseDisposition))completionHandler
{
    if ([response isKindOfClass:[NSHTTPURLResponse class]]) {
        if (self.memoryHeaderBlock) {
            NSHTTPURLResponse* httpResponse = (NSHTTPURLResponse*)response;
            NSDictionary* headers = [httpResponse allHeaderFields];
            self.memoryHeaderBlock(headers, self.context);
        }
    }
    completionHandler(NSURLSessionResponseAllow);
}

- (void)URLSession:(NSURLSession*)session task:(NSURLSessionTask*)task
    didSendBodyData:(int64_t)bytesSent
    totalBytesSent:(int64_t)totalBytesSent
    totalBytesExpectedToSend:(int64_t)totalBytesExpectedToSend
{
    if (self.uploadProgress) {
        self.uploadProgress(self.context, totalBytesSent, totalBytesExpectedToSend);
    }
}

- (void)URLSession:(nonnull NSURLSession*)session
    downloadTask:(nonnull NSURLSessionDownloadTask*)downloadTask
    didWriteData:(int64_t)bytesWritten
    totalBytesWritten:(int64_t)totalBytesWritten
    totalBytesExpectedToWrite:(int64_t)totalBytesExpectedToWrite
{
    if (self.downloadProgress) {
        self.downloadProgress(self.context, totalBytesWritten, totalBytesExpectedToWrite);
    }
}

- (void)URLSession:(NSURLSession*)session
    task:(NSURLSessionTask*)task
    willPerformHTTPRedirection:(NSHTTPURLResponse*)response
    newRequest:(NSURLRequest*)request
    completionHandler:(void (^)(NSURLRequest* _Nullable))completionHandler
{
    NSURLRequest* redirectRequest = request;
    if (self.redirectionBlock) {
        redirectRequest = self.redirectionBlock(session, task, response, request);
    }
    if (completionHandler) {
        completionHandler(redirectRequest);
    }
}

- (void)URLSession:(NSURLSession*)session
    didReceiveChallenge:(NSURLAuthenticationChallenge*)challenge
    completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition, NSURLCredential* _Nullable))completionHandler
{
    if (challenge.protectionSpace.authenticationMethod == NSURLAuthenticationMethodServerTrust) {
        NSURLCredential* credential = [NSURLCredential credentialForTrust:challenge.protectionSpace.serverTrust];
        if (credential) {
            completionHandler(NSURLSessionAuthChallengeUseCredential, credential);
        } else {
            completionHandler(NSURLSessionAuthChallengeCancelAuthenticationChallenge, credential);
        }
    } else if ([challenge.protectionSpace.authenticationMethod
                isEqualToString:NSURLAuthenticationMethodClientCertificate]) {
        NSData *p12Data = [NSData dataWithContentsOfFile:self.requestParam.ca];
        if (p12Data) {
            CFDataRef inP12Data = (__bridge CFDataRef)p12Data;
            NSDictionary *options = @{(__bridge id)kSecImportExportPassphrase: self.requestParam.password};
            CFArrayRef items = CFArrayCreate(NULL, 0, 0, NULL);
            OSStatus status = SecPKCS12Import(inP12Data, (__bridge CFDictionaryRef)options, &items);
            
            if (status == errSecSuccess) {
                CFDictionaryRef identityDict = (CFDictionaryRef)CFArrayGetValueAtIndex(items, 0);
                SecIdentityRef identityRef = (SecIdentityRef)CFDictionaryGetValue(identityDict, kSecImportItemIdentity);
                SecCertificateRef certificateRef;
                SecIdentityCopyCertificate(identityRef, &certificateRef);
                NSArray *certificates = @[(__bridge id)certificateRef];
                NSURLCredential *credential = [NSURLCredential 
                                               credentialWithIdentity:identityRef 
                                               certificates:certificates 
                                               persistence:NSURLCredentialPersistencePermanent];
                completionHandler(NSURLSessionAuthChallengeUseCredential, credential);
                
                CFRelease(certificateRef);
            } else {
                completionHandler(NSURLSessionAuthChallengeCancelAuthenticationChallenge, nil);
            }
            CFRelease(items);
        } else {
            completionHandler(NSURLSessionAuthChallengeCancelAuthenticationChallenge, nil);
        }     
    } else {
        completionHandler(NSURLSessionAuthChallengePerformDefaultHandling, nil);
    }
}

#pragma mark - setter

- (void)setResponseBlock:(HTTPRequestSuccessBlock)responseBlock
{
    _responseBlock = responseBlock;
}

- (void)setFailBlock:(HTTPRequestFailureBlock)failBlock
{
    _failBlock = failBlock;
}

- (void)setUploadProgress:(HTTPRequestProgressBlock)uploadProgress
{
    _uploadProgress = uploadProgress;
}

- (void)setDownloadProgress:(HTTPRequestProgressBlock)downloadProgress
{
    _downloadProgress = downloadProgress;
}

- (void)setMemoryBodyBlock:(HTTPRequestMemoryBodyBlock)memoryBodyBlock
{
    _memoryBodyBlock = memoryBodyBlock;
}

- (void)setMemoryHeaderBlock:(HTTPRequestMemoryHeaderBlock)memoryHeaderBlock
{
    _memoryHeaderBlock = memoryHeaderBlock;
}

- (void)setRedirectionBlock:(HTTPRequestWillRedirectionBlock)redirectionBlock
{
    _redirectionBlock = redirectionBlock;
}

#pragma mark - private

- (void)serializingQueryParams
{
    if (!self.requestParam.bodyParam) {
        return;
    }

    NSDictionary* parameters = nil;
    if ([self.requestParam.bodyParam isKindOfClass:[NSDictionary class]]) {
        parameters = self.requestParam.bodyParam;
    } else {
        parameters = [self convertDictionaryWithJSONString:self.requestParam.bodyParam];
    }

    NSString* query = nil;
    if (parameters) {
        NSMutableArray* mutablePairs = [NSMutableArray array];
        for (QueryStringPair* pair in queryStringPairsFromDictionary(parameters)) {
            [mutablePairs addObject:[pair URLEncodedStringValue]];
        }
        query = [mutablePairs componentsJoinedByString:@"&"];
    }

    NSSet<NSString*>* methodSet = [NSSet setWithObjects:@"GET", @"HEAD", @"DELETE", nil];
    if ([methodSet containsObject:[self.requestParam.method uppercaseString]]) {
        if (query && query.length > 0) {
            NSString* absoluteString = [self.request.URL absoluteString];
            NSString* string = [absoluteString stringByAppendingFormat:self.request.URL.query ?@"&%@":@"?%@",query];
            self.request.URL = [NSURL URLWithString:string];
        }
    } else {
        NSString * contentType = [self.request valueForHTTPHeaderField:@"Content-Type"];
        //default application/json
        if (contentType.length == 0) {
            contentType = @"application/json";
            [self.request setValue:@"application/json" forHTTPHeaderField:@"Content-Type"];
        }
        NSData * bodyData = queryDataFromParameters(contentType, query, parameters);
        if (!bodyData && [self.requestParam.bodyParam isKindOfClass:[NSString class]]) {
            bodyData = [self.requestParam.bodyParam dataUsingEncoding:NSUTF8StringEncoding];
        }
        if (bodyData) {
            [self.request setHTTPBody:bodyData];
        }
    }
}

NSData * queryDataFromParameters(NSString* contentType, NSString* query,NSDictionary* parameters)
{
    NSData *bodyData = nil;
    if ([contentType isEqualToString:@"application/x-plist"]) {
        if (parameters) {
            NSError *error = nil;
            bodyData = [NSPropertyListSerialization dataWithPropertyList:parameters
                            format:NSPropertyListXMLFormat_v1_0 options:0 error:&error];
            if (error) {
                NSLog(@"JSONSerialization fail：%@", error.localizedDescription);
            }
        }
    } else if ([contentType isEqualToString:@"application/x-www-form-urlencoded"]) {
        if (query) {
            bodyData = [query dataUsingEncoding:NSUTF8StringEncoding];
        }
    }else {
        if (parameters) {
            NSError *error = nil;
            bodyData = [NSJSONSerialization dataWithJSONObject:parameters options:0 error:&error];
            if (error) {
                NSLog(@"JSONSerialization fail：%@", error.localizedDescription);
            }
        }
    }
    return bodyData;
}

NSString* queryStringFromParameters(NSDictionary* parameters)
{
    NSMutableArray* mutablePairs = [NSMutableArray array];
    for (QueryStringPair* pair in queryStringPairsFromDictionary(parameters)) {
        [mutablePairs addObject:[pair URLEncodedStringValue]];
    }

    return [mutablePairs componentsJoinedByString:@"&"];
}

NSArray* queryStringPairsFromDictionary(NSDictionary* dictionary)
{
    return queryStringPairsFromKeyAndValue(nil, dictionary);
}

NSArray* queryStringPairsFromKeyAndValue(NSString* key, id value)
{
    NSMutableArray* mutableQueryStringComponents = [NSMutableArray array];

    NSSortDescriptor* sortDescriptor =
    [NSSortDescriptor sortDescriptorWithKey:@"description" ascending:YES selector:@selector(compare:)];

    if ([value isKindOfClass:[NSDictionary class]]) {
        NSDictionary* dictionary = value;
        for (id nestedKey in [dictionary.allKeys sortedArrayUsingDescriptors:@[sortDescriptor]]) {
            id nestedValue = dictionary[nestedKey];
            if (nestedValue) {
                NSString* nestedString = [NSString stringWithFormat:@"%@[%@]", key, nestedKey];
                NSArray* queryArray = queryStringPairsFromKeyAndValue((key ? nestedString : nestedKey), nestedValue);
                [mutableQueryStringComponents addObjectsFromArray: queryArray];
            }
        }
    } else if ([value isKindOfClass:[NSArray class]]) {
        NSArray* array = value;
        for (id nestedValue in array) {
            NSArray* qArray = queryStringPairsFromKeyAndValue([NSString stringWithFormat:@"%@[]", key], nestedValue);
            [mutableQueryStringComponents addObjectsFromArray:qArray];
        }
    } else if ([value isKindOfClass:[NSSet class]]) {
        NSSet* set = value;
        for (id obj in [set sortedArrayUsingDescriptors:@[sortDescriptor]]) {
            [mutableQueryStringComponents addObjectsFromArray:queryStringPairsFromKeyAndValue(key, obj)];
        }
    } else {
        [mutableQueryStringComponents addObject:[[QueryStringPair alloc] initWithField:key value:value]];
    }

    return mutableQueryStringComponents;
}

- (NSDictionary*)convertDictionaryWithJSONString:(NSString*)jsonString
{
    if (!jsonString || jsonString.length == 0) {
        return nil;
    }
    NSData* jsonData = [jsonString dataUsingEncoding:NSUTF8StringEncoding];
    NSError* err;
    NSDictionary* dic = [NSJSONSerialization JSONObjectWithData:jsonData
                                                        options:NSJSONReadingMutableContainers
                                                        error:&err];

    if (err) {
        NSLog(@"%@", err);
        return nil;
    }
    return dic;
}

- (NSDictionary*)setproxyId:(NSString*)proxyId proxyPort:(NSInteger)proxyPort
{
    NSDictionary* proxyDic = [[NSDictionary alloc] init];
    proxyDic = @{@"HTTPSEnable": @YES,
                @"HTTPSProxy": proxyId,
                @"HTTPSPort": [NSString stringWithFormat:@"%ld", (long)proxyPort],
                @"HTTPEnable": @YES,
                @"HTTPProxy": proxyId,
                @"HTTPPort": [NSString stringWithFormat:@"%ld", (long)proxyPort]};
    return proxyDic;
}

- (void)cleanTasks
{
    [self.urlSession getTasksWithCompletionHandler:^(NSArray* dataTasks,
                                                    NSArray* uploadTasks,
                                                    NSArray* downloadTasks) {
        NSArray<NSURLSessionTask*>* tasks;
        if (dataTasks.count != 0) {
            tasks = dataTasks;
        }
        if (dataTasks.count != 0) {
            tasks = uploadTasks;
        }
        if (dataTasks.count != 0) {
            tasks = downloadTasks;
        }
        for (NSURLSessionTask* task in tasks) {
            [task cancel];
        }
    }];
}

- (void)deInitialize
{
    _responseBlock = nil;
    _memoryBodyBlock = nil;
    _memoryHeaderBlock = nil;
    _uploadProgress = nil;
    _downloadProgress = nil;
    _failBlock = nil;

    if (self.urlSession) {
        [self cleanTasks];
        [self.urlSession invalidateAndCancel];
        self.urlSession = nil;
    }

    self.mutableData = nil;
}

- (void)dealloc
{
    [self deInitialize];
#if __has_feature(objc_mrc)
    [super dealloc];
#endif
}

+ (BOOL)isExistFileForPath:(NSString*)strFilePath
{
    if (strFilePath.length < 1) {
        return NO;
    }
    NSFileManager *fileMgr = [NSFileManager defaultManager];
    BOOL bDir = NO;
    BOOL bExist = [fileMgr fileExistsAtPath:strFilePath isDirectory:&bDir];
    if (!bDir && bExist) {
        return  YES;
    }
    return NO;
}

+ (BOOL)isExistDirectoryForPath:(NSString*)strDirPath
{
    if (strDirPath.length < 1) {
        return NO;
    }
    NSFileManager* fileMgr = [NSFileManager defaultManager];
    BOOL bDir = NO;
    BOOL bExist = [fileMgr fileExistsAtPath:strDirPath isDirectory:&bDir];
    if (bDir && bExist) {
        return YES;
    }
    return NO;
}

+ (BOOL)createDirectoryForPath:(NSString*)strDirPath
{
    if (strDirPath.length < 1) {
        return NO;
    }
    if ([self isExistDirectoryForPath:strDirPath]) {
        return YES;
    }
    NSFileManager* fileMgr = [NSFileManager defaultManager];
    BOOL result = [fileMgr createDirectoryAtPath:strDirPath withIntermediateDirectories:YES attributes:nil error:nil];
    return result;
}

@end
