#pragma once

#import <Cocoa/Cocoa.h>

#include "application.h"

@interface ApplicationDelegate : NSObject<NSApplicationDelegate>

- (instancetype)init:(velvet::Application *)application;

- (void)applicationDidFinishLaunching:(NSNotification *)notification;
- (BOOL)applicationShouldHandleReopen:(NSApplication *)sender hasVisibleWindows:(BOOL)hasVisibleWindows;

@end