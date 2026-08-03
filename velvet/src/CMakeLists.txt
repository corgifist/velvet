add_subdirectory(support)
add_subdirectory(html)
add_subdirectory(os)
add_subdirectory(platform)
add_subdirectory(graphics)
add_subdirectory(font)
add_subdirectory(dom)
add_subdirectory(web)

set(Velvet_Sources
    ${Velvet_Sources}
    PARENT_SCOPE
)

set(Velvet_Private_Include_Dirs
    ${Velvet_Private_Include_Dirs}
    PARENT_SCOPE
)