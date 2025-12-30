#include "application_delegate.h"
#include "application.h"

using namespace velvet;

@interface ApplicationDelegate ()

@property Application *application;

@end

@implementation ApplicationDelegate

- (instancetype)init:(Application *)application {
    self.application = application;
    return self;
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    _application->fire_callback("launch", nullptr);
}

- (BOOL)applicationShouldHandleReopen:(NSApplication *)sender hasVisibleWindows:(BOOL)hasVisibleWindows {
    _application->fire_callback("relaunch", (bool) hasVisibleWindows);
    return YES;
}

@end