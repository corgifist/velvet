#import "window_delegate.h"
#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>
#include <Foundation/NSObjCRuntime.h>
#include "application.h"
#include <cstddef>
#include "window.h"
#import "mac_window.h"

#include <vector>
#include <any>

@interface WindowDelegate ()

- (velvet::Window *)getWindowByPlatformHandle:(NSWindow *)handle;

@end

@implementation WindowDelegate

std::vector<velvet::Window*> windows;

- (instancetype)init {
    return self;
}

- (void)addWindow:(velvet::Window *)window {
    if (!window) return;
    ((NSWindow*) window->get_platform_handle()).delegate = self;
    windows.push_back(window);
}

- (BOOL)windowShouldClose:(NSWindow *)sender {
    velvet::Window *window = [self getWindowByPlatformHandle:sender];
    if (!window) return YES;
    std::vector<std::any> responses = velvet::Application::shared()->fire_callback("should-close", window);
    for (auto &response : responses) {
        if (response.type() != typeid(bool)) continue;
        if (!std::any_cast<bool>(response)) return NO;
    }
    delete window;
    return YES;
}

- (void)windowDidBecomeKey:(NSNotification *)notification {
    MacWindow *key = (MacWindow *) [NSApp keyWindow];
    if (!key) return;
    [NSApp setMenu:key.menuBar];
    auto app = velvet::Application::shared();
    auto name = app->application_name();
    if (!name.empty()) {
        [[[NSApp menu] itemAtIndex:0] setTitle:[NSString stringWithCString:name.data() encoding:NSUTF8StringEncoding]];
    }
}

- (velvet::Window *)getWindowByPlatformHandle:(NSWindow *)handle {
    for (auto &window : windows) {
        if (window->get_platform_handle() == handle) return window;
    }
    return NULL;
}

@end