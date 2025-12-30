#include "application.h"
#include "application_delegate.h"
#include "macOS/window_delegate.h"

#include <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#include <objc/NSObject.h>
#import <Cocoa/Cocoa.h>

@interface MacApplication : NSObject

@property (nonatomic, strong) ApplicationDelegate *applicationDelegate;
@property (nonatomic, strong) WindowDelegate *windowDelegate;

@end

@implementation MacApplication

@end

namespace velvet {

Application::Application() {
    NSApplication *app = [NSApplication sharedApplication];

    MacApplication *data = [[[MacApplication alloc] init] retain];
    this->m_data = data;

    data.applicationDelegate = [[ApplicationDelegate alloc] init:this];
    data.windowDelegate = [[WindowDelegate alloc] init];
    app.delegate = data.applicationDelegate;
}

void Application::run() {
    [[NSApplication sharedApplication] run];
}

void Application::exit(int code) {
    [NSApp terminate:nil];
}

void Application::add_window(Window *window) {
    [((MacApplication*) m_data).windowDelegate addWindow:window];
}

void Application::log(std::string_view message) {
    NSLog(@"%s", message.data());
}

} // namespace velvet