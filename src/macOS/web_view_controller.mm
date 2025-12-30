#import "web_view_controller.h"
#include "definitions.h"
#include "window.h"

#include <CoreGraphics/CGGeometry.h>
#include <Foundation/Foundation.h>
#include <WebKit/WebKit.h>

@interface WebViewController ()

@property (nonatomic, strong) NSString *url;

@end

@implementation WebViewController

- (instancetype)init:(const char *) url {
    self = [super init];
    if (self) {
        self.url = [NSString stringWithCString:velvet::construct_url(url).c_str() encoding:NSUTF8StringEncoding];
    }

    return self;
}

- (void)loadView {
    WKWebViewConfiguration *config = [[WKWebViewConfiguration alloc] init];

    [config.preferences setValue:[NSNumber numberWithBool:YES] forKey:@"fullScreenEnabled"];

    self.webView = [[WKWebView alloc] initWithFrame:CGRectMake(0, 0, 800, 600) configuration:config];

    self.view = _webView;
}

- (void)viewDidLoad {
    [_webView loadRequest:[[NSURLRequest alloc] initWithURL:[[NSURL alloc] initWithString:_url]]];
}

@end