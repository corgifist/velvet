add_subdirectory(glad)

set(Velvet_Sources
    ${Velvet_Sources}
    src/platform/universal/window.c
    src/platform/universal/render.c
    src/platform/universal/presentation.c
    PARENT_SCOPE
)

set(Velvet_Private_Include_Dirs
    ${Velvet_Private_Include_Dirs}
    PARENT_SCOPE
)