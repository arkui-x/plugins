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

#import <UIKit/UIKit.h>
#import "ios_document_picker.h"

@interface ios_document_picker()<UIDocumentPickerDelegate>

@property (nonatomic, copy) CallBack selectResult;

@end

@implementation ios_document_picker

+ (instancetype)shareManager {
    static ios_document_picker *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[ios_document_picker alloc] init];
    });
    return instance;
}

- (void)selectOptions:(ios_document_select_options *)selectOptions callBack:(CallBack)selectResult {
    UIDocumentPickerViewController *picker = [[UIDocumentPickerViewController alloc] initWithDocumentTypes:selectOptions.allowedUTIs inMode:UIDocumentPickerModeOpen];
    picker.delegate = self;
    picker.modalPresentationStyle = UIModalPresentationFullScreen;
    self.selectResult = selectResult;
    picker.allowsMultipleSelection = selectOptions.allowsMultipleSelection;
    UIViewController *controller = [self getApplicationTopViewController];
    if (controller) {
        [controller presentViewController:picker animated:YES completion:nil];
    }
}

- (UIViewController * _Nullable)getApplicationTopViewController {
    UIWindow *window = [[UIApplication sharedApplication].delegate window];
    if (!window) {
        return nil;
    }
    UIViewController *viewController = window.rootViewController;
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

#pragma mark - UIDocumentPickerDelegate
 
- (void)documentPicker:(UIDocumentPickerViewController *)controller didPickDocumentsAtURLs:(NSArray<NSURL *> *)urls {
    NSMutableArray *uris = [NSMutableArray array];
    int errorCode = 0;
    for (NSURL *url in urls) {
        BOOL fileUrlAuthozied = [url startAccessingSecurityScopedResource];
        if (fileUrlAuthozied) {
            NSFileCoordinator *fileCoordinator = [[NSFileCoordinator alloc] init];
            NSError *error;
            [fileCoordinator coordinateReadingItemAtURL:url options:0 error:&error byAccessor:^(NSURL *newURL) {
                [uris addObject:newURL.absoluteString];
            }];
            if (error) {
                errorCode = (int)error.code;
            }
            [url stopAccessingSecurityScopedResource];
        } else {
            NSLog(@"authorized fail");
        }
    }
    if (self.selectResult) {
        self.selectResult(uris, errorCode);
    }
}

- (void)documentPickerWasCancelled:(UIDocumentPickerViewController *)controller {
    if (self.selectResult) {
        self.selectResult(@[], 0);
    }
}
@end
