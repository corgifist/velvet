#import "window.h"
#include <AppKit/NSWindow.h>
#include "application.h"
#import "web_view_controller.h"

#import <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

#import "mac_window.h"

@implementation MacWindow

@end

namespace velvet {

Window::Window(std::string_view title, std::string_view url, Menu menu, Rect content, Flag flags) {
    NSWindowStyleMask style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable;
    if (flags & Flag::Resizable) style |= NSWindowStyleMaskResizable;
    if (flags & Flag::Minimizable) style |= NSWindowStyleMaskMiniaturizable;

    MacWindow *data = [[MacWindow alloc]
        initWithContentRect:NSMakeRect(content.x, content.y, content.w, content.h) 
        styleMask:style
        backing:NSBackingStoreBuffered 
        defer:NO
    ];
    this->m_data = data;
    this->m_userData = nullptr;

    data.menuBar = (NSMenu *) menu.get_platform_handle();

    data.webViewController = [[WebViewController alloc] init:url.data()];
    data.webViewController.view.frame = NSMakeRect(content.x, content.y, content.w, content.h);

    [data setTitle:[NSString stringWithCString:title.data() encoding:NSUTF8StringEncoding]];
    [data setContentViewController:data.webViewController];
}

Window::~Window() {
    [((MacWindow*) m_data) release];
}

void Window::focus() {
    [((MacWindow*) m_data) orderFront:nil];
}

void Window::show() {
    [((MacWindow*) m_data) makeKeyAndOrderFront:nil];
}

void Window::hide() {
    [((MacWindow*) m_data) orderOut:nil];
}

void Window::minimize() {
    [((MacWindow*) m_data) miniaturize:nil];
}

void *Window::get_platform_handle() {
    return ((MacWindow*) (this->m_data));
}

} // namespace velvet