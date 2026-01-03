#pragma once

#include <functional>
#include <optional>
#include <string_view>
#include <variant>
namespace velvet {

class MenuItem {
public:
    MenuItem(std::string_view name, 
        std::optional<std::string_view> shortcut = std::nullopt, 
        std::optional<std::function<void()>> callback = std::nullopt);

    /*
        void *MenuItem::get_platform_handle();

        returns NSMenuItem* on macOS etc.
    */
    void *get_platform_handle();

private:
    std::optional<std::function<void()>> m_callback;
    void *m_data;
};

class Menu {
public:
    Menu(std::string_view title, std::vector<std::variant<Menu, MenuItem>> items = {});
    Menu(std::vector<std::variant<Menu, MenuItem>> items = {});

    void add_item(std::variant<Menu, MenuItem> item);

    /*
        void *Menu::get_platform_handle();

        returns NSMenuItem* on macOS etc.
    */
    void *get_platform_handle();

private:
    void *m_data;
};

} // namespace velvets