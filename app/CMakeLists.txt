add_executable(Velvet_App)
target_sources(Velvet_App
    PRIVATE
    main.cpp
)

set_target_properties(Velvet_App PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)