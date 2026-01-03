if (APPLE)
    add_subdirectory(macOS)
endif()

set(Velvet_Sources
    ${Velvet_Sources}
    definitions.cpp
    application.cpp
    window.cpp
)

set(Velvet_Headers
    ${Velvet_Headers}
    definitions.h
    application.h
    window.h
    menu.h
    velvet.h
)

add_library(velvet STATIC ${Velvet_Sources} ${Velvet_Headers})
set_target_properties(velvet PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
target_include_directories(velvet PUBLIC .)

if (APPLE)
    target_link_libraries(velvet
        PUBLIC "-framework Foundation"
        PUBLIC "-framework AppKit"
        PUBLIC "-framework WebKit"
    )
endif()