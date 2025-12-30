#import "window.h"
#include <AppKit/NSWindow.h>
#include "application.h"
#import "web_view_controller.h"

#import <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

@interface MacWindow : NSObject

@property (nonatomic, strong) NSWindow *window;
@property (nonatomic, strong) WebViewController *webViewController;

@end

@implementation MacWindow

@end

namespace velvet {

Window::Window(std::string_view title, std::string_view url, Rect content, Flag flags) {
    MacWindow *data = [[MacWindow alloc] init];
    this->m_data = data;
    this->m_userData = nullptr;

    NSWindowStyleMask style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable;
    if (flags & Flag::Resizable) style |= NSWindowStyleMaskResizable;
    if (flags & Flag::Minimizable) style |= NSWindowStyleMaskMiniaturizable;

    data.window = [[NSWindow alloc]
        initWithContentRect:NSMakeRect(content.x, content.y, content.w, content.h) 
        styleMask:style
        backing:NSBackingStoreBuffered 
        defer:NO
    ];

    data.webViewController = [[WebViewController alloc] init:url.data()];
    data.webViewController.view.frame = NSMakeRect(content.x, content.y, content.w, content.h);

    [data.window setTitle:[NSString stringWithCString:title.data() encoding:NSUTF8StringEncoding]];
    [data.window setContentViewController:data.webViewController];
    show();
}

Window::~Window() {
    [((MacWindow*) m_data) release];
}

void Window::focus() {
    [((MacWindow*) m_data).window orderFront:nil];
}

void Window::show() {
    [NSApp activateIgnoringOtherApps:YES];
    [((MacWindow*) m_data).window makeKeyAndOrderFront:nil];
}

void Window::hide() {
    [((MacWindow*) (this->m_data)).window orderOut:nil];
}

void Window::minimize() {
    [((MacWindow*) m_data).window miniaturize:nil];
}

void *Window::get_platform_handle() {
    return ((MacWindow*) (this->m_data)).window;
}

} // namespace velvet