#pragma once

#include "window.h"
#import <Cocoa/Cocoa.h>

@interface WindowDelegate : NSObject<NSWindowDelegate>

- (void)addWindow:(velvet::Window *)window;

- (BOOL)windowShouldClose:(NSWindow *)sender;

@end