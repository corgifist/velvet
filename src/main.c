#include <stdio.h>
#include <unistd.h>

#include "memory.h"
#include "velvet.h"

int main(int argc, const char *argv[]) {
    char cwd[256];
    getcwd(cwd, 256);
    printf("%s | %s\n", argv[0], cwd);
    printf("%0.2f\n", M_PI);

#if VELVET_PLATFORM(WINDOWS)
    printf("win32\n");
#elif VELVET_PLATFORM(GNU_LINUX)
    printf("gnu linux\n");
#elif VELVET_PLATFORM(MAC)
    printf("mac\n");
#endif

    void *mem = VL_MALLOC(sizeof(int));
    printf("void: %i\n", *((int*) mem));
    VL_FREE(mem);

    return 0;
}