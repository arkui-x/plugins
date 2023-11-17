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

#import "OHMultipartFormStream.h"

#define OHDefaultBundary [NSString stringWithFormat:@"Boundary-%@", [[NSUUID UUID] UUIDString]]

typedef enum {
    OHStepBoundaryBegin    = 1,
    OHStepFields           = 2,
    OHStepBody             = 3,
    OHStepBoundaryFinal    = 4,
    OHStepEnd              = 5,
} OHSubFormPartReadStep;

@interface OHSubFormPart : NSObject

@property (nonatomic, assign) unsigned long long bodyContentLength;
@property (nonatomic, assign) unsigned long long contentLength;

@property (nonatomic, strong) NSInputStream *innerStream;

@property (nonatomic, strong) NSDictionary *parameters;
@property (nonatomic, copy) NSString *boundary;

@property (nonatomic, assign) OHSubFormPartReadStep currentReadStep;
@property (nonatomic, assign) unsigned long long currentStepReadOffset;

//for join multipart formData
@property (nonatomic, assign) BOOL isFirstPart;
@property (nonatomic, assign) BOOL isLastPart;

@end

@implementation OHSubFormPart

- (instancetype)init {
    self = [super init];
    if (self) {
        _isFirstPart = false;
        _isLastPart = false;
        _currentReadStep = OHStepBoundaryBegin;
    }
    return self;
}

- (void)dealloc {
    if (_innerStream) {
        [_innerStream close];
        _innerStream = nil;
    }
}

- (unsigned long long)contentLength {
    unsigned long long length = 0;

    NSData *beginData = [self dataForBoundaryBegin];
    length += [beginData length];

    NSData *fieldsData = [[self stringFormFields] dataUsingEncoding:NSUTF8StringEncoding];
    length += [fieldsData length];

    length += _bodyContentLength;

    NSData *finalData = [self dataForBoundaryFinal];
    length += [finalData length];

    return length;
}

- (NSData *)dataForBoundaryBegin {
    NSString *boundaryBeginStr = _isFirstPart ? [NSString stringWithFormat:@"--%@\r\n", _boundary] : [NSString stringWithFormat:@"\r\n--%@\r\n", _boundary];
    return [boundaryBeginStr dataUsingEncoding:NSUTF8StringEncoding];
}

- (NSData *)dataForBoundaryFinal {
    NSData *data = [NSData data];
    if (_isLastPart) {
        data = [[NSString stringWithFormat:@"\r\n--%@--\r\n", _boundary] dataUsingEncoding:NSUTF8StringEncoding];
    }
    return data;
}

- (BOOL)hasBytesAvailable {
    
    if (_currentReadStep <= OHStepBoundaryFinal) {
        return true;
    }
    
    if (_innerStream.streamStatus <= NSStreamStatusWriting) {
        return true;
    }
    
    return false;
}

- (NSString *)stringFormFields {
    NSMutableString *filedString = [NSMutableString string];
    
    [_parameters enumerateKeysAndObjectsUsingBlock:^(NSString *key, NSString *value, BOOL *stop) {
        [filedString appendString:[NSString stringWithFormat:@"%@: %@\r\n", key, value]];
    }];
    [filedString appendString:@"\r\n"];

    return [NSString stringWithString:filedString];
}

- (NSInteger)read:(uint8_t *)buffer maxLength:(NSUInteger)len {
    
    NSInteger totalReadLength = 0;

    if (_currentReadStep == OHStepBoundaryBegin) {
        
        NSData *readData = [self dataForBoundaryBegin] ;
        totalReadLength += [self readFromData:readData toBuffer:&buffer[totalReadLength] maxLength:(len - (NSUInteger)totalReadLength)];
    }

    if (_currentReadStep == OHStepFields) {
        
        NSData *readData = [[self stringFormFields] dataUsingEncoding:NSUTF8StringEncoding];
        totalReadLength += [self readFromData:readData toBuffer:&buffer[totalReadLength] maxLength:(len - (NSUInteger)totalReadLength)];
    }

    if (_currentReadStep == OHStepBody) {
        NSInteger streamReadLength = 0;

        streamReadLength = [_innerStream read:&buffer[totalReadLength] maxLength:(len - (NSUInteger)totalReadLength)];
        
        if (streamReadLength == -1) {
            return -1;
        } else {
            totalReadLength += streamReadLength;
            
            _currentStepReadOffset += streamReadLength;

            if ([_innerStream streamStatus] >= NSStreamStatusAtEnd) {
                [self transitionToNextStep];
            }
        }
    }

    if (_currentReadStep == OHStepBoundaryFinal) {
        
        NSData *readData = [self dataForBoundaryFinal];
        totalReadLength += [self readFromData:readData toBuffer:&buffer[totalReadLength] maxLength:(len - (NSUInteger)totalReadLength)];
    }

    return totalReadLength;
}

- (NSInteger)readFromData:(NSData *)data
                 toBuffer:(uint8_t *)buffer
                maxLength:(NSUInteger)length {
    
    NSUInteger totalRemainLength = [data length] - ((NSUInteger)_currentStepReadOffset);
    NSUInteger expectedLength = MIN(totalRemainLength, length);
    NSRange range = NSMakeRange((NSUInteger)_currentStepReadOffset, expectedLength);
    [data getBytes:buffer range:range];

    _currentStepReadOffset += range.length;
    
    if (((NSUInteger)_currentStepReadOffset) >= [data length]) {
        [self transitionToNextStep];
    }

    return (NSInteger)range.length;
}

- (void)transitionToNextStep {
    if ([[NSThread currentThread] isMainThread] == false) {
        dispatch_sync(dispatch_get_main_queue(), ^{
            [self transitionToNextStep];
        });
        return;
    }

    switch (_currentReadStep) {
        case OHStepBoundaryBegin:
            _currentReadStep = OHStepFields;
            break;
        case OHStepFields:
            [_innerStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
            [_innerStream open];
            _currentReadStep = OHStepBody;
            break;
        case OHStepBody:
            [_innerStream close];
            _currentReadStep = OHStepBoundaryFinal;
            break;
        case OHStepBoundaryFinal:
            _currentReadStep = OHStepEnd;
            break;
        case OHStepEnd:
            break;
        default:
            _currentReadStep = OHStepBoundaryBegin;
            break;
    }
    _currentStepReadOffset = 0;

}

@end


//to make status write
@interface NSStream ()
@property (readwrite, assign) NSStreamStatus streamStatus;
@property (readwrite, copy) NSError *streamError;
@end

@interface OHMultipartFormStream ()

@property (nonatomic, strong) NSMutableArray<OHSubFormPart *> *subStreams;
@property (nonatomic, copy) NSString *boundary;
@property (nonatomic, strong) OHSubFormPart *currentPart;
@property (nonatomic, strong) NSEnumerator *partsEnumerator;

@end

@implementation OHMultipartFormStream

//for NSStream need
@synthesize delegate;

@synthesize streamStatus;
@synthesize streamError;

+ (NSMutableURLRequest *)requestWithURL:(NSURL *)url
                                 method:(NSString *)method
                             parameters:(NSDictionary *)parameters
                    multipartFormStream:(void (^)(OHMultipartFormStream *multipartStream))multipartFormStream  {
    
    NSMutableURLRequest *request = [[NSMutableURLRequest alloc] initWithURL:url];
    request.HTTPMethod = method;
    [request setAllHTTPHeaderFields: parameters];
    
    NSString *boundary = OHDefaultBundary;
    
    OHMultipartFormStream *formStream = [[OHMultipartFormStream alloc] init];
    formStream.boundary = boundary;
    
    request.HTTPBodyStream = formStream;
    
    multipartFormStream(formStream);
    [formStream initializeStreams];
    
    [request setValue:[NSString stringWithFormat:@"multipart/form-data; boundary=%@", boundary] forHTTPHeaderField:@"Content-Type"];
    [request setValue:[NSString stringWithFormat:@"%llu", [formStream contentLength]] forHTTPHeaderField:@"Content-Length"];
    
    return request;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _subStreams = [[NSMutableArray alloc] init];
    }
    return self;
}

- (void)initializeStreams {
    if (!(_subStreams > 0)) { return; }
    
    [_subStreams firstObject].isFirstPart = true;
    [_subStreams lastObject].isLastPart = true;
}

- (unsigned long long)contentLength {
    unsigned long long length = 0;
    for (OHSubFormPart *part in _subStreams) {
        length += [part contentLength];
    }
    return length;
}


- (NSInteger)read:(uint8_t *)buffer maxLength:(NSUInteger)len {
    
    if ([self streamStatus] == NSStreamStatusClosed) { return 0; }
    
    NSInteger totalExpectedReadLenght = len;
    NSInteger totalReadLength = 0;
    while (totalReadLength < totalExpectedReadLenght) {
        
        BOOL isCurrentPartEmpty = _currentPart == nil || [_currentPart hasBytesAvailable] == false;
        
        if (isCurrentPartEmpty) {
            _currentPart = [_partsEnumerator nextObject];
            if(!_currentPart) {
                break;
            }
        } else {
            NSInteger expectedLength = totalExpectedReadLenght - totalReadLength;
            NSInteger readLength = [_currentPart read:&buffer[totalReadLength] maxLength:expectedLength];
            
            if (readLength == -1) {
                self.streamError = _currentPart.innerStream.streamError;
                break;
            }
            totalReadLength += readLength;
        }
    }

    return totalReadLength;
}

- (BOOL)getBuffer:(uint8_t * _Nullable *)buffer length:(NSUInteger *)len {
    return false;
}


- (void)appendWithFilePath:(NSURL *)filePath
                  fileName:(NSString *)fileName
                 fieldName:(NSString *)fieldName
                  mimeType:(NSString *)mimeType  {
    
    NSError *error;
    
    NSDictionary *fileAttributes = [[NSFileManager defaultManager] attributesOfItemAtPath:filePath.path error:&error];
    if (!fileAttributes) {
        return ;
    }
    
    NSMutableDictionary *paramters = [NSMutableDictionary dictionary];
    [paramters setValue:[NSString stringWithFormat:@"form-data; name=\"%@\"; filename=\"%@\"", fieldName, fileName] forKey:@"Content-Disposition"];
    [paramters setValue:mimeType forKey:@"Content-Type"];
    
    
    OHSubFormPart *part = [[OHSubFormPart alloc] init];
    part.boundary = _boundary;
    part.parameters = paramters;
    part.bodyContentLength = [fileAttributes[NSFileSize] unsignedLongLongValue];
    part.innerStream = [NSInputStream inputStreamWithURL:filePath];
    
    [_subStreams addObject:part];
}

- (void)appendWithFormData:(NSData *)data name:(NSString *)name {
    
    NSMutableDictionary *paramters = [NSMutableDictionary dictionary];
    [paramters setValue:[NSString stringWithFormat:@"form-data; name=\"%@\"", name] forKey:@"Content-Disposition"];
    
    OHSubFormPart *part = [[OHSubFormPart alloc] init];
    part.boundary = _boundary;
    part.parameters = paramters;
    part.bodyContentLength = [data length];
    part.innerStream = [NSInputStream inputStreamWithData:data];
    
    [_subStreams addObject:part];
}

#pragma mark - NSStream

- (BOOL)hasBytesAvailable {
    return [self streamStatus] == NSStreamStatusOpen;
}

- (void)open {
    if (self.streamStatus == NSStreamStatusOpen) { return; }

    self.streamStatus = NSStreamStatusOpen;
    [self initializeStreams];
    _partsEnumerator = [_subStreams objectEnumerator];
}

- (void)close {
    self.streamStatus = NSStreamStatusClosed;
}

- (void)scheduleInRunLoop:(NSRunLoop *)aRunLoop forMode:(NSRunLoopMode)mode {}

- (void)removeFromRunLoop:(NSRunLoop *)aRunLoop forMode:(NSRunLoopMode)mode {}


@end

