#include "menu.h"
#include <AppKit/AppKit.h>
#include <Foundation/Foundation.h>
#include <functional>
#include <string_view>
#include <variant>

@interface InteropMenuItem : NSMenuItem

@property (nonatomic) std::optional<std::function<void()>> callback;

- (instancetype)init:(NSString *)title shortcut:(nullable NSString *)shortcut callback:(std::optional<std::function<void()>>)callback;

- (void)fire;

@end

@implementation InteropMenuItem

- (instancetype)init:(NSString *)title shortcut:(nullable NSString *)shortcut callback:(std::optional<std::function<void()>>)callback {
    self = [super initWithTitle:title action:@selector(fire:) keyEquivalent:shortcut];
    if (self) {
        self.callback = callback;
    }
    return self;
}

- (void)fire {
    if (_callback.has_value()) {
        _callback.value()();
    }
}

@end

@interface SubMenuCompatible : NSMenu

@property (nonatomic, strong) NSMenuItem *holder;

- (instancetype)init:(nonnull NSString *)title;

@end

@implementation SubMenuCompatible

- (instancetype)init:(nonnull NSString *)title {
    self = [super initWithTitle:title];
    self.holder = [[NSMenuItem alloc] init];
    [_holder setSubmenu:self];
    return self;
}

@end

namespace velvet {

MenuItem::MenuItem(std::string_view t_title, 
    std::optional<std::string_view> t_shortcut, 
    std::optional<std::function<void()>> t_callback) {
    this->m_callback = t_callback;

    this->m_data = [[InteropMenuItem alloc] 
        init:[NSString stringWithCString:t_title.data() encoding:NSUTF8StringEncoding]
        shortcut:(t_shortcut ? [NSString stringWithCString:t_shortcut->data() encoding:NSUTF8StringEncoding] : @"")
        callback:t_callback
    ];
}

void *MenuItem::get_platform_handle() {
    return m_data;
}

Menu::Menu(std::string_view t_title, std::vector<std::variant<Menu, MenuItem>> t_items) {
    this->m_data = [[SubMenuCompatible alloc] init:[NSString stringWithCString:t_title.data() encoding:NSUTF8StringEncoding]];

    for (auto &item : t_items) {
        add_item(item);
    }
}

Menu::Menu(std::vector<std::variant<Menu, MenuItem>> t_items) : Menu("", t_items) {}

void Menu::add_item(std::variant<Menu, MenuItem> t_item) {
    if (std::holds_alternative<MenuItem>(t_item)) {
        MenuItem item = std::get<MenuItem>(t_item);
        [((SubMenuCompatible *) m_data) addItem:(NSMenuItem *)item.get_platform_handle()];
    } else {
        Menu item = std::get<Menu>(t_item);
        [((SubMenuCompatible *) m_data) addItem:((SubMenuCompatible *) item.get_platform_handle()).holder];
    }
}

void *Menu::get_platform_handle() {
    return m_data;
}

} // namespace velvet