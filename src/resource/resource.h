#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace velvet {

class Resource {
public:
    std::string name;
    uint8_t *data;
    uint64_t size;
};

class Resources {
public:
    static uint8_t *get_resource_blob();
    static std::optional<Resource> get_resource_by_name();
};

} // namespace velvet

template <>
struct std::hash<velvet::Resource> {
    std::size_t operator()(const velvet::Resource &resource) {
        return std::hash<std::string>()(resource.name);
    }
};
