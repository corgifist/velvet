if(BUILD_DOM_TEXT_NODE)
    add_subdirectory(text)
endif()

set(Velvet_Sources
    ${Velvet_Sources}
    src/dom/dom.c
    src/dom/element.c
    PARENT_SCOPE
)