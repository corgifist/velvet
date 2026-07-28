set(Velvet_Sources
    ${Velvet_Sources}
    src/platform/universal/window.c
    src/platform/universal/render.c
    src/platform/universal/presentation.c
    src/platform/universal/brush.c
    src/platform/universal/glad/src/gl.c
    src/platform/universal/sleep.c
    src/platform/universal/bitmap.c
    src/platform/universal/gl_check.c
    PARENT_SCOPE
)

set(Velvet_Headers
    ${Velvet_Headers}
    src/platform/universal/gl_check.h
    src/platform/universal/glad/include/glad/gl.h
    src/platform/universal/glad/include/KHR/khrplatform.h
    PARENT_SCOPE
)

set(Velvet_Private_Include_Dirs
    ${Velvet_Private_Include_Dirs}
    src/platform/universal/glad/include
    PARENT_SCOPE
)