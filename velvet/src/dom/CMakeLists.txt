if(BUILD_DOM_TEXT_NODE)
    add_subdirectory(text)
endif()

if(BUILD_DOM_BODY_NODE)
    add_subdirectory(body)
endif()

if (BUILD_DOM_HTML_NODE)
    add_subdirectory(html)
endif()

set(Velvet_Sources
    ${Velvet_Sources}
    src/dom/dom.c
    src/dom/element.c
    src/dom/behavior/text.c
    PARENT_SCOPE
)