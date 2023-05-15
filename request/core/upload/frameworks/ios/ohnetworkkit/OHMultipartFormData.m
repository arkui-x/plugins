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

#import "OHMultipartFormData.h"
#import <MobileCoreServices/MobileCoreServices.h>


NSString * const OHURLRequestSerializationErrorDomain = @"com.oh.error.serialization.request";
NSString * const OHNetworkingOperationFailingURLRequestErrorKey = @"com.oh.serialization.request.error.response";

NSString * OHPercentEscapedStringFromString(NSString *string) {
    static NSString * const kOHCharactersGeneralDelimitersToEncode = @":#[]@";
    static NSString * const kOHCharactersSubDelimitersToEncode = @"!$&'()*+,;=";
    NSMutableCharacterSet * allowedCharacterSet = [[NSCharacterSet URLQueryAllowedCharacterSet] mutableCopy];
    [allowedCharacterSet removeCharactersInString:[kOHCharactersGeneralDelimitersToEncode stringByAppendingString:kOHCharactersSubDelimitersToEncode]];

    static NSUInteger const batchSize = 50;
    NSUInteger index = 0;
    NSMutableString *escaped = @"".mutableCopy;
    while (index < string.length) {
        NSUInteger length = MIN(string.length - index, batchSize);
        NSRange range = NSMakeRange(index, length);
        range = [string rangeOfComposedCharacterSequencesForRange:range];
        NSString *substring = [string substringWithRange:range];
        NSString *encoded = [substring stringByAddingPercentEncodingWithAllowedCharacters:allowedCharacterSet];
        [escaped appendString:encoded];
        index += range.length;
    }
    return escaped;
}

static NSString * OHCreateMultipartFormBoundary() {
    return [NSString stringWithFormat:@"Boundary+%08X%08X", arc4random(), arc4random()];
}

static NSString * const kOHMultipartFormCRLF = @"\r\n";

static inline NSString * OHMultipartFormInitialBoundary(NSString *boundary) {
    return [NSString stringWithFormat:@"--%@%@", boundary, kOHMultipartFormCRLF];
}

static inline NSString * OHMultipartFormEncapsulationBoundary(NSString *boundary) {
    return [NSString stringWithFormat:@"%@--%@%@", kOHMultipartFormCRLF, boundary, kOHMultipartFormCRLF];
}

static inline NSString * OHMultipartFormFinalBoundary(NSString *boundary) {
    return [NSString stringWithFormat:@"%@--%@--%@", kOHMultipartFormCRLF, boundary, kOHMultipartFormCRLF];
}

static inline NSString * OHContentTypeForPathExtension(NSString *extension) {
    NSString *UTI = (__bridge_transfer NSString *)UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, (__bridge CFStringRef)extension, NULL);
    NSString *contentType = (__bridge_transfer NSString *)UTTypeCopyPreferredTagWithClass((__bridge CFStringRef)UTI, kUTTagClassMIMEType);
    if (!contentType) {
        return @"application/octet-stream";
    } else {
        return contentType;
    }
}

#pragma mark - OHQueryStringPair
@implementation OHQueryStringPair

- (instancetype)initWithField:(id)field value:(id)value {
    self = [super init];
    if (!self) {
        return nil;
    }

    self.field = field;
    self.value = value;
    return self;
}

- (NSString *)URLEncodedStringValue {
    if (!self.value || [self.value isEqual:[NSNull null]]) {
        return OHPercentEscapedStringFromString([self.field description]);
    } else {
        return [NSString stringWithFormat:@"%@=%@", OHPercentEscapedStringFromString([self.field description]), OHPercentEscapedStringFromString([self.value description])];
    }
}

@end


#pragma mark - OHHttpBodyPart
@interface OHHttpBodyPart : NSObject

@property (nonatomic, assign) NSStringEncoding stringEncoding;
@property (nonatomic, strong) NSDictionary *headers;
@property (nonatomic, copy) NSString *boundary;
@property (nonatomic, strong) id body;
@property (nonatomic, assign) unsigned long long bodyContentLength;
@property (nonatomic, strong) NSInputStream *inputStream;
@property (nonatomic, assign) BOOL hasInitialBoundary;
@property (nonatomic, assign) BOOL hasFinalBoundary;
@property (readonly, nonatomic, assign, getter = hasBytesAvailable) BOOL bytesAvailable;
@property (readonly, nonatomic, assign) unsigned long long contentLength;

- (NSInteger)read:(uint8_t *)buffer
        maxLength:(NSUInteger)length;

@end


#pragma mark - OHMulpartBodyStream
@interface OHMulpartBodyStream : NSInputStream <NSStreamDelegate>

@property (nonatomic, assign) NSUInteger numberOfBytesInPacket;
@property (nonatomic, assign) NSTimeInterval delay;
@property (nonatomic, strong) NSInputStream *inputStream;
@property (readonly, nonatomic, assign) unsigned long long contentLength;
@property (readonly, nonatomic, assign, getter = isEmpty) BOOL empty;

- (instancetype)initWithStringEncoding:(NSStringEncoding)encoding;
- (void)setInitialAndFinalBoundaries;
- (void)appendHTTPBodyPart:(OHHttpBodyPart *)bodyPart;

@end


#pragma mark - OHStreamingMultipartFormData
@interface OHStreamingMultipartFormData ()

@property (readwrite, nonatomic, copy) NSMutableURLRequest *request;
@property (readwrite, nonatomic, assign) NSStringEncoding stringEncoding;
@property (readwrite, nonatomic, copy) NSString *boundary;
@property (readwrite, nonatomic, strong) OHMulpartBodyStream *bodyStream;

@end


@implementation OHStreamingMultipartFormData

- (instancetype)initWithURLRequest:(NSMutableURLRequest *)urlRequest
                    stringEncoding:(NSStringEncoding)encoding {
    self = [super init];
    if (!self) {
        return nil;
    }

    self.request = urlRequest;
    self.stringEncoding = encoding;
    self.boundary = OHCreateMultipartFormBoundary();
    self.bodyStream = [[OHMulpartBodyStream alloc] initWithStringEncoding:encoding];
    return self;
}

- (void)setRequest:(NSMutableURLRequest *)request {
    _request = [request mutableCopy];
}

- (BOOL)appendPartWithFileURL:(NSURL *)fileURL
                         name:(NSString *)name
                     fileName:(NSString *)fileName
                     mimeType:(NSString *)mimeType
                        error:(NSError * __autoreleasing *)error {
    NSParameterAssert(fileURL);
    NSParameterAssert(name);
    NSParameterAssert(fileName);
    NSParameterAssert(mimeType);

    if (![fileURL isFileURL]) {
        NSDictionary *userInfo = @{NSLocalizedFailureReasonErrorKey: NSLocalizedStringFromTable(@"Expected URL to be a file URL", @"OHNetworkKit", nil)};
        if (error) {
            *error = [[NSError alloc] initWithDomain:OHURLRequestSerializationErrorDomain code:NSURLErrorBadURL userInfo:userInfo];
        }
        return NO;
    } else if ([fileURL checkResourceIsReachableAndReturnError:error] == NO) {
        NSDictionary *userInfo = @{NSLocalizedFailureReasonErrorKey: NSLocalizedStringFromTable(@"File URL not reachable.", @"OHNetworkKit", nil)};
        if (error) {
            *error = [[NSError alloc] initWithDomain:OHURLRequestSerializationErrorDomain code:NSURLErrorBadURL userInfo:userInfo];
        }
        return NO;
    }

    NSDictionary *fileAttributes = [[NSFileManager defaultManager] attributesOfItemAtPath:[fileURL path] error:error];
    if (!fileAttributes) {
        return NO;
    }

    NSMutableDictionary *mutableHeaders = [NSMutableDictionary dictionary];
    [mutableHeaders setValue:[NSString stringWithFormat:@"form-data; name=\"%@\"; filename=\"%@\"", name, fileName] forKey:@"Content-Disposition"];
    [mutableHeaders setValue:mimeType forKey:@"Content-Type"];
    OHHttpBodyPart *bodyPart = [[OHHttpBodyPart alloc] init];
    bodyPart.stringEncoding = self.stringEncoding;
    bodyPart.headers = mutableHeaders;
    bodyPart.boundary = self.boundary;
    bodyPart.body = fileURL;
    bodyPart.bodyContentLength = [fileAttributes[NSFileSize] unsignedLongLongValue];
    [self.bodyStream appendHTTPBodyPart:bodyPart];
    return YES;
}

- (void)appendPartWithFormData:(NSData *)data
                          name:(NSString *)name {
    NSParameterAssert(name);
    NSMutableDictionary *mutableHeaders = [NSMutableDictionary dictionary];
    [mutableHeaders setValue:[NSString stringWithFormat:@"form-data; name=\"%@\"", name] forKey:@"Content-Disposition"];
    [self appendPartWithHeaders:mutableHeaders body:data];
}

- (void)appendPartWithHeaders:(NSDictionary *)headers
                         body:(NSData *)body {
    NSParameterAssert(body);
    OHHttpBodyPart *bodyPart = [[OHHttpBodyPart alloc] init];
    bodyPart.stringEncoding = self.stringEncoding;
    bodyPart.headers = headers;
    bodyPart.boundary = self.boundary;
    bodyPart.bodyContentLength = [body length];
    bodyPart.body = body;
    [self.bodyStream appendHTTPBodyPart:bodyPart];
}

- (NSMutableURLRequest *)requestByFinalizingMultipartFormData {
    if ([self.bodyStream isEmpty]) {
        return self.request;
    }

    // Reset the initial and final boundaries to ensure correct Content-Length
    [self.bodyStream setInitialAndFinalBoundaries];
    [self.request setHTTPBodyStream:self.bodyStream];
    [self.request setValue:[NSString stringWithFormat:@"multipart/form-data; boundary=%@", self.boundary] forHTTPHeaderField:@"Content-Type"];
    [self.request setValue:[NSString stringWithFormat:@"%llu", [self.bodyStream contentLength]] forHTTPHeaderField:@"Content-Length"];
    return self.request;
}

@end

#pragma mark - NSStream
@interface NSStream ()

@property (readwrite) NSStreamStatus streamStatus;
@property (readwrite, copy) NSError *streamError;

@end


#pragma mark - OHMulpartBodyStream
@interface OHMulpartBodyStream () <NSCopying>

@property (readwrite, nonatomic, assign) NSStringEncoding stringEncoding;
@property (readwrite, nonatomic, strong) NSMutableArray *HTTPBodyParts;
@property (readwrite, nonatomic, strong) NSEnumerator *HTTPBodyPartEnumerator;
@property (readwrite, nonatomic, strong) OHHttpBodyPart *currentHTTPBodyPart;
@property (readwrite, nonatomic, strong) NSMutableData *buffer;

@end

@implementation OHMulpartBodyStream

#if (defined(__IPHONE_OS_VERSION_MAX_ALLOWED) && __IPHONE_OS_VERSION_MAX_ALLOWED >= 80000)
@synthesize delegate;
#endif
@synthesize streamStatus;
@synthesize streamError;


- (instancetype)initWithStringEncoding:(NSStringEncoding)encoding {
    self = [super init];
    if (!self) {
        return nil;
    }

    self.stringEncoding = encoding;
    self.HTTPBodyParts = [NSMutableArray array];
    self.numberOfBytesInPacket = NSIntegerMax;
    return self;
}

- (void)setInitialAndFinalBoundaries {
    if ([self.HTTPBodyParts count] > 0) {
        for (OHHttpBodyPart *bodyPart in self.HTTPBodyParts) {
            bodyPart.hasInitialBoundary = NO;
            bodyPart.hasFinalBoundary = NO;
        }

        [[self.HTTPBodyParts firstObject] setHasInitialBoundary:YES];
        [[self.HTTPBodyParts lastObject] setHasFinalBoundary:YES];
    }
}

- (void)appendHTTPBodyPart:(OHHttpBodyPart *)bodyPart {
    [self.HTTPBodyParts addObject:bodyPart];
}

- (BOOL)isEmpty {
    return [self.HTTPBodyParts count] == 0;
}

#pragma mark - NSInputStream
- (NSInteger)read:(uint8_t *)buffer
        maxLength:(NSUInteger)length {
    if ([self streamStatus] == NSStreamStatusClosed) {
        return 0;
    }

    NSInteger totalNumberOfBytesRead = 0;
    while ((NSUInteger)totalNumberOfBytesRead < MIN(length, self.numberOfBytesInPacket)) {
        if (!self.currentHTTPBodyPart || ![self.currentHTTPBodyPart hasBytesAvailable]) {
            if (!(self.currentHTTPBodyPart = [self.HTTPBodyPartEnumerator nextObject])) {
                break;
            }
        } else {
            NSUInteger maxLength = MIN(length, self.numberOfBytesInPacket) - (NSUInteger)totalNumberOfBytesRead;
            NSInteger numberOfBytesRead = [self.currentHTTPBodyPart read:&buffer[totalNumberOfBytesRead] maxLength:maxLength];
            if (numberOfBytesRead == -1) {
                self.streamError = self.currentHTTPBodyPart.inputStream.streamError;
                break;
            } else {
                totalNumberOfBytesRead += numberOfBytesRead;
                if (self.delay > 0.0f) {
                    [NSThread sleepForTimeInterval:self.delay];
                }
            }
        }
    }
    return totalNumberOfBytesRead;
}

- (BOOL)getBuffer:(__unused uint8_t **)buffer
           length:(__unused NSUInteger *)len {
    return NO;
}

- (BOOL)hasBytesAvailable {
    return [self streamStatus] == NSStreamStatusOpen;
}

#pragma mark - NSStream
- (void)open {
    if (self.streamStatus == NSStreamStatusOpen) {
        return;
    }
    self.streamStatus = NSStreamStatusOpen;
    [self setInitialAndFinalBoundaries];
    self.HTTPBodyPartEnumerator = [self.HTTPBodyParts objectEnumerator];
}

- (void)close {
    self.streamStatus = NSStreamStatusClosed;
}

- (id)propertyForKey:(__unused NSString *)key {
    return nil;
}

- (BOOL)setProperty:(__unused id)property
             forKey:(__unused NSString *)key {
    return NO;
}

- (void)scheduleInRunLoop:(__unused NSRunLoop *)aRunLoop
                  forMode:(__unused NSString *)mode {
}

- (void)removeFromRunLoop:(__unused NSRunLoop *)aRunLoop
                  forMode:(__unused NSString *)mode {
}

- (unsigned long long)contentLength {
    unsigned long long length = 0;
    for (OHHttpBodyPart *bodyPart in self.HTTPBodyParts) {
        length += [bodyPart contentLength];
    }
    return length;
}

#pragma mark - Undocumented CFReadStream Bridged Methods
- (void)_scheduleInCFRunLoop:(__unused CFRunLoopRef)aRunLoop
                     forMode:(__unused CFStringRef)aMode {
}

- (void)_unscheduleFromCFRunLoop:(__unused CFRunLoopRef)aRunLoop
                         forMode:(__unused CFStringRef)aMode {
}

- (BOOL)_setCFClientFlags:(__unused CFOptionFlags)inFlags
                 callback:(__unused CFReadStreamClientCallBack)inCallback
                  context:(__unused CFStreamClientContext *)inContext {
    return NO;
}

#pragma mark - NSCopying
- (instancetype)copyWithZone:(NSZone *)zone {
    OHMulpartBodyStream *bodyStreamCopy = [[[self class] allocWithZone:zone] initWithStringEncoding:self.stringEncoding];
    for (OHHttpBodyPart *bodyPart in self.HTTPBodyParts) {
        [bodyStreamCopy appendHTTPBodyPart:[bodyPart copy]];
    }
    [bodyStreamCopy setInitialAndFinalBoundaries];
    return bodyStreamCopy;
}

@end


#pragma mark - OHHttpBodyPart
typedef enum {
    OHEncapsulationBoundaryPhase = 1,
    OHHeaderPhase                = 2,
    OHBodyPhase                  = 3,
    OHFinalBoundaryPhase         = 4,
} OHHTTPBodyPartReadPhase;

@interface OHHttpBodyPart () <NSCopying> {
    OHHTTPBodyPartReadPhase _phase;
    NSInputStream *_inputStream;
    unsigned long long _phaseReadOffset;
}

- (BOOL)transitionToNextPhase;
- (NSInteger)readData:(NSData *)data
           intoBuffer:(uint8_t *)buffer
            maxLength:(NSUInteger)length;
@end

@implementation OHHttpBodyPart

- (instancetype)init {
    self = [super init];
    if (!self) {
        return nil;
    }

    [self transitionToNextPhase];
    return self;
}

- (void)dealloc {
    if (_inputStream) {
        [_inputStream close];
        _inputStream = nil;
    }
}

- (NSInputStream *)inputStream {
    if (!_inputStream) {
        if ([self.body isKindOfClass:[NSData class]]) {
            _inputStream = [NSInputStream inputStreamWithData:self.body];
        } else if ([self.body isKindOfClass:[NSURL class]]) {
            _inputStream = [NSInputStream inputStreamWithURL:self.body];
        } else if ([self.body isKindOfClass:[NSInputStream class]]) {
            _inputStream = self.body;
        } else {
            _inputStream = [NSInputStream inputStreamWithData:[NSData data]];
        }
    }
    return _inputStream;
}

- (NSString *)stringForHeaders {
    NSMutableString *headerString = [NSMutableString string];
    for (NSString *field in [self.headers allKeys]) {
        [headerString appendString:[NSString stringWithFormat:@"%@: %@%@", field, [self.headers valueForKey:field], kOHMultipartFormCRLF]];
    }
    [headerString appendString:kOHMultipartFormCRLF];
    return [NSString stringWithString:headerString];
}

- (unsigned long long)contentLength {
    unsigned long long length = 0;
    NSData *encapsulationBoundaryData = [([self hasInitialBoundary] ? OHMultipartFormInitialBoundary(self.boundary) : OHMultipartFormEncapsulationBoundary(self.boundary)) dataUsingEncoding:self.stringEncoding];
    length += [encapsulationBoundaryData length];

    NSData *headersData = [[self stringForHeaders] dataUsingEncoding:self.stringEncoding];
    length += [headersData length];
    length += _bodyContentLength;

    NSData *closingBoundaryData = ([self hasFinalBoundary] ? [OHMultipartFormFinalBoundary(self.boundary) dataUsingEncoding:self.stringEncoding] : [NSData data]);
    length += [closingBoundaryData length];
    return length;
}

- (BOOL)hasBytesAvailable {
    if (_phase == OHFinalBoundaryPhase) {
        return YES;
    }

    switch (self.inputStream.streamStatus) {
        case NSStreamStatusNotOpen:
        case NSStreamStatusOpening:
        case NSStreamStatusOpen:
        case NSStreamStatusReading:
        case NSStreamStatusWriting:
            return YES;
        case NSStreamStatusAtEnd:
        case NSStreamStatusClosed:
        case NSStreamStatusError:
        default:
            return NO;
    }
    return NO;
}

- (NSInteger)read:(uint8_t *)buffer
        maxLength:(NSUInteger)length {
    NSInteger totalNumberOfBytesRead = 0;
    if (_phase == OHEncapsulationBoundaryPhase) {
        NSData *encapsulationBoundaryData = [([self hasInitialBoundary] ? OHMultipartFormInitialBoundary(self.boundary) : OHMultipartFormEncapsulationBoundary(self.boundary)) dataUsingEncoding:self.stringEncoding];
        totalNumberOfBytesRead += [self readData:encapsulationBoundaryData intoBuffer:&buffer[totalNumberOfBytesRead] maxLength:(length - (NSUInteger)totalNumberOfBytesRead)];
    }

    if (_phase == OHHeaderPhase) {
        NSData *headersData = [[self stringForHeaders] dataUsingEncoding:self.stringEncoding];
        totalNumberOfBytesRead += [self readData:headersData intoBuffer:&buffer[totalNumberOfBytesRead] maxLength:(length - (NSUInteger)totalNumberOfBytesRead)];
    }

    if (_phase == OHBodyPhase) {
        NSInteger numberOfBytesRead = 0;
        numberOfBytesRead = [self.inputStream read:&buffer[totalNumberOfBytesRead] maxLength:(length - (NSUInteger)totalNumberOfBytesRead)];
        if (numberOfBytesRead == -1) {
            return -1;
        } else {
            totalNumberOfBytesRead += numberOfBytesRead;

            if ([self.inputStream streamStatus] >= NSStreamStatusAtEnd) {
                [self transitionToNextPhase];
            }
        }
    }

    if (_phase == OHFinalBoundaryPhase) {
        NSData *closingBoundaryData = ([self hasFinalBoundary] ? [OHMultipartFormFinalBoundary(self.boundary) dataUsingEncoding:self.stringEncoding] : [NSData data]);
        totalNumberOfBytesRead += [self readData:closingBoundaryData intoBuffer:&buffer[totalNumberOfBytesRead] maxLength:(length - (NSUInteger)totalNumberOfBytesRead)];
    }

    return totalNumberOfBytesRead;
}

- (NSInteger)readData:(NSData *)data
           intoBuffer:(uint8_t *)buffer
            maxLength:(NSUInteger)length {
    NSRange range = NSMakeRange((NSUInteger)_phaseReadOffset, MIN([data length] - ((NSUInteger)_phaseReadOffset), length));
    [data getBytes:buffer range:range];

    _phaseReadOffset += range.length;
    if (((NSUInteger)_phaseReadOffset) >= [data length]) {
        [self transitionToNextPhase];
    }
    return (NSInteger)range.length;
}

- (BOOL)transitionToNextPhase {
    if (![[NSThread currentThread] isMainThread]) {
        dispatch_sync(dispatch_get_main_queue(), ^{
            [self transitionToNextPhase];
        });
        return YES;
    }

    switch (_phase) {
        case OHEncapsulationBoundaryPhase:
            _phase = OHHeaderPhase;
            break;
        case OHHeaderPhase:
            [self.inputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
            [self.inputStream open];
            _phase = OHBodyPhase;
            break;
        case OHBodyPhase:
            [self.inputStream close];
            _phase = OHFinalBoundaryPhase;
            break;
        case OHFinalBoundaryPhase:
        default:
            _phase = OHEncapsulationBoundaryPhase;
            break;
    }
    _phaseReadOffset = 0;
    return YES;
}

#pragma mark - NSCopying
- (instancetype)copyWithZone:(NSZone *)zone {
    OHHttpBodyPart *bodyPart = [[[self class] allocWithZone:zone] init];
    bodyPart.stringEncoding = self.stringEncoding;
    bodyPart.headers = self.headers;
    bodyPart.bodyContentLength = self.bodyContentLength;
    bodyPart.body = self.body;
    bodyPart.boundary = self.boundary;

    return bodyPart;
}

@end