add_subdirectory(html)
add_subdirectory(head)
add_subdirectory(style)
add_subdirectory(div)
add_subdirectory(text)

set(Velvet_Sources
    ${Velvet_Sources}
    src/dom/dom.c
    src/dom/element.c
    PARENT_SCOPE
)