#include "window.h"
#include "application.h"

namespace velvet {

velvet::Window *WindowBuilder::build() {
    velvet::Window::Flag flags = Window::Flag::None;

    if (m_resizable) flags |= Window::Flag::Resizable;
    if (m_minimizable) flags |= Window::Flag::Minimizable;

    velvet::Window *window = new velvet::Window(m_title, m_url, m_content, flags);
    return window;
}

WindowBuilder &WindowBuilder::title(std::string_view t_title) {
    this->m_title = t_title;
    return *this;
}

std::string WindowBuilder::title() {
    return m_title;
}

WindowBuilder &WindowBuilder::url(std::string_view t_url) {
    this->m_url = t_url;
    return *this;
}

std::string WindowBuilder::url() {
    return m_url;
}

WindowBuilder &WindowBuilder::resizable(bool t_resizable) {
    this->m_resizable = t_resizable;
    return *this;
}

bool WindowBuilder::resizable() {
    return m_resizable;
}

WindowBuilder &WindowBuilder::minimizable(bool t_minimizable) {
    m_minimizable = t_minimizable;
    return *this;
}

bool WindowBuilder::minimizable() {
    return m_minimizable;
}

} // namespace velvet