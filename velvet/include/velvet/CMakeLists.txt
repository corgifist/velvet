add_subdirectory(support)
add_subdirectory(html)
add_subdirectory(platform)
add_subdirectory(os)
add_subdirectory(graphics)
add_subdirectory(font)

set(Velvet_Headers
    include/velvet/velvet.h
    include/velvet/common.h
    ${Velvet_Headers}
    PARENT_SCOPE
)