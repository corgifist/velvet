#pragma once

#include "window.h"
#include <unordered_map>
#include <functional>
#include <any>
#include <string_view>
#include <vector>

namespace velvet {

using ApplicationCallback = std::function<std::any(std::any)>;

class Application {
public:
    void run();

    void add_callback(const std::string &name, ApplicationCallback callback);
    std::vector<std::any> fire_callback(const std::string &name, std::any arg);

    void log(std::string_view message);

    void exit(int code);

    void add_window(Window *window);

    void set_application_name(std::string_view name);
    std::string application_name();

    static Application *shared();

private:
    Application();

    std::string m_name;

    std::unordered_map<std::string, std::vector<ApplicationCallback>> m_callbacks;

    void *m_data;
};

} // namespace velvet