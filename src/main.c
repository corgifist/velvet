#include <stdio.h>
#include <unistd.h>

#include "velvet.h"

int main(int argc, const char *argv[]) {
    char cwd[256];
    getcwd(cwd, 256);
    printf("%s | %s\n", argv[0], cwd);
    printf("%0.2f\n", M_PI);
    return 0;
}