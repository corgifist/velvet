add_executable(velvet_app)
target_sources(velvet_app
    PRIVATE
    main.c
)

target_link_libraries(velvet_app PRIVATE velvet)

set_target_properties(velvet_app PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)