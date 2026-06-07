add_subdirectory(support)
add_subdirectory(html)

set(Velvet_Headers
    include/velvet/velvet.h
    ${Velvet_Headers}
    PARENT_SCOPE
)