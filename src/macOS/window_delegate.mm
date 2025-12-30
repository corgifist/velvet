#import "window_delegate.h"
#include "application.h"
#include <cstddef>
#include "window.h"

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

- (velvet::Window *)getWindowByPlatformHandle:(NSWindow *)handle {
    for (auto &window : windows) {
        if (window->get_platform_handle() == handle) return window;
    }
    return NULL;
}

@end