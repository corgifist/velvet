#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unicode/umachine.h>

#include "velvet/html/document.h"
#include "velvet/html/tidy.h"
#include "velvet/support/api.h"
#include "velvet/support/da.h"

#include "velvet/html/parser.h"
#include "velvet/support/memory.h"
#include "velvet/support/result.h"
#include "velvet/velvet.h"

void da_test() {
    VL_DA(int) array = VL_DA_INIT(int);
    vl_da_header_t *header = VL_DA_HEADER(array);
    printf("%zu %zu %zu\n", header->capacity, header->count, header->element_size);
    VL_DA_FREE(array);

    VL_DA(int) array2 = VL_DA_INIT(int);
    for (int i = 0; i < 12; ++i) {
        *VL_DA_PUSH(array2, int) = (i + 1) * 13;
    }
    header = VL_DA_HEADER(array2);
    printf("%zu %zu %zu\n", header->capacity, header->count, header->element_size);
    for (int i = 0; i < header->count; ++i) {
        printf("%i: %i\n", i, array2[i]);
    }
    VL_DA_DELETE(array2, 0);
    VL_DA_DELETE(array2, 1);
    VL_DA_DELETE(array2, 1);
    VL_DA_DELETE(array2, 1);
    VL_DA_DELETE(array2, 1);
    printf("%zu %zu %zu\n", header->capacity, header->count, header->element_size);
    for (int i = 0; i < header->count; ++i) {
        printf("%i: %i\n", i, array2[i]);
    }
    VL_DA_FREE(array2);
}

void da_stress_test() {
    VL_DA(int) da = VL_DA_INIT(int);
    srand(time(NULL));
    for (int i = 0; i < 100; ++i) {
        *VL_DA_PUSH(da, int) = rand();
    }
    printf("the numbers are:\n");
    for (int i = 0; i < 100; i++) {
        printf("%i\n", da[i]);
    }
    printf("added 100 random numbers: %zu %zu %zu\n", VL_DA_HEADER(da)->capacity, VL_DA_HEADER(da)->count, VL_DA_HEADER(da)->element_size);
    for (int i = 0; i < 99; ++i) {
        VL_DA_DELETE(da, 0);
    }
    printf("deleted all numbers except one: %i\n", da[0]);
}

#include <unicode/utf8.h>

void icu_test() {
    const char *msg = u8"Hello, мир!";
    int i = 0;
    UChar32 c;
    size_t count = 0;
    do {
        U8_NEXT(msg, i, 0, c);
        if (c <= 0) break;
        count++;
        printf("%zu: %i (%s)\n", count, c, msg);
    } while (c > 0);
}

void lexer_test() {
    vl_html_lexer_t lexer = {0};
    const char *text = "<node name=\"Привет, \nмир!\">";
    if (vl_html_lexer_init(&lexer, text)) {
        printf("vl_html_lexer_init error\n");
        return;
    }
    printf("expect squiggles: %s\n", lexer.text);
    printf("length: %zu\n", lexer.length);

    vl_html_token_t tok = {0};
    while (!vl_html_lexer_get(&lexer, &tok)) {
        if (tok.type == VL_HTML_TOKEN_TYPE_STOP) break;
        printf("%i %zu %i %.*s\n", tok.type, tok.inline_pos, tok.text_length, tok.text_length, tok.text);
    }

    if (vl_html_lexer_deinit(&lexer)) {
        printf("failed to deinit lexer\n");
        return;
    }
    printf("deinitialized lexer\n");
}

void parser_test() {
    vl_html_parser_t parser;
    const char *input = VL_STRINGIFY(
        <div attr="Hello &amp;" attr2='World'>
            Text Node 1
            <p>Hello, Velvet! &apos;  5 &gt; 4</p>
            <checkbox selected/>
            Text Node 2
            "This is a string &amp;"
            <p>
                Testing entities: &amp; | &amps; | &quot; | &apos;
            </p>
        </div>
    );
    if (vl_html_parser_init(&parser, input)) {
        printf("failed to initalize parser: %s\n", input);
        return;
    }
    
    vl_html_node_t root = {0};
    if (vl_html_node_init(&root)) {
        printf("failed to initialize html root\n");
        return;
    }
    if (vl_html_parser_get(&parser, &root) != VL_SUCCESS) {
        printf("failed to parse root\n");
        return;
    }
    vl_html_parser_deinit(&parser);
    vl_html_node_print(&root);
}

void empty_parser_test() {
    vl_html_parser_t parser;
    const char *input = "";
    if (vl_html_parser_init(&parser, input)) {
        printf("failed to initialize parser\n");
        return;
    }
    vl_html_node_t node = {0};
    if (vl_html_node_init(&node)) {
        printf("failed to initialize node\n");
        return;
    }
    if (vl_html_parser_get(&parser, &node)) {
        printf("failed to parse root\n");
        return;
    }
    vl_html_node_print(&node);
}

void tidy_test() {
    vl_html_parser_t parser;
    const char *input = VL_STRINGIFY(
        <head>
            <title>Tidy test</title>
        </head>
        <body>
            Hello, World!
        </body>
    );
    if (vl_html_parser_init(&parser, input)) {
        printf("failed to initialize parser\n");
        return;
    }
    vl_html_node_t node = {0};
    if (vl_html_node_init(&node)) {
        printf("failed to initialize node\n");
        return;
    }
    if (vl_html_parser_get(&parser, &node)) {
        printf("failed to parse root\n");
        return;
    }
    if (vl_html_tidy_node(&node)) {
        printf("failed to tidy node\n");
        return;
    }
    vl_html_node_print(&node);
}

void document_test() {
    const char *input = VL_STRINGIFY(
        <!-- HTML4 doctype just to test the parser capabilities-->
        <!doctype HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
        <head>
            <!-- First comment -->
            <title>Hello, Velvet!</title>
            <style>
                p {
                    color: black;
                }
            </style>
        </head>     
        <body>
            Hello, World!
            <p>
                <!-- Another comment! -->
                Such a beautiful HTML Document!
            </p>
        </body>
        <div>
            <!-- So many comments... -->
            Oh no, an oopsie-whoopsie :|
            <p>
                So <!-- OMG --> close!
            </p>
        </div>
    );
    vl_html_document_t *document = vl_html_document_new(input); 

    vl_memory_print_allocations();
    vl_html_document_print(document);
    vl_html_document_free(document);
    vl_memory_print_allocations();
}

void memory_test() {
    vl_memory_print_allocations();
    int *ints = vl_malloc(sizeof(int) * 5);
    printf("-------------------\n");
    vl_memory_print_allocations();
    ints = vl_realloc(ints, sizeof(int) * 10);
    printf("-------------------\n");
    vl_memory_print_allocations();
    vl_free(ints);
    printf("-------------------\n");
    vl_memory_print_allocations();
}

#include "velvet/os/window.h"
#include "velvet/os/sleep.h"
#include "velvet/support/feature.h"

void window_test() {
    printf("universal platform: %i\n", VL_FEATURE(UNIVERSAL_PLATFORM));
    if (vl_os_window_init()) {
        printf("failed to initialize vl_os_window\n");
        return;
    }

    vl_os_window_t *window = vl_os_window_new("velvet");
    if (!window) {
        printf("failed to create vl_os_window\n");
        return;
    }

    vl_os_sleep(3000);

    vl_os_window_free(window);
}

int main(int argc, const char *argv[]) {

    // da_stress_test();
    // icu_test();
    // lexer_test();
    // parser_test();
    // empty_parser_test();
    // tidy_test();
    // document_test();
    // memory_test();
    window_test();

    return 0;
}