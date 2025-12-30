#pragma once

#include "geometry.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace velvet {

class Window {
public:
    friend class WindowBuilder;

    ~Window();

    void focus();

    void show();
    void hide();

    void minimize();

    // returns NSWindow* on macOS
    void *get_platform_handle();

    // you can do anything you want with this pointer
    void *m_userData;
    
private:
    enum class Flag : uint32_t {
        None = 0,
        Resizable = 1 << 0,
        Minimizable = 1 << 1
    };

    friend inline bool operator&(Window::Flag lhs, Window::Flag rhs) {
        return static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs);
    }

    friend inline Window::Flag operator&=(Window::Flag &lhs, Window::Flag rhs) {
        return lhs = static_cast<Window::Flag>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
    }

    friend inline Window::Flag operator|=(Window::Flag &lhs, Window::Flag rhs) {
        return lhs = static_cast<Window::Flag>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    }

    Window(std::string_view title, std::string_view url, Rect content, Flag flags);

    void *m_data;
};


class WindowBuilder {
public:
    WindowBuilder() : m_title("Velvet"), m_url("https://google.com"), m_content(0, 0, 640, 480), m_resizable(true) {}

    WindowBuilder &title(std::string_view title);
    std::string title();

    WindowBuilder &url(std::string_view url);
    std::string url();

    WindowBuilder &content_rect(Rect rect);
    Rect content_rect();

    WindowBuilder &resizable(bool resizable);
    bool resizable();

    WindowBuilder &minimizable(bool minimizable);
    bool minimizable();

    velvet::Window *build();

private:
    std::string m_title, m_url;
    Rect m_content;
    bool m_resizable, m_minimizable;
};


} // namespace velvet