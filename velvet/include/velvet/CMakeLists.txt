add_subdirectory(support)
add_subdirectory(html)
add_subdirectory(platform)
add_subdirectory(os)

set(Velvet_Headers
    include/velvet/velvet.h
    ${Velvet_Headers}
    PARENT_SCOPE
)