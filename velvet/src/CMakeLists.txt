add_subdirectory(support)
add_subdirectory(html)
add_subdirectory(os)
add_subdirectory(platform)
add_subdirectory(graphics)

set(Velvet_Sources
    ${Velvet_Sources}
    PARENT_SCOPE
)

set(Velvet_Private_Include_Dirs
    ${Velvet_Private_Include_Dirs}
    PARENT_SCOPE
)