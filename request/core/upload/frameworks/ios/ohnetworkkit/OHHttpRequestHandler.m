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

#import "OHHttpRequestHandler.h"
#import "OHMultiFormData.h"
#import <MobileCoreServices/MobileCoreServices.h>


FOUNDATION_EXPORT NSArray * OHQueryStringPairsFromDictionary(NSDictionary *dictionary);
FOUNDATION_EXPORT NSArray * OHQueryStringPairsFromKeyAndValue(NSString *key, id value);

NSString * OHQueryStringFromParameters(NSDictionary *parameters) {
    NSMutableArray *mutablePairs = [NSMutableArray array];
    for (OHQueryStringPair *pair in OHQueryStringPairsFromDictionary(parameters)) {
        [mutablePairs addObject:[pair URLEncodedStringValue]];
    }
    return [mutablePairs componentsJoinedByString:@"&"];
}

NSArray * OHQueryStringPairsFromDictionary(NSDictionary *dictionary) {
    return OHQueryStringPairsFromKeyAndValue(nil, dictionary);
}

NSArray * OHQueryStringPairsFromKeyAndValue(NSString *key, id value) {
    NSMutableArray *mutableQueryStringComponents = [NSMutableArray array];
    NSSortDescriptor *sortDescriptor = [NSSortDescriptor sortDescriptorWithKey:@"description"
        ascending:YES selector:@selector(compare:)];

    if ([value isKindOfClass:[NSDictionary class]]) {
        NSDictionary *dictionary = value;
        for (id nestedKey in [dictionary.allKeys sortedArrayUsingDescriptors:@[ sortDescriptor ]]) {
            id nestedValue = dictionary[nestedKey];
            if (nestedValue) {
                [mutableQueryStringComponents addObjectsFromArray:OHQueryStringPairsFromKeyAndValue(
                    (key ? [NSString stringWithFormat:@"%@[%@]", key, nestedKey] : nestedKey), nestedValue)];
            }
        }
    } else if ([value isKindOfClass:[NSArray class]]) {
        NSArray *array = value;
        for (id nestedValue in array) {
            [mutableQueryStringComponents addObjectsFromArray:
                OHQueryStringPairsFromKeyAndValue([NSString stringWithFormat:@"%@[]", key], nestedValue)];
        }
    } else if ([value isKindOfClass:[NSSet class]]) {
        NSSet *set = value;
        for (id obj in [set sortedArrayUsingDescriptors:@[ sortDescriptor ]]) {
            [mutableQueryStringComponents addObjectsFromArray:OHQueryStringPairsFromKeyAndValue(key, obj)];
        }
    } else {
        [mutableQueryStringComponents addObject:[[OHQueryStringPair alloc] initWithField:key value:value]];
    }
    return mutableQueryStringComponents;
}

typedef NSString * (^OHQueryStringSerializationBlock)(NSURLRequest *request, id parameters,
    NSError *__autoreleasing *error);

static NSArray * OHHTTPRequestSerializerObservedKeyPaths() {
    static NSArray *_OHHTTPRequestSerializerObservedKeyPaths = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _OHHTTPRequestSerializerObservedKeyPaths = @[
            NSStringFromSelector(@selector(HTTPShouldHandleCookies)),
            NSStringFromSelector(@selector(HTTPShouldUsePipelining)),
            NSStringFromSelector(@selector(networkServiceType)), NSStringFromSelector(@selector(timeoutInterval))];
    });

    return _OHHTTPRequestSerializerObservedKeyPaths;
}

static void *OHHTTPRequestSerializerObserverContext = &OHHTTPRequestSerializerObserverContext;

@interface OHHttpRequestHandler ()

@property (readwrite, nonatomic, strong) NSMutableSet *mutableObservedChangedKeyPaths;
@property (readwrite, nonatomic, strong) NSMutableDictionary *mutableHTTPRequestHeaders;
@property (readwrite, nonatomic, strong) dispatch_queue_t requestHeaderModificationQueue;
@property (readwrite, nonatomic, assign) OHHTTPRequestQueryStringSerializationStyle queryStringSerializationStyle;
@property (readwrite, nonatomic, copy) OHQueryStringSerializationBlock queryStringSerialization;

@end

@implementation OHHttpRequestHandler

+ (instancetype)instance {
    return [[self alloc] init];
}

- (instancetype)init {
    self = [super init];
    if (!self) {
        return nil;
    }

    self.encoding = NSUTF8StringEncoding;
    self.mutableHTTPRequestHeaders = [NSMutableDictionary dictionary];
    self.requestHeaderModificationQueue = dispatch_queue_create("requestHeaderModificationQueue",
        DISPATCH_QUEUE_CONCURRENT);

    NSMutableArray *acceptLanguagesComponents = [NSMutableArray array];
    [[NSLocale preferredLanguages] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        float q = 1.0f - (idx * 0.1f);
        [acceptLanguagesComponents addObject:[NSString stringWithFormat:@"%@;q=%0.1g", obj, q]];
        *stop = q <= 0.5f;
    }];
    [self setValue:[acceptLanguagesComponents componentsJoinedByString:@", "] forHeaderField:@"Accept-Language"];

    NSString *userAgent = nil;
    userAgent = [NSString stringWithFormat:@"%@/%@ (%@; iOS %@; Scale/%0.2f)",
        [[NSBundle mainBundle] infoDictionary][(__bridge NSString *)kCFBundleExecutableKey] ?:
            [[NSBundle mainBundle] infoDictionary][(__bridge NSString *)kCFBundleIdentifierKey],
            [[NSBundle mainBundle] infoDictionary][@"CFBundleShortVersionString"] ?:
            [[NSBundle mainBundle] infoDictionary][(__bridge NSString *)kCFBundleVersionKey],
            [[UIDevice currentDevice] model], [[UIDevice currentDevice] systemVersion],
            [[UIScreen mainScreen] scale]];
    if (userAgent) {
        if (![userAgent canBeConvertedToEncoding:NSASCIIStringEncoding]) {
            NSMutableString *mutableUserAgent = [userAgent mutableCopy];
            if (CFStringTransform((__bridge CFMutableStringRef)(mutableUserAgent), NULL,
                (__bridge CFStringRef)@"Any-Latin; Latin-ASCII; [:^ASCII:] Remove", false)) {
                userAgent = mutableUserAgent;
            }
        }
        [self setValue:userAgent forHeaderField:@"User-Agent"];
    }

    self.httpMethodEncodeParamInUri = [NSSet setWithObjects:@"GET", @"HEAD", @"DELETE", nil];
    self.mutableObservedChangedKeyPaths = [NSMutableSet set];
    for (NSString *keyPath in OHHTTPRequestSerializerObservedKeyPaths()) {
        if ([self respondsToSelector:NSSelectorFromString(keyPath)]) {
            [self addObserver:self forKeyPath:keyPath options:NSKeyValueObservingOptionNew
                context:OHHTTPRequestSerializerObserverContext];
        }
    }

    return self;
}

- (void)dealloc {
    for (NSString *keyPath in OHHTTPRequestSerializerObservedKeyPaths()) {
        if ([self respondsToSelector:NSSelectorFromString(keyPath)]) {
            [self removeObserver:self forKeyPath:keyPath context:OHHTTPRequestSerializerObserverContext];
        }
    }
}

#pragma mark -
- (NSDictionary *)requestHeaders {
    NSDictionary __block *value;
    dispatch_sync(self.requestHeaderModificationQueue, ^{
        value = [NSDictionary dictionaryWithDictionary:self.mutableHTTPRequestHeaders];
    });
    return value;
}

- (void)setValue:(NSString *)value
forHeaderField:(NSString *)field {
    dispatch_barrier_sync(self.requestHeaderModificationQueue, ^{
        [self.mutableHTTPRequestHeaders setValue:value forKey:field];
    });
}

- (NSString *)valueForHeaderField:(NSString *)field {
    NSString __block *value;
    dispatch_sync(self.requestHeaderModificationQueue, ^{
        value = [self.mutableHTTPRequestHeaders valueForKey:field];
    });
    return value;
}

- (void)clearAuthHeader {
    dispatch_barrier_sync(self.requestHeaderModificationQueue, ^{
        [self.mutableHTTPRequestHeaders removeObjectForKey:@"Authorization"];
    });
}

#pragma mark -
- (void)setQryStrSeriWithStyle:(OHHTTPRequestQueryStringSerializationStyle)style {
    self.queryStringSerializationStyle = style;
    self.queryStringSerialization = nil;
}

- (void)setQryStrSeriWithBlock:(NSString *(^)(NSURLRequest *, id, NSError *__autoreleasing *))block {
    self.queryStringSerialization = block;
}

#pragma mark -
- (NSMutableURLRequest *)requestWithMethod:(NSString *)method
                                 urlString:(NSString *)urlString
                                parameters:(id)parameters
                                     error:(NSError *__autoreleasing *)error {
    NSParameterAssert(method);
    NSParameterAssert(urlString);
    NSURL *url = [NSURL URLWithString:urlString];
    NSParameterAssert(url);
    NSMutableURLRequest *mutableRequest = [[NSMutableURLRequest alloc] initWithURL:url];
    mutableRequest.HTTPMethod = method;

    for (NSString *keyPath in self.mutableObservedChangedKeyPaths) {
        [mutableRequest setValue:[self valueForKeyPath:keyPath] forKey:keyPath];
    }
    mutableRequest = [[self requestBySeriReq:mutableRequest withParameters:parameters error:error]
        mutableCopy];
	return mutableRequest;
}

- (NSMutableURLRequest *)multipartFormRequestWithMethod:(NSString *)method
                                              urlString:(NSString *)urlString
                                             parameters:(NSDictionary *)parameters
                              constructingBodyWithBlock:(void (^)(id <OHMultiFormData> formData))block
                                                  error:(NSError *__autoreleasing *)error {
    NSParameterAssert(method);
    NSParameterAssert(![method isEqualToString:@"GET"] && ![method isEqualToString:@"HEAD"]);
    NSMutableURLRequest *mutableRequest = [self requestWithMethod:method urlString:urlString parameters:nil
        error:error];
    __block OHStreamingMultiFormData *formData = [[OHStreamingMultiFormData alloc]
        initWithUrlReq:mutableRequest encoding:NSUTF8StringEncoding];

    if (parameters) {
        for (OHQueryStringPair *pair in OHQueryStringPairsFromDictionary(parameters)) {
            NSData *data = nil;
            if ([pair.value isKindOfClass:[NSData class]]) {
                data = pair.value;
            } else if ([pair.value isEqual:[NSNull null]]) {
                data = [NSData data];
            } else {
                data = [[pair.value description] dataUsingEncoding:self.encoding];
            }

            if (data) {
                [formData addPartWithFormData:data name:[pair.field description]];
            }
        }
    }

    if (block) {
        block(formData);
    }

    return [formData requestByFinMultiFormData];
}

- (NSURLRequest *)requestBySeriReq:(NSURLRequest *)request
                               withParameters:(id)parameters
                                        error:(NSError *__autoreleasing *)error {
    NSParameterAssert(request);
    NSMutableURLRequest *mutableRequest = [request mutableCopy];
    [self.requestHeaders enumerateKeysAndObjectsUsingBlock:^(id field, id value, BOOL * __unused stop) {
        if (![request valueForHeaderField:field]) {
            [mutableRequest setValue:value forHeaderField:field];
        }
    }];

    NSString *query = nil;
    if (parameters) {
        if (self.queryStringSerialization) {
            NSError *serializationError;
            query = self.queryStringSerialization(request, parameters, &serializationError);
            if (serializationError) {
                if (error) {
                    *error = serializationError;
                }
                return nil;
            }
        } else {
            switch (self.queryStringSerializationStyle) {
                case OHHTTPRequestQueryStringDefaultStyle:
                    query = OHQueryStringFromParameters(parameters);
                    break;
            }
        }
    }

    if ([self.httpMethodEncodeParamInUri containsObject:[[request HTTPMethod] uppercaseString]]) {
        if (query && query.length > 0) {
            mutableRequest.URL = [NSURL URLWithString:[[mutableRequest.URL absoluteString]
                stringByAppendingFormat:mutableRequest.URL.query ? @"&%@" : @"?%@", query]];
        }
    } else {
        if (!query) {
            query = @"";
        }
        if (![mutableRequest valueForHeaderField:@"Content-Type"]) {
            [mutableRequest setValue:@"application/x-www-form-urlencoded" forHeaderField:@"Content-Type"];
        }
        [mutableRequest setHTTPBody:[query dataUsingEncoding:self.encoding]];
    }
    return mutableRequest;
}

#pragma mark - NSKeyValueObserving
+ (BOOL)automaticallyNotifiesObserversForKey:(NSString *)key {
    if ([OHHTTPRequestSerializerObservedKeyPaths() containsObject:key]) {
        return NO;
    }
    return [super automaticallyNotifiesObserversForKey:key];
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(__unused id)object
                        change:(NSDictionary *)change
                       context:(void *)context {
    if (context == OHHTTPRequestSerializerObserverContext) {
        if ([change[NSKeyValueChangeNewKey] isEqual:[NSNull null]]) {
            [self.mutableObservedChangedKeyPaths removeObject:keyPath];
        } else {
            [self.mutableObservedChangedKeyPaths addObject:keyPath];
        }
    }
}

@end