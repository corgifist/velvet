#include "application.h"
#include <mach/policy.h>

namespace velvet {

void Application::add_callback(const std::string &t_name, ApplicationCallback t_callback) {
    m_callbacks[t_name].push_back(t_callback);
}

std::vector<std::any> Application::fire_callback(const std::string &t_name, std::any t_arg) {
    if (m_callbacks.find(t_name) == m_callbacks.end()) return {};

    std::vector<std::any> values;
    for (auto &callback : m_callbacks[t_name]) {
        values.push_back(callback(t_arg));
    }
    return values;
}

Application *Application::shared() {
    static Application s_application;
    return &s_application;
}

} // namespace velvet