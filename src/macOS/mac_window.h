#import <AppKit/AppKit.h>
#import "web_view_controller.h"

@interface MacWindow : NSWindow

@property (nonatomic, strong) WebViewController *webViewController;
@property (nonatomic, strong) NSMenu *menuBar;

@end