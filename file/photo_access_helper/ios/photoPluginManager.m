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
#import <Photos/Photos.h>
#import <MobileCoreServices/MobileCoreServices.h>

//PHPickerViewController
#import <PhotosUI/PhotosUI.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

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
    if (photoPickerResult == NULL) {
        return;
    }
    self.currentPhotoPickerResult = photoPickerResult;
    UIViewController *topVC = [self getApplicationTopViewController];
    if(@available(iOS 14, *)) {
        PHPickerConfiguration *config = [[PHPickerConfiguration alloc] init];
        config.selectionLimit = 0;
        if ([type isEqualToString:@"image/*"]) {
            config.filter = [PHPickerFilter imagesFilter];
        } else if ([type isEqualToString:@"video/*"]) {
            config.filter = [PHPickerFilter videosFilter];
        } else if ([type isEqualToString:@"*/*"]) {
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
        if ([type isEqualToString:@"image/*"]) {
            imagePickerVC.mediaTypes = @[(NSString *)kUTTypeImage];
        } else if ([type isEqualToString:@"video/*"]) {
            imagePickerVC.mediaTypes = @[(NSString *)kUTTypeMovie];
        } else if ([type isEqualToString:@"*/*"]) {
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
    for (int i = 0; i < results.count; i++) {
        PHPickerResult *result = results[i];
        [result.itemProvider loadInPlaceFileRepresentationForTypeIdentifier:UTTypeMovie.identifier completionHandler:^(NSURL * _Nullable url, BOOL isInPlace, NSError * _Nullable error) {
            if(!error){
                //处理图片资源
                BOOL fileUrlAuthozied = [url startAccessingSecurityScopedResource];
                if (fileUrlAuthozied) {
                    // 通过文件协调工具来得到新的文件地址，以此得到文件保护功能
                    NSFileCoordinator *fileCoordinator = [[NSFileCoordinator alloc] init];
                    NSError *error;
                    [fileCoordinator coordinateReadingItemAtURL:url options:0 error:&error byAccessor:^(NSURL *newURL) {
                        if (!error) {
                            NSString *urlString = [[NSString alloc] initWithFormat:@"file://%@",[url path]];
                            // NSString *urlString = [url path];
                            [uriArray addObject:urlString];
                        }
                        if (uriArray.count >= results.count) {
                            self.currentPhotoPickerResult(uriArray, 0);
                            return;
                        }
                    }];
                    [url stopAccessingSecurityScopedResource];
                } else {
                    // 授权失败
                    NSLog(@"PHPickerViewController didFinishPicking authorized fail");
                }
            }
        }];
        [result.itemProvider loadInPlaceFileRepresentationForTypeIdentifier:UTTypeImage.identifier completionHandler:^(NSURL * _Nullable url, BOOL isInPlace, NSError * _Nullable error) {
            if (!error) {
                //处理图片资源
                BOOL fileUrlAuthozied = [url startAccessingSecurityScopedResource];
                if (fileUrlAuthozied) {
                    // 通过文件协调工具来得到新的文件地址，以此得到文件保护功能
                    NSFileCoordinator *fileCoordinator = [[NSFileCoordinator alloc] init];
                    NSError *error;
                    [fileCoordinator coordinateReadingItemAtURL:url options:0 error:&error byAccessor:^(NSURL *newURL) {
                        if (!error) {
                            NSString *urlString = [[NSString alloc] initWithFormat:@"file://%@",[url path]];
                            // NSString *urlString = [url path];
                            [uriArray addObject:urlString];
                        }
                        if (uriArray.count >= results.count) {
                            self.currentPhotoPickerResult(uriArray, 0);
                            return;
                        }
                    }];
                    [url stopAccessingSecurityScopedResource];
                } else {
                    // 授权失败
                    NSLog(@"PHPickerViewController didFinishPicking authorized fail");
                }
            }
        }];
    }
}

#pragma mark UIImagePickerControllerDelegate
- (void)imagePickerController:(UIImagePickerController *)picker didFinishPickingMediaWithInfo:(NSDictionary<NSString *,id> *)info
{
    [picker dismissViewControllerAnimated:YES completion:nil];
    NSString *pickerType = (NSString*)(info[UIImagePickerControllerMediaType]);
    NSString *pickerUrlString = @"";
    if ([info[UIImagePickerControllerMediaType] isEqualToString:(NSString *)kUTTypeMovie]) {
        // 选择视频
        NSURL *mediaUrl = [info objectForKey:UIImagePickerControllerMediaURL];
        if (mediaUrl) {
            pickerUrlString = [[NSString alloc] initWithFormat:@"file://%@",[mediaUrl path]];
        }
    } else if ([info[UIImagePickerControllerMediaType] isEqualToString:(NSString *)kUTTypeImage]) {
        // 选择图片
        NSURL *imageUrl = [info objectForKey:UIImagePickerControllerImageURL];
        if (imageUrl) {
            pickerUrlString = [[NSString alloc] initWithFormat:@"file://%@",[imageUrl path]];
        }
    } else {
        // 类型错误
        NSLog(@"imagePickerController didFinishPickingMediaWithInfo type unknow");
    }
    if (pickerUrlString && ![@"" isEqualToString:pickerUrlString]) {
        self.currentPhotoPickerResult(@[pickerUrlString],0);
    }
}

- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker {
    [picker dismissViewControllerAnimated:YES completion:nil];
}

@end
