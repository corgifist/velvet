#pragma once

#include <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

@interface WebViewController : NSViewController

@property (nonatomic, strong) WKWebView *webView;

- (instancetype)init:(const char *) url;

- (void)loadView;

- (void)viewDidLoad;

@end