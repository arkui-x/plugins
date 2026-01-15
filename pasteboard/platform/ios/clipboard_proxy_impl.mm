/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
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

#import "clipboard_proxy_impl.h"

#include <atomic>
#include <map>
#include <unordered_set>

#include "common/constant.h"
#include "image_mime_type.h"
#include "log.h"
#include "pasteboard_error.h"
#include "pixel_map.h"

#import <MobileCoreServices/UTCoreTypes.h>
#import <UIKit/UIKit.h>
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 150000
#import <UniformTypeIdentifiers/UTCoreTypes.h>
#endif

namespace {
static UIPasteboard *GetGlobalPasteboard() {
    static UIPasteboard *pasteboard = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        pasteboard = [UIPasteboard generalPasteboard];
    });
    return pasteboard;
}
}

namespace OHOS {
namespace Plugin {

static NSString *const TYPE_RECORD = @"com.arkui-x.pasteboard.record";
static NSString *const TYPE_PROPS = @"com.arkui-x.pasteboard.props";
static NSString *const TYPE_WANT = @"com.arkui-x.pasteboard.want";
static NSString *const TYPE_IMAGE = @"image/image";
static NSString *const TYPE_IMAGE_PNG = @"image/png";
static NSString *const TYPE_IMAGE_JPEG = @"image/jpeg";
static NSString *const TYPE_TEXT_PLAIN = @"text/plain";
static NSString *const TYPE_TEXT_HTML = @"text/html";
static NSString *const TYPE_WEB_URL = @"web/url";
static NSString *const TYPE_FILE_URL = @"file/url";

struct ObserverEntry {
    id token;
    std::shared_ptr<std::atomic_bool> alive;
};
static std::map<sptr<PasteboardObserverStub>, ObserverEntry> g_observerTokenMap;

ClipboardProxyImpl::~ClipboardProxyImpl()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& it : g_observerTokenMap) {
        if (it.second.alive) {
            it.second.alive->store(false, std::memory_order_relaxed);
        }
        if (it.second.token) {
            [[NSNotificationCenter defaultCenter] removeObserver:it.second.token];
        }
    }
    g_observerTokenMap.clear();
}

std::string TranslateMimeType(NSString *type)
{
    if (@available(iOS 15.0, *)) {
        if ([type isEqualToString:UTTypeUTF8PlainText.identifier]) {
            return MIMETYPE_TEXT_PLAIN;
        }
        if ([type isEqualToString:UTTypePNG.identifier] ||
            [type isEqualToString:UTTypeJPEG.identifier] ||
            [type isEqualToString:UTTypeImage.identifier]) {
            return MIMETYPE_PIXELMAP;
        }
        if ([type isEqualToString:UTTypeHTML.identifier]) {
            return MIMETYPE_TEXT_HTML;
        }
        if ([type isEqualToString:UTTypeURL.identifier] ||
            [type isEqualToString:UTTypeFileURL.identifier]) {
            return MIMETYPE_TEXT_URI;
        }
        if ([type isEqualToString:TYPE_WANT]) {
            return MIMETYPE_TEXT_WANT;
        }
    } else {
        if ([type isEqualToString:(NSString *)kUTTypeUTF8PlainText]) {
            return MIMETYPE_TEXT_PLAIN;
        }
        if ([type isEqualToString:(NSString *)kUTTypePNG] ||
            [type isEqualToString:(NSString *)kUTTypeJPEG] ||
            [type isEqualToString:(NSString *)kUTTypeImage]) {
            return MIMETYPE_PIXELMAP;
        }
        if ([type isEqualToString:(NSString *)kUTTypeHTML]) {
            return MIMETYPE_TEXT_HTML;
        }
        if ([type isEqualToString:(NSString *)kUTTypeURL] ||
            [type isEqualToString:(NSString *)kUTTypeFileURL]) {
            return MIMETYPE_TEXT_URI;
        }
        if ([type isEqualToString:TYPE_WANT]) {
            return MIMETYPE_TEXT_WANT;
        }
    }
    return "";
}

NSString *TranslateMimeType(const std::string& mimeType)
{
    if (@available(iOS 15.0, *)) {
        if (mimeType == MIMETYPE_TEXT_PLAIN) {
            return UTTypeUTF8PlainText.identifier;
        }
        if (mimeType == MIMETYPE_PIXELMAP) {
            return UTTypeImage.identifier;
        }
        if (mimeType == MIMETYPE_TEXT_HTML) {
            return UTTypeHTML.identifier;
        }
        if (mimeType == MIMETYPE_TEXT_URI) {
            return UTTypeURL.identifier;
        }
        if (mimeType == MIMETYPE_TEXT_WANT) {
            return TYPE_WANT;
        }
    } else {
        if (mimeType == MIMETYPE_TEXT_PLAIN) {
            return (NSString *)kUTTypeUTF8PlainText;
        }
        if (mimeType == MIMETYPE_PIXELMAP) {
            return (NSString *)kUTTypeImage;
        }
        if (mimeType == MIMETYPE_TEXT_HTML) {
            return (NSString *)kUTTypeHTML;
        }
        if (mimeType == MIMETYPE_TEXT_URI) {
            return (NSString *)kUTTypeURL;
        }
        if (mimeType == MIMETYPE_TEXT_WANT) {
            return TYPE_WANT;
        }
    }
    return @"";
}

NSString *TranslateImageUTType(const std::string& imageMimeType)
{
    if (@available(iOS 15.0, *)) {
        if (imageMimeType == Media::IMAGE_JPEG_FORMAT) {
            return UTTypeJPEG.identifier;
        }
        if (imageMimeType == Media::IMAGE_PNG_FORMAT) {
            return UTTypePNG.identifier;
        }
        return UTTypeImage.identifier;
    } else {
        if (imageMimeType == Media::IMAGE_JPEG_FORMAT) {
            return (NSString *)kUTTypeJPEG;
        }
        if (imageMimeType == Media::IMAGE_PNG_FORMAT) {
            return (NSString *)kUTTypePNG;
        }
        return (NSString *)kUTTypeImage;
    }
}

NSString *TranslateUTType(NSString *type)
{
    if (@available(iOS 15.0, *)) {
        if ([type isEqualToString:TYPE_IMAGE_PNG]) {
            return UTTypePNG.identifier;
        }
        if ([type isEqualToString:TYPE_IMAGE_JPEG]) {
            return UTTypeJPEG.identifier;
        }
        if ([type isEqualToString:TYPE_IMAGE]) {
            return UTTypeImage.identifier;
        }
        if ([type isEqualToString:TYPE_TEXT_PLAIN]) {
            return UTTypeUTF8PlainText.identifier;
        }
        if ([type isEqualToString:TYPE_TEXT_HTML]) {
            return UTTypeHTML.identifier;
        }
        if ([type isEqualToString:TYPE_WEB_URL]) {
            return UTTypeURL.identifier;
        }
        if ([type isEqualToString:TYPE_FILE_URL]) {
            return UTTypeFileURL.identifier;
        }
    } else {
        if ([type isEqualToString:TYPE_IMAGE_PNG]) {
            return (NSString *)kUTTypePNG;
        }
        if ([type isEqualToString:TYPE_IMAGE_JPEG]) {
            return (NSString *)kUTTypeJPEG;
        }
        if ([type isEqualToString:TYPE_IMAGE]) {
            return (NSString *)kUTTypeImage;
        }
        if ([type isEqualToString:TYPE_TEXT_PLAIN]) {
            return (NSString *)kUTTypeUTF8PlainText;
        }
        if ([type isEqualToString:TYPE_TEXT_HTML]) {
            return (NSString *)kUTTypeHTML;
        }
        if ([type isEqualToString:TYPE_WEB_URL]) {
            return (NSString *)kUTTypeURL;
        }
        if ([type isEqualToString:TYPE_FILE_URL]) {
            return (NSString *)kUTTypeFileURL;
        }
    }
    return @"";
}

static CGImageRef CreateCGImageFromData(
    const std::vector<uint8_t>& bgra, int32_t width, int32_t height, uint32_t bytesPerRow)
{
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    if (!colorSpace) {
        return nullptr;
    }
    CFDataRef cfData = CFDataCreate(kCFAllocatorDefault, bgra.data(), (CFIndex)bgra.size());
    if (!cfData) {
        CGColorSpaceRelease(colorSpace);
        return nullptr;
    }
    CGDataProviderRef provider = CGDataProviderCreateWithCFData(cfData);
    CFRelease(cfData);
    if (!provider) {
        CGColorSpaceRelease(colorSpace);
        return nullptr;
    }
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst;
    CGImageRef imageRef = CGImageCreate(width, height, 8, 32, bytesPerRow, colorSpace, bitmapInfo, provider,
                                        NULL, true, kCGRenderingIntentDefault);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(colorSpace);
    return imageRef;
}

static UIImage *PixelMapToUIImage(const std::shared_ptr<Media::PixelMap>& pixelMap)
{
    if (pixelMap == nullptr) {
        return nil;
    }
    const int32_t width = pixelMap->GetWidth();
    const int32_t height = pixelMap->GetHeight();
    if (width <= 0 || height <= 0) {
        return nil;
    }
    if (static_cast<uint32_t>(width) > (UINT32_MAX / 4)) {
        return nil;
    }
    const uint32_t bytesPerRow = static_cast<uint32_t>(width) * 4;
    if (static_cast<uint64_t>(bytesPerRow) > (UINT64_MAX / static_cast<uint64_t>(height))) {
        return nil;
    }
    const uint64_t bufferSize = static_cast<uint64_t>(bytesPerRow) * static_cast<uint64_t>(height);
    if (bufferSize == 0) {
        return nil;
    }
    std::vector<uint8_t> bgra(static_cast<size_t>(bufferSize));
    Media::RWPixelsOptions opts;
    opts.pixels = bgra.data();
    opts.bufferSize = bufferSize;
    opts.stride = bytesPerRow;
    opts.region = Media::Rect { 0, 0, width, height };
    opts.pixelFormat = Media::PixelFormat::BGRA_8888;
    if (pixelMap->ReadPixels(opts) != 0) {
        return nil;
    }
    CGImageRef imageRef = CreateCGImageFromData(bgra, width, height, bytesPerRow);
    UIImage *image = imageRef ? [UIImage imageWithCGImage:imageRef] : nil;
    if (imageRef) {
        CGImageRelease(imageRef);
    }
    return image;
}

static std::shared_ptr<Media::PixelMap> UIImageToPixelMap(UIImage *image)
{
    if (!image) {
        return nullptr;
    }
    CGImageRef cgImage = image.CGImage;
    if (!cgImage) {
        return nullptr;
    }
    const size_t width = CGImageGetWidth(cgImage);
    const size_t height = CGImageGetHeight(cgImage);
    if (width == 0 || height == 0) {
        return nullptr;
    }
    const size_t bytesPerRow = width * 4;
    std::vector<uint8_t> pixels(bytesPerRow * height);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    if (!colorSpace) {
        return nullptr;
    }
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst;
    CGContextRef context = CGBitmapContextCreate(pixels.data(), width, height, 8, bytesPerRow, colorSpace, bitmapInfo);
    CGColorSpaceRelease(colorSpace);
    if (!context) {
        return nullptr;
    }
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), cgImage);
    CGContextRelease(context);
    Media::InitializationOptions opts;
    opts.size.width = static_cast<int32_t>(width);
    opts.size.height = static_cast<int32_t>(height);
    opts.pixelFormat = Media::PixelFormat::BGRA_8888;
    opts.alphaType = Media::AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    opts.editable = true;
    std::unique_ptr<Media::PixelMap> pixelMap = Media::PixelMap::Create(opts);
    if (!pixelMap) {
        return nullptr;
    }
    const uint64_t bufferSize = static_cast<uint64_t>(pixels.size());
    uint32_t ret = pixelMap->WritePixels(pixels.data(), bufferSize);
    if (ret != 0) {
        return nullptr;
    }
    return std::shared_ptr<Media::PixelMap>(pixelMap.release());
}

NSDictionary *MakePropsItem(const PasteDataProperty& props)
{
    const size_t capacity = props.CountTLV();
    if (capacity == 0) {
        return nil;
    }
    WriteOnlyBuffer buffer(capacity);
    if (props.EncodeTLV(buffer)) {
        NSData *propsData = [NSData dataWithBytes:buffer.Data() length:buffer.Size()];
        if (!propsData || propsData.length == 0) {
            return nil;
        }
        return @{ TYPE_PROPS : propsData };
    }
    return nil;
}

NSDictionary *MakePixelMapItem(std::shared_ptr<Media::PixelMap> pixelMap)
{
    if (pixelMap == nullptr) {
        return nil;
    }
    Media::ImageInfo imageInfo;
    pixelMap->GetImageInfo(imageInfo);
    const std::string format = imageInfo.encodedFormat;
    if (format == Media::IMAGE_JPEG_FORMAT || format == Media::IMAGE_PNG_FORMAT) {
        UIImage *image = PixelMapToUIImage(pixelMap);
        if (!image) {
            return nil;
        }
        return @{ TranslateImageUTType(format) : image };
    }
    std::vector<uint8_t> buffer = TLVUtils::PixelMap2Vector(pixelMap);
    NSData *imageData = [NSData dataWithBytes:buffer.data() length:buffer.size()];
    if (!imageData || imageData.length == 0) {
        return nil;
    }
    return @{ TranslateImageUTType(format) : imageData };
}

NSDictionary *MakeTextHtmlItem(std::shared_ptr<std::string> plain, std::shared_ptr<std::string> html)
{
    if (html == nullptr) {
        return nil;
    }
    NSString *htmlString = [NSString stringWithCString:html->c_str() encoding:NSUTF8StringEncoding];
    if (plain != nullptr) {
        NSString *plainString = [NSString stringWithCString:plain->c_str() encoding:NSUTF8StringEncoding];
        return @{ TranslateMimeType(MIMETYPE_TEXT_PLAIN) : plainString,
                  TranslateMimeType(MIMETYPE_TEXT_HTML) : htmlString };
    }
    return @{ TranslateMimeType(MIMETYPE_TEXT_PLAIN) : @"",
              TranslateMimeType(MIMETYPE_TEXT_HTML) : htmlString };
}

NSDictionary *MakePlainTextItem(std::shared_ptr<std::string> plain)
{
    if (plain == nullptr) {
        return nil;
    }
    NSString *plainString = [NSString stringWithCString:plain->c_str() encoding:NSUTF8StringEncoding];
    return @{ TranslateMimeType(MIMETYPE_TEXT_PLAIN) : plainString };
}

NSDictionary *MakeUriItem(std::shared_ptr<Uri> uri)
{
    if (uri == nullptr) {
        return nil;
    }
    NSString *uriString = [NSString stringWithCString:uri->ToString().c_str() encoding:NSUTF8StringEncoding];
    NSURL *url = nil;
    if ([uriString hasPrefix:@"file://"]) {
        url = [NSURL URLWithString:uriString];
    } else if ([uriString hasPrefix:@"/"]) {
        url = [NSURL fileURLWithPath:uriString];
    } else {
        url = [NSURL URLWithString:uriString];
    }
    if (!url) {
        return nil;
    }
    return @{ TranslateMimeType(MIMETYPE_TEXT_URI) : url };
}

NSDictionary *MakeWantItem(std::shared_ptr<AAFwk::Want> want)
{
    if (want == nullptr) {
        return nil;
    }
    const std::string wantJson = TLVUtils::Want2Json(*want);
    if (wantJson.empty()) {
        return nil;
    }
    NSString *jsonString = [NSString stringWithUTF8String:wantJson.c_str()];
    NSData *jsonData = [jsonString dataUsingEncoding:NSUTF8StringEncoding];
    if (!jsonData || jsonData.length == 0) {
        return nil;
    }
    return @{ TYPE_WANT : jsonData };
}

NSDictionary *MakeCustomItem(std::shared_ptr<MineCustomData> custom)
{
    if (custom == nullptr) {
        return nil;
    }
    auto itemData = custom->GetItemData();
    if (itemData.empty()) {
        return nil;
    }
    std::string key = itemData.begin()->first;
    if (key.empty()) {
        return nil;
    }
    const size_t capacity = custom->CountTLV();
    if (capacity == 0) {
        return nil;
    }
    WriteOnlyBuffer buffer(capacity);
    if (custom->EncodeTLV(buffer)) {
        NSData *customData = [NSData dataWithBytes:buffer.Data() length:buffer.Size()];
        if (!customData || customData.length == 0) {
            return nil;
        }
        return @{ [NSString stringWithUTF8String:key.c_str()] : customData };
    }
    return nil;
}

NSDictionary *MakeRecordItem(std::shared_ptr<PasteDataRecord> record)
{
    if (record == nullptr) {
        return nil;
    }
    const size_t capacity = record->CountTLV();
    if (capacity == 0) {
        return nil;
    }
    WriteOnlyBuffer buffer(capacity);
    if (record->EncodeTLV(buffer)) {
        NSData *recordData = [NSData dataWithBytes:buffer.Data() length:buffer.Size()];
        if (!recordData || recordData.length == 0) {
            return nil;
        }
        return @{ TYPE_RECORD : recordData };
    }
    return nil;
}

NSDictionary *MakeItem(std::shared_ptr<PasteDataRecord> record)
{
    if (record == nullptr) {
        return nil;
    }
    NSMutableDictionary *item = [NSMutableDictionary dictionary];
    auto recordItem = MakeRecordItem(record);
    if (recordItem) {
        [item addEntriesFromDictionary:recordItem];
    }
    auto plainText = MakePlainTextItem(record->GetPlainText());
    if (plainText) {
        [item addEntriesFromDictionary:plainText];
    }
    auto uri = MakeUriItem(record->GetUri());
    if (uri) {
        [item addEntriesFromDictionary:uri];
    }
    auto htmlText = MakeTextHtmlItem(record->GetPlainText(), record->GetHtmlText());
    if (htmlText) {
        [item addEntriesFromDictionary:htmlText];
    }
    auto pixelMap = MakePixelMapItem(record->GetPixelMap());
    if (pixelMap) {
        [item addEntriesFromDictionary:pixelMap];
    }
    auto want = MakeWantItem(record->GetWant());
    if (want) {
        [item addEntriesFromDictionary:want];
    }
    auto custom = MakeCustomItem(record->GetCustomData());
    if (custom) {
        [item addEntriesFromDictionary:custom];
    }
    return item;
}

static void AppendRecordItem(NSMutableArray<NSDictionary*> *items, const std::shared_ptr<PasteDataRecord>& record)
{
    if (items == nullptr || record == nullptr) {
        return;
    }
    @autoreleasepool {
        NSDictionary *item = MakeItem(record);
        if (item.count > 0) {
            [items addObject:item];
        }
    }
}

int32_t ClipboardProxyImpl::SetPasteData(const PasteData& pasteData)
{
    @autoreleasepool {
        auto records = pasteData.AllRecords();
        if (records.empty()) {
            Clear();
            return ERR_OK;
        }
        UIPasteboard *pasteboard = GetGlobalPasteboard();
        if (!pasteboard) {
            return ERR_NO_INIT;
        }
        NSMutableArray<NSDictionary*> *items = [NSMutableArray arrayWithCapacity:records.size() + 1];
        for (auto record = records.rbegin(); record != records.rend(); ++record) {
            AppendRecordItem(items, *record);
        }
        auto propsItem = MakePropsItem(pasteData.GetProperty());
        if (propsItem) {
            [items addObject:propsItem];
        }
        pasteboard.items = items;
        return ERR_OK;
    }
}

static std::shared_ptr<Media::PixelMap> PixelMapFromPasteboardValue(id value)
{
    if (!value) {
        return nullptr;
    }
    if ([value isKindOfClass:[UIImage class]]) {
        return UIImageToPixelMap((UIImage *)value);
    }
    if ([value isKindOfClass:[NSData class]]) {
        NSData *data = (NSData *)value;
        if (!data || data.length == 0) {
            return nullptr;
        }
        std::vector<uint8_t> buffer((const uint8_t *)data.bytes,
                                    (const uint8_t *)data.bytes + data.length);
        auto pixelMap = TLVUtils::Vector2PixelMap(buffer);
        if (pixelMap) {
            return pixelMap;
        }
        UIImage *image = [UIImage imageWithData:data];
        return UIImageToPixelMap(image);
    }
    return nullptr;
}

static std::shared_ptr<Media::PixelMap> ProcessImageItem(NSDictionary<NSString *, id> *item)
{
    id pngVal = item[TranslateUTType(TYPE_IMAGE_PNG)];
    id jpegVal = item[TranslateUTType(TYPE_IMAGE_JPEG)];
    id imageVal = item[TranslateUTType(TYPE_IMAGE)];
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    if (pngVal) {
        pixelMap = PixelMapFromPasteboardValue(pngVal);
    }
    if (!pixelMap && jpegVal) {
        pixelMap = PixelMapFromPasteboardValue(jpegVal);
    }
    if (!pixelMap && imageVal) {
        pixelMap = PixelMapFromPasteboardValue(imageVal);
    }
    return pixelMap;
}

static std::shared_ptr<std::string> ProcessPlainTextItem(NSDictionary<NSString *, id> *item)
{
    NSString *textStr = nil;
    id text = item[TranslateUTType(TYPE_TEXT_PLAIN)];
    if (text && [text isKindOfClass:[NSString class]]) {
        textStr = [NSString stringWithString:text];
    }
    if (!textStr || textStr.length == 0) {
        return nullptr;
    }
    return std::make_shared<std::string>(textStr.UTF8String);
}

static std::shared_ptr<std::string> ProcessHtmlItem(NSDictionary<NSString *, id> *item)
{
    NSString *htmlStr = nil;
    id html = item[TranslateUTType(TYPE_TEXT_HTML)];
    if (html && [html isKindOfClass:[NSString class]]) {
        htmlStr = [NSString stringWithString:html];
    }
    std::shared_ptr<std::string> htmlText = nullptr;
    if (!htmlStr || htmlStr.length == 0) {
        return nullptr;
    }
    return std::make_shared<std::string>(htmlStr.UTF8String);
}

static std::shared_ptr<Uri> ProcessUriItem(NSDictionary<NSString *, id> *item)
{
    NSURL *webUrl = item[TranslateUTType(TYPE_WEB_URL)];
    NSURL *fileUrl = item[TranslateUTType(TYPE_FILE_URL)];
    if (!webUrl && !fileUrl) {
        return nullptr;
    }
    NSURL *url = webUrl ? webUrl : fileUrl;
    std::string path = url.absoluteString ? url.absoluteString.UTF8String : "";
    if (path.empty()) {
        return nullptr;
    }
    return std::make_shared<Uri>(path);
}

static std::shared_ptr<AAFwk::Want> ProcessWantItem(NSDictionary<NSString *, id> *item)
{
    id data = item[TYPE_WANT];
    if (!data || ![data isKindOfClass:[NSData class]] || [(NSData *)data length] == 0) {
        return nullptr;
    }
    NSData *wantData = [NSData dataWithData:data];
    NSString *wantJsonString = [[NSString alloc] initWithData:wantData encoding:NSUTF8StringEncoding];
    if (!wantJsonString || wantJsonString.length == 0) {
        return nullptr;
    }
    return TLVUtils::Json2Want(std::string([wantJsonString UTF8String]));
}

static bool IsInternalPasteboardType(NSString *type)
{
    if (!type || type.length == 0) {
        return true;
    }
    return [type isEqualToString:TYPE_PROPS] || [type isEqualToString:TYPE_RECORD];
}

static std::vector<std::shared_ptr<MineCustomData>> ProcessCustomDataItem(NSDictionary<NSString *, id> *item)
{
    std::vector<std::shared_ptr<MineCustomData>> customs;
    for (NSString *key in item.allKeys) {
        if (IsInternalPasteboardType(key)) {
            continue;
        }
        std::string mime = TranslateMimeType(key);
        if (!mime.empty()) {
            continue;
        }
        id data = item[key];
        if (!data || ![data isKindOfClass:[NSData class]] || [(NSData *)data length] == 0) {
            continue;
        }
        NSData *customData = [NSData dataWithData:data];
        std::vector<uint8_t> buffer((const uint8_t *)customData.bytes,
                                    (const uint8_t *)customData.bytes + customData.length);
        ReadOnlyBuffer readBuffer(buffer);
        auto custom = std::make_shared<MineCustomData>();
        if (custom->DecodeTLV(readBuffer)) {
            customs.push_back(custom);
        }
    }
    return customs;
}

static void ProcessPasteboardItem(NSDictionary<NSString *, id> *item, PasteData& pasteData)
{
    std::string mimeType = "";
    for (NSString *key in item.allKeys) {
        std::string mime = TranslateMimeType(key);
        if (!mime.empty()) {
            mimeType = mime;
            break;
        }
    }
    if (mimeType.empty() && item.allKeys.count > 0) {
        mimeType = [NSString stringWithString:item.allKeys.firstObject].UTF8String;
    }
    PasteDataRecord::Builder builder(mimeType);
    auto pixelMap = ProcessImageItem(item);
    if (pixelMap) {
        builder.SetPixelMap(pixelMap);
    }
    auto plainText = ProcessPlainTextItem(item);
    if (plainText) {
        builder.SetPlainText(plainText);
    }
    auto htmlText = ProcessHtmlItem(item);
    if (htmlText) {
        builder.SetHtmlText(htmlText);
    }
    auto uri = ProcessUriItem(item);
    if (uri) {
        builder.SetUri(uri);
    }
    auto want = ProcessWantItem(item);
    if (want) {
        builder.SetWant(want);
    }
    auto customs = ProcessCustomDataItem(item);
    for (auto& custom : customs) {
        builder.SetCustomData(custom);
    }
    pasteData.AddRecord(builder.Build());
}

static bool TryDecodePropsItem(NSDictionary<NSString *, id> *item, PasteData& pasteData)
{
    id data = item[TYPE_PROPS];
    if (!data || ![data isKindOfClass:[NSData class]] || [(NSData *)data length] == 0) {
        return false;
    }
    NSData *propsData = [NSData dataWithData:data];
    std::vector<uint8_t> buffer((const uint8_t *)propsData.bytes,
                                (const uint8_t *)propsData.bytes + propsData.length);
    ReadOnlyBuffer readBuffer(buffer);
    PasteDataProperty props;
    if (!props.DecodeTLV(readBuffer)) {
        return false;
    }
    pasteData.SetProperty(props);
    return true;
}

static bool TryDecodeRecordItem(NSDictionary<NSString *, id> *item, PasteData& pasteData)
{
    id data = item[TYPE_RECORD];
    if (!data || ![data isKindOfClass:[NSData class]] || [(NSData *)data length] == 0) {
        return false;
    }
    NSData *recordData = [NSData dataWithData:data];
    std::vector<uint8_t> buffer((const uint8_t *)recordData.bytes,
                                (const uint8_t *)recordData.bytes + recordData.length);
    ReadOnlyBuffer readBuffer(buffer);
    auto record = std::make_shared<PasteDataRecord>();
    if (!record) {
        return false;
    }
    if (!record->DecodeTLV(readBuffer)) {
        return false;
    }
    pasteData.AddRecord(record);
    return true;
}

int32_t ClipboardProxyImpl::GetPasteData(PasteData& pasteData, int32_t& realErrorCode)
{
    @autoreleasepool {
        UIPasteboard *pasteboard = GetGlobalPasteboard();
        if (!pasteboard) {
            realErrorCode = ERR_NO_INIT;
            return ERR_NO_INIT;
        }
        for (NSDictionary<NSString *, id> *item in pasteboard.items) {
            if (TryDecodePropsItem(item, pasteData)) {
                continue;
            }
            if (TryDecodeRecordItem(item, pasteData)) {
                continue;
            }
            ProcessPasteboardItem(item, pasteData);
        }
        realErrorCode = ERR_OK;
        return ERR_OK;
    }
}

int ClipboardProxyImpl::Clear()
{
    @autoreleasepool {
        UIPasteboard *pasteboard = GetGlobalPasteboard();
        if (!pasteboard) {
            return ERR_NO_INIT;
        }
        pasteboard.items = @[];
        return ERR_OK;
    }
}

bool ClipboardProxyImpl::Subscribe(PasteboardObserverType type, sptr<PasteboardObserverStub> callback)
{
    if (!callback) {
        return false;
    }
    UIPasteboard *pasteboard = GetGlobalPasteboard();
    if (!pasteboard) {
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (g_observerTokenMap.find(callback) != g_observerTokenMap.end()) {
        return true;
    }
    auto alive = std::make_shared<std::atomic_bool>(true);
    id token = [[NSNotificationCenter defaultCenter] addObserverForName:UIPasteboardChangedNotification
        object:pasteboard queue:[NSOperationQueue mainQueue] usingBlock:^(NSNotification * _Nonnull notification) {
        if (!alive || !alive->load(std::memory_order_relaxed)) {
            return;
        }
        if (callback && !notification.userInfo) {
            callback->OnPasteboardChanged();
        }
    }];
    if (!token) {
        return false;
    }
    g_observerTokenMap[callback] = { token, alive };
    return true;
}

void ClipboardProxyImpl::Unsubscribe(PasteboardObserverType type, sptr<PasteboardObserverStub> callback)
{
    if (!callback) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = g_observerTokenMap.find(callback);
    if (it != g_observerTokenMap.end()) {
        if (it->second.alive) {
            it->second.alive->store(false, std::memory_order_relaxed);
        }
        if (it->second.token) {
            [[NSNotificationCenter defaultCenter] removeObserver:it->second.token];
        }
        g_observerTokenMap.erase(it);
    }
}

void ClipboardProxyImpl::NotifyObservers() {}

int ClipboardProxyImpl::DetectPatterns(
    const std::vector<Pattern>& patternsToCheck,
    std::vector<Pattern>& funcResult)
{
    return -1;
}

ErrCode ClipboardProxyImpl::HasPasteData(bool& funcResult)
{
    UIPasteboard *pasteboard = GetGlobalPasteboard();
    if (!pasteboard) {
        return ERR_NO_INIT;
    }
    funcResult = (pasteboard.items.count > 0);
    return ERR_OK;
}

ErrCode ClipboardProxyImpl::HasDataType(const std::string& mimeType, bool& funcResult)
{
    if (mimeType.empty()) {
        return static_cast<int>(PasteboardError::PARAM_ERROR);
    }
    UIPasteboard *pasteboard = GetGlobalPasteboard();
    if (!pasteboard) {
        return ERR_NO_INIT;
    }
    NSString *type = TranslateMimeType(mimeType);
    if (!type || type.length == 0) {
        type = [NSString stringWithUTF8String:mimeType.c_str()];
    }
    funcResult = [pasteboard containsPasteboardTypes:@[type] inItemSet:nil];
    return ERR_OK;
}

NSSet<NSString *> *GetAllTypesFromPasteboard(UIPasteboard *pasteboard)
{
    if (!pasteboard) {
        return [NSSet set];
    }
    NSArray<NSDictionary<NSString *, id> *> *items = pasteboard.items;
    if (!items || items.count == 0) {
        return [NSSet set];
    }
    NSMutableSet<NSString *> *typeSet = [NSMutableSet set];
    for (NSDictionary<NSString *, id> *item in items) {
        for (NSString *key in item) {
            if (key.length > 0) {
                [typeSet addObject:key];
            }
        }
    }
    return typeSet;
}

static bool TryGetMimeTypeFromPasteboardType(NSString *type, std::string& mimeType)
{
    mimeType = TranslateMimeType(type);
    if (!mimeType.empty()) {
        return true;
    }
    const char* cstr = type.UTF8String;
    if (!cstr || cstr[0] == '\0') {
        return false;
    }
    mimeType = cstr;
    return true;
}

ErrCode ClipboardProxyImpl::GetMimeTypes(std::vector<std::string>& funcResult)
{
    @autoreleasepool {
        funcResult.clear();
        UIPasteboard *pasteboard = GetGlobalPasteboard();
        if (!pasteboard) {
            return ERR_NO_INIT;
        }
        NSSet<NSString *> *typeSet = GetAllTypesFromPasteboard(pasteboard);
        if (!typeSet || typeSet.count == 0) {
            return ERR_OK;
        }
        funcResult.reserve(typeSet.count);
        std::unordered_set<std::string> set;
        set.reserve(typeSet.count);
        for (NSString *type in typeSet) {
            if (IsInternalPasteboardType(type)) {
                continue;
            }
            std::string mimeType;
            if (!TryGetMimeTypeFromPasteboardType(type, mimeType)) {
                continue;
            }
            if (set.insert(mimeType).second) {
                funcResult.emplace_back(std::move(mimeType));
            }
        }
        return ERR_OK;
    }
}

ErrCode ClipboardProxyImpl::GetChangeCount(uint32_t& changeCount)
{
    UIPasteboard *pasteboard = GetGlobalPasteboard();
    if (!pasteboard) {
        return ERR_NO_INIT;
    }
    changeCount = (uint32_t)pasteboard.changeCount;
    return ERR_OK;
}
} // namespace Plugin
} // namespace OHOS
