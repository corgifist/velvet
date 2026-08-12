#ifndef VELVET_SUPPORT_VARIADIC_H
#define VELVET_SUPPORT_VARIADIC_H

#define VL_VA_NARGS_(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, \
                     A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, \
                     A21, A22, A23, A24, A25, A26, A27, A28, A29, A30, \
                     A31, A32, COUNT, ...) COUNT
/**
 * Keep in mind that 32 is the limit of arguments for VL_VA_NARGS
 * Passing more that 32 arguments may yield unexpected and unwanted results
 */
#define VL_VA_NARGS(...) VL_VA_NARGS_(__VA_ARGS__, \
    32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
    19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
    9, 8, 7, 6, 5, 4, 3, 2, 1)

#define VL_VA_CONCAT_(A, B) A##B
#define VL_VA_CONCAT(A, B) VL_VA_CONCAT_(A, B)

#define VL_VA_DISPATCH(HANDLER, ...) VL_VA_CONCAT(HANDLER,VL_VA_NARGS(__VA_ARGS__))(__VA_ARGS__)

#endif // VELVET_SUPPORT_VARIADIC_H