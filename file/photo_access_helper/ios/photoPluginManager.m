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

#import "photoPluginManager.h"
#import <UIKit/UIKit.h>

//UIImagePickerViewController
#import <MobileCoreServices/MobileCoreServices.h>

//PHPickerViewController
#import <PhotosUI/PhotosUI.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

typedef void(^ReadURLCallBack)(NSURL* resultURL);
#define PHOTO_PICKER_TYPE_IMAGE @"image/*"
#define PHOTO_PICKER_TYPE_VIDEO @"video/*"
#define PHOTO_PICKER_TYPE_IMAGE_VIDEO @"*/*"
#define PHOTO_PICKER_BASE_PATH @"file://%@"

@interface photoPluginManager()<UINavigationControllerDelegate,PHPickerViewControllerDelegate,UIImagePickerControllerDelegate>

@property (nonatomic, copy) CallBack currentPhotoPickerResult;

@end

@implementation photoPluginManager

+ (instancetype)shareManager {
    static photoPluginManager *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[photoPluginManager alloc] init];
    });
    return instance;
}

-(void)startPhotoPickerIosWithType:(NSString *)type callBack:(CallBack)photoPickerResult {
    if (photoPickerResult == NULL || type == NULL) {
        return;
    }
    self.currentPhotoPickerResult = photoPickerResult;
    UIViewController *topVC = [self getApplicationTopViewController];
    if(@available(iOS 14, *)) {
        PHPickerConfiguration *config = [[PHPickerConfiguration alloc] init];
        config.selectionLimit = 0;
        if ([type isEqualToString:PHOTO_PICKER_TYPE_IMAGE]) {
            config.filter = [PHPickerFilter imagesFilter];
        } else if ([type isEqualToString:PHOTO_PICKER_TYPE_VIDEO]) {
            config.filter = [PHPickerFilter videosFilter];
        } else if ([type isEqualToString:PHOTO_PICKER_TYPE_IMAGE_VIDEO]) {
            config.filter = [PHPickerFilter anyFilterMatchingSubfilters:@[PHPickerFilter.imagesFilter,PHPickerFilter.videosFilter]];
        } else {
            config.filter = [PHPickerFilter anyFilterMatchingSubfilters:@[PHPickerFilter.imagesFilter,PHPickerFilter.videosFilter]];
        }
        PHPickerViewController *pickerViewController = [[PHPickerViewController alloc] initWithConfiguration:config];
        pickerViewController.delegate = self;
        [topVC presentViewController:pickerViewController animated:YES completion:nil];
    } else {
        UIImagePickerController *imagePickerVC = [[UIImagePickerController alloc] init];
        imagePickerVC.sourceType = UIImagePickerControllerSourceTypePhotoLibrary;
        if ([type isEqualToString:PHOTO_PICKER_TYPE_IMAGE]) {
            imagePickerVC.mediaTypes = @[(NSString *)kUTTypeImage];
        } else if ([type isEqualToString:PHOTO_PICKER_TYPE_VIDEO]) {
            imagePickerVC.mediaTypes = @[(NSString *)kUTTypeMovie];
        } else if ([type isEqualToString:PHOTO_PICKER_TYPE_IMAGE_VIDEO]) {
            imagePickerVC.mediaTypes = @[(NSString *)kUTTypeMovie, (NSString *)kUTTypeImage];
        } else {
            imagePickerVC.mediaTypes = @[(NSString *)kUTTypeMovie, (NSString *)kUTTypeImage];
        }
        imagePickerVC.videoQuality = UIImagePickerControllerQualityTypeHigh;
        imagePickerVC.delegate = self;
        [topVC presentViewController:imagePickerVC animated:YES completion:nil];
    }
}

- (UIViewController *)getApplicationTopViewController {
    UIWindow *mainWindow = [[UIApplication sharedApplication].delegate window];
    if (!mainWindow) {
        NSLog(@"photoAccessHelp photoPickerView select getApplicationTopViewController window is null");
        return [[UIViewController alloc] init];
    }
    UIViewController *viewController = mainWindow.rootViewController;
    return [self findTopViewController:viewController];
}

- (UIViewController *)findTopViewController:(UIViewController*)topViewController {
    while (true) {
        if (topViewController.presentedViewController) {
            topViewController = topViewController.presentedViewController;
        } else if ([topViewController isKindOfClass:[UINavigationController class]]
                    && [(UINavigationController*)topViewController topViewController]) {
            topViewController = [(UINavigationController *)topViewController topViewController];
        } else if ([topViewController isKindOfClass:[UITabBarController class]]) {
            UITabBarController *tab = (UITabBarController *)topViewController;
            topViewController = tab.selectedViewController;
        } else {
            break;
        }
    }
    return topViewController;
}

#pragma mark PHPickerViewControllerDelegate
- (void)picker:(PHPickerViewController *)picker didFinishPicking:(nonnull NSArray<PHPickerResult *> *)results {
    [picker dismissViewControllerAnimated:YES completion:nil];
    if (!self.currentPhotoPickerResult) {
        return;
    }
    NSMutableArray *uriArray = [[NSMutableArray alloc] init];
    if (results.count <= 0) {
        self.currentPhotoPickerResult(uriArray, 0);
        return;
    }
    __weak __typeof(self)weakSelf = self;
    for (int i = 0; i < results.count; i++) {
        PHPickerResult *result = results[i];
        [result.itemProvider loadFileRepresentationForTypeIdentifier:UTTypeMovie.identifier completionHandler:^(NSURL * _Nullable url, NSError * _Nullable error) {
            if(error){
                NSLog(@"PHPickerViewController loadFileRepresentationForTypeIdentifier UITTypeMovie failed");
                return;
            }
            __strong __typeof(weakSelf)strongSelf = weakSelf;
            if (!strongSelf) {
                NSLog(@"PHPickerViewController pick movie failed");
                return;
            }
            [strongSelf loadResultwith:url UriArray:uriArray ChooseCount:results.count];
        }];
        [result.itemProvider loadFileRepresentationForTypeIdentifier:UTTypeImage.identifier completionHandler:^(NSURL * _Nullable url, NSError * _Nullable error) {
            if(error){
                NSLog(@"PHPickerViewController loadFileRepresentationForTypeIdentifier UTTypeImage failed");
                return;
            }
            __strong __typeof(weakSelf)strongSelf = weakSelf;
            if (!strongSelf) {
                NSLog(@"PHPickerViewController pick image failed");
                return;
            }
            [strongSelf loadResultwith:url UriArray:uriArray ChooseCount:results.count];
        }];
    }
}

- (void)loadResultwith:(NSURL *)url UriArray:(NSMutableArray *)uriArray ChooseCount:(NSUInteger)chooseCount {
    if (![[NSFileManager defaultManager] isReadableFileAtPath:[url path]]) {
        NSLog(@"PHPickerViewController loadFileRepresentationForTypeIdentifier url path invalid");
        return;
    }
    NSString *fileName =[url lastPathComponent];
    NSString *tempPath= NSTemporaryDirectory();
    NSString *destinationPath= [[NSString alloc] initWithFormat:@"%@%@",tempPath,fileName];;
    NSURL *destinationUrl = [NSURL fileURLWithPath:destinationPath];
    if ([[NSFileManager defaultManager] isReadableFileAtPath:[destinationUrl path]]) {
        NSString *urlString = [[NSString alloc] initWithFormat:PHOTO_PICKER_BASE_PATH,[destinationUrl path]];
        [uriArray addObject:urlString];
    } else {
        NSError *error;
        [[NSFileManager defaultManager] copyItemAtURL:url toURL:destinationUrl error:&error];
        if (error) {
            NSLog(@"PHPickerViewController copy file failed");
        } else {
            NSString *urlString = [[NSString alloc] initWithFormat:PHOTO_PICKER_BASE_PATH,[destinationUrl path]];
            [uriArray addObject:urlString];
        }
    }
    if (uriArray.count >= chooseCount) {
        self.currentPhotoPickerResult(uriArray, 0);
    }
}

- (void)readURL:(NSURL *)currentURL callBack:(ReadURLCallBack)readURLCallBack {
    BOOL fileUrlAuthozied = [currentURL startAccessingSecurityScopedResource];
    if (fileUrlAuthozied) {
        NSFileCoordinator *fileCoordinator = [[NSFileCoordinator alloc] init];
        NSError *error;
        [fileCoordinator coordinateReadingItemAtURL:currentURL options:0 error:&error byAccessor:^(NSURL *newURL) {
            if (!error) {
                readURLCallBack(newURL);
            } else {
                readURLCallBack(nil);
            }
        }];
        [currentURL stopAccessingSecurityScopedResource];
    } else {
        readURLCallBack(nil);
        NSLog(@"PHPickerViewController didFinishPicking authorized fail");
    }
}

#pragma mark UIImagePickerControllerDelegate
- (void)imagePickerController:(UIImagePickerController *)picker didFinishPickingMediaWithInfo:(NSDictionary<NSString *,id> *)info
{
    [picker dismissViewControllerAnimated:YES completion:nil];
    NSString *pickerType = (NSString*)(info[UIImagePickerControllerMediaType]);
    NSString *pickerUrlString = @"";
    if ([info[UIImagePickerControllerMediaType] isEqualToString:(NSString *)kUTTypeMovie]) {
        NSURL *mediaUrl = [info objectForKey:UIImagePickerControllerMediaURL];
        if (mediaUrl) {
            pickerUrlString = [[NSString alloc] initWithFormat:PHOTO_PICKER_BASE_PATH,[mediaUrl path]];
        }
    } else if ([info[UIImagePickerControllerMediaType] isEqualToString:(NSString *)kUTTypeImage]) {
        NSURL *imageUrl = [info objectForKey:UIImagePickerControllerImageURL];
        if (imageUrl) {
            pickerUrlString = [[NSString alloc] initWithFormat:PHOTO_PICKER_BASE_PATH,[imageUrl path]];
        }
    } else {
        NSLog(@"imagePickerController didFinishPickingMediaWithInfo type unknow");
    }
    if (pickerUrlString && ![@"" isEqualToString:pickerUrlString]) {
        self.currentPhotoPickerResult(@[pickerUrlString],0);
    }
}

- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker {
    [picker dismissViewControllerAnimated:YES completion:nil];
    self.currentPhotoPickerResult(@[],0);
}

- (BOOL)checkPhotoPermission {
    PHAuthorizationStatus status = [PHPhotoLibrary authorizationStatus];
    switch (status) {
        case PHAuthorizationStatusNotDetermined:
        case PHAuthorizationStatusRestricted:
        case PHAuthorizationStatusDenied: {
            return NO;
        }
        case PHAuthorizationStatusLimited:
        case PHAuthorizationStatusAuthorized: {
            return YES;
        }
        default:
            break;
    }
    return NO;
}

- (PHFetchOptions *)getFetchOptionsWithSortDic:(NSDictionary<NSString *, NSNumber *> *)sortDic {
    PHFetchOptions *fetchOptions = [[PHFetchOptions alloc] init];
    if (sortDic.count > 0) {
        NSMutableArray *sortDescriptors = [[NSMutableArray alloc] init];
        for (NSString *key in sortDic) {
            int value = [sortDic[key] intValue];
            [sortDescriptors addObject:[NSSortDescriptor sortDescriptorWithKey:key ascending:value]];
        }
        fetchOptions.sortDescriptors = sortDescriptors;
    }
    return fetchOptions;
}

- (PHFetchResult *)getFetchResult:(int)fetchLimit offset:(int)offset asset:(PHFetchResult *)asset {
    NSRange range = NSMakeRange(offset, fetchLimit);
    NSInteger count = asset.count;
    int64_t fetchCount = (int64_t)offset + (int64_t)fetchLimit;
    if (fetchCount > count) {
        range.length = MAX(count - offset, 0);
    }
    PHFetchResult *subFetchResults = [asset objectsAtIndexes:[NSIndexSet indexSetWithIndexesInRange:range]];
    return subFetchResults;
}

-(PHFetchResult *)getPhotoFetchResultPredicate:(NSString *)predicate
                                    fetchLimit:(int)fetchLimit
                                        offset:(int)offset
                                       sortKey:(NSDictionary<NSString *, NSNumber *> *)sortDic
                               localIdentifier:(NSString *)localIdentifier {
    PHFetchOptions *fetchOptions = [self getFetchOptionsWithSortDic:sortDic];
    if (predicate != nil && predicate.length > 0) {
        fetchOptions.predicate = [NSPredicate predicateWithFormat:predicate];
    }
    PHFetchResult *asset;
    if (localIdentifier != nil && localIdentifier.length > 0) {
        PHFetchResult *collection = [PHAssetCollection fetchAssetCollectionsWithLocalIdentifiers:@[localIdentifier] 
                                                                                         options:nil];
        asset = [PHAsset fetchAssetsInAssetCollection:collection.firstObject options:fetchOptions];
    } else {
        asset = [PHAsset fetchAssetsWithOptions:fetchOptions];
    }
    return [self getFetchResult:fetchLimit offset:offset asset:asset];
}

-(PHFetchResult *)getAlbumFetchResultLimit:(int)fetchLimit
                                    offset:(int)offset
                                   sortKey:(NSDictionary<NSString *, NSNumber *> *)sortDic
                                 albumType:(int)albumType
                              albumSubType:(long long)albumSubType {
    PHFetchOptions * fetchOptions = [self getFetchOptionsWithSortDic:sortDic];
    PHFetchResult *userAlbums = [PHAssetCollection fetchAssetCollectionsWithType:albumType
                                                                         subtype:albumSubType
                                                                         options:fetchOptions];
    return [self getFetchResult:fetchLimit offset:offset asset:userAlbums];
}

@end
