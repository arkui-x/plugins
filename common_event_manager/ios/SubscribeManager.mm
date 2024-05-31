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

#import "SubscribeManager.h"

#include <string>
#include <vector>
#include "inner_api/plugin_utils_inner.h"
#include "inner_api/plugin_utils_napi.h"

#import "Subscriber.h"

@interface SubscribeManager ()

@property (nonatomic, strong) NSMutableDictionary<NSString*, Subscriber*>* subscriberMap;
@end

@implementation SubscribeManager
+ (instancetype)shareManager {
    static SubscribeManager* _manager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _manager = [[SubscribeManager alloc] init];
    });
    return _manager;
}

- (BOOL)registeSubscriber:(id)key events:(id)events {
    NSLog(@"%s, key : %@, events : %@", __func__, (NSString*)key, (NSMutableArray*)events);
    NSString* keyString = (NSString*)key;
    NSArray* eventsArray = [(NSMutableArray*)events copy];

    if (!keyString.length || !eventsArray.count) {
        NSLog(@"registe subscriber failed, key : %@, events : %@", keyString, eventsArray);
        return NO;
    }

    Subscriber* subscriber = [self getSubscriber:keyString];
    if (!subscriber) {
        subscriber = [[Subscriber alloc] init];
    }
    [subscriber subscribeEvents:eventsArray key:keyString];
    
    @synchronized (self) {
        [self.subscriberMap setObject:subscriber forKey:keyString];
    }
    return YES;
}

- (BOOL)removeSubscriber:(id)key callbackInfo:(void*)ptr {
    NSLog(@"%s, key : %@", __func__, (NSString*)key);
    NSString* keyString = (NSString*)key;

    if (!keyString.length) {
        NSLog(@"removeSubscriber fail, key is nil");
        return NO;
    }

    Subscriber* subscriber = [self getSubscriber:keyString];
    if (!subscriber) {
        return YES;
    }
    [subscriber unSubscribeEvents:ptr];

    @synchronized (self) {
        [self.subscriberMap removeObjectForKey:keyString];
    }

    return YES;
}

- (Subscriber*)getSubscriber:(NSString*)key {
    @synchronized(self) {
        return [self.subscriberMap objectForKey:key];
    }
}

- (NSMutableDictionary<NSString*, Subscriber*>*)subscriberMap {
    if (!_subscriberMap) {
        @synchronized(self) {
            _subscriberMap = [[NSMutableDictionary alloc] init];
        }
    }
    return _subscriberMap;
}
@end
