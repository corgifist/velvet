
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unicode/umachine.h>

#include "velvet/graphics/brush.h"
#include "velvet/graphics/color.h"
#include "velvet/graphics/geometry.h"
#include "velvet/graphics/presentation.h"
#include "velvet/graphics/render.h"
#include "velvet/html/document.h"
#include "velvet/html/tidy.h"
#include "velvet/support/da.h"

#include "velvet/html/parser.h"
#include "velvet/support/error_pool.h"
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
        <div attr="Hello &quot; &apos; but not &quots; & quot; & quot ; &amp ;" attr2='World'>
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
    vl_error_pool_t ep = {0};
    vl_html_document_t *document = vl_html_document_new_with_ep(input, &ep);
    if (!document) {
        printf("failed to parse document\n");
        vl_error_pool_dump(&ep);
        return;
    }
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

void window_resize (vl_os_window_t *window, int w, int h) {
    for (int i = 0; i < VL_DA_LENGTH(window->owned_renders); i++) {
        printf("%i %i\n", w, h);
        vl_graphics_render_resize(window->owned_renders[i], w, h);
    }
}

void window_test() {
    printf("universal platform: %i\n", VL_FEATURE(UNIVERSAL_PLATFORM));
    if (vl_os_window_init()) {
        printf("failed to initialize vl_os_window\n");
        return;
    }

    vl_os_window_t *window = vl_os_window_new("velvet", 640, 480);
    if (!window) {
        printf("failed to create vl_os_window\n");
        return;
    }
    window->callback_resize = window_resize;
    vl_graphics_render_t *render = vl_graphics_render_new(window);
    if (!render) {
        printf("failed to create vl_graphics_render_t\n");
        return;
    }
    printf("graphics vendor: %s\n", render->backend);

    vl_graphics_presentation_t *present = vl_graphics_presentation_new(window, render);
    vl_graphics_brush_t *white = vl_graphics_brush_new_solid(render, VL_COLOR(1, 1, 1, 1));
    vl_graphics_brush_t *green = vl_graphics_brush_new_solid(render, VL_COLOR(0, 1, 0, 1));
    vl_graphics_brush_t *blue = vl_graphics_brush_new_solid(render, VL_COLOR(0, 0, 1, 1));

    vl_gradient_stop_t stops[] = {
        VL_GRADIENT_STOP(0.2, VL_RED),
        VL_GRADIENT_STOP(0.5, VL_GREEN),
        VL_GRADIENT_STOP(0.8, VL_BLUE)
    };
    vl_graphics_brush_t *gradient = vl_graphics_brush_new_linear_gradient(render, stops, sizeof(stops) / sizeof(*stops));

    static vl_point_t quad[] = {
        VL_POINT(0, 0),
        VL_POINT(300, 200),
        VL_POINT(400, 300),
        VL_POINT(200, 400)
    };

    bool close;
    float angle = 0;
    while (!vl_os_window_should_close(window, &close) && !close) {
        vl_os_window_poll_events();

        vl_graphics_presentation_begin(present);
        vl_graphics_render_clear(render, VL_BLACK);
        vl_graphics_render_batch_begin(render);
            // vl_graphics_render_batch_rect(render, VL_RECT_EX(0, 0, 640, 100), green);
            // vl_graphics_render_batch_rect(render, VL_RECT_EX(0, 480, 400, 400), green);
            // vl_graphics_render_batch_rect(render, VL_RECT_EX(200, 200, 250, 250), blue);
            vl_graphics_render_batch_quad_colored(render, VL_QUAD(
                 quad[0], quad[1], quad[2], quad[3]
            ), NULL, VL_QUAD_COLORS(
                VL_RED, VL_GREEN, VL_BLUE, VL_WHITE
            ));
            vl_graphics_render_batch_rect_colored(render, VL_RECT(
                VL_POINT(410, 200),
                VL_POINT(610, 400)
            ), NULL, VL_QUAD_COLORS(
                VL_RED, VL_GREEN, 
                VL_BLUE, VL_WHITE
            ));
            vl_graphics_render_batch_rect(render, VL_RECT_EX(
                300, 300, 500, 500
            ), gradient);
            static int moving_quad = -1;
            if (!window->io.mouse_down[VL_MOUSE_BUTTON_LEFT]) {
                moving_quad = -1;
            }
            for (int i = 0; i < sizeof(quad) / sizeof(*quad); i++) {
                vl_rect_t rect = VL_RECT(
                    VL_POINT(quad[i].x - 5, quad[i].y - 5), VL_POINT(quad[i].x + 5, quad[i].y + 5)
                );
                vl_graphics_brush_t *brush = white;
                if (VL_POINT_IN_RECT(window->io.cursor, rect)) {
                    brush = green;
                    if (window->io.mouse_down[VL_MOUSE_BUTTON_LEFT] && moving_quad == -1) {
                        moving_quad = i;
                    }
                }
                vl_graphics_render_batch_rect(render, rect, brush);
            }
            if (moving_quad != -1) {
                quad[moving_quad] = window->io.cursor;
            }
            vl_graphics_render_batch_point(render, VL_POINT(500, 100), sinf(angle) * 6, VL_RED);
        vl_graphics_render_batch_end(render);
        vl_graphics_presentation_end(present);
        angle += 0.01;
        vl_graphics_brush_linear_gradient_t *l = (vl_graphics_brush_linear_gradient_t*) gradient;
        l->start = VL_POINT(cosf(angle) / 2, sinf(angle) / 2);
        l->end = VL_POINT_SUB(VL_POINT(1, 1), l->start);
    }

    vl_graphics_brush_free(blue);
    vl_graphics_brush_free(green);
    vl_graphics_brush_free(white);
    vl_graphics_brush_free(gradient);
    vl_graphics_presentation_free(present);
    vl_graphics_render_free(render);
    vl_os_window_free(window);
    vl_memory_print_allocations();
}

void document_tidy_test() {
    vl_html_document_t *document = vl_html_document_new(VL_STRINGIFY(
        <body>

        </body>
    ));
    if (vl_html_tidy_document(document)) {
        printf("vl_html_tidy_document error");
        return;
    }
    vl_html_document_print(document);
}

#include "velvet/support/ht.h"

void ht_test() {
    VL_HT(int, int) int_map = VL_HT_NEW(int, int);
    vl_memory_print_allocations();
    for (int i = 0; i < 100; i++) {
        VL_HT_PUSH(int_map, int, i, int, i * 100);
    }
    vl_ht_entry_t it = VL_HT_ENTRY();
    int index = 0;
    while (vl_ht_iterate(int_map, &it)) {
        printf("%i %zu %zu %i %i\n", index++, it.__index, it.hash, *((int*) it.key), *((int*) it.value));
    }
    VL_HT_FREE(int_map);
}

void large_document_test() {
    FILE *f = fopen("test/large_test.html", "rb");
    printf("f: %p\n", f);
    fseek(f, 0, SEEK_END);
    size_t len = ftell(f);
    fseek(f, -len, SEEK_END);
    char *content = vl_malloc(len + 1);
    int c;
    int i = 0;
    while ((c = fgetc(f)) > 0) {
        content[i++] = (char) c;
    }
    content[len] = '\0';
    printf("%.*s...\n", 32, content);
    vl_error_pool_t ep = {0};
    vl_html_document_t *doc = vl_html_document_new_with_ep(content, &ep);
    if (!doc) {
        printf("failed to parse large document\n");
        vl_error_pool_dump(&ep);
        return;
    }
    vl_html_document_print(doc);
}

void error_pool_test() {
    vl_error_pool_t ep = VL_ERROR_POOL();
    vl_error_pool_append(&ep, 1, 1, "Hello, PI World! %0.2f", 3.14f);
    vl_error_pool_append(&ep, 17, 32, "unexpected symbol '%c'", 'A');

    vl_error_t err = VL_ERROR();
    while (vl_error_pool_iterate(&ep, &err)) {
        printf("%zu:%zu: %s\n", err.line, err.pos, err.message);
    }
    vl_memory_print_allocations();
    vl_error_pool_deinit(&ep);
    vl_memory_print_allocations();
}

void parser_quote_test() {
    const char *input = VL_STRINGIFY(
        <body>
            Alice's friend
        </body>
        <footer>
            Bob's friend
        </footer>
    );

    vl_error_pool_t *ep = vl_error_pool_new();
    vl_html_document_t *doc = vl_html_document_new_with_ep(input, ep);
    if (!doc) {
        vl_error_pool_dump(ep);
        return;
    }
    vl_html_document_print(doc);
}

int main(int argc, const char *argv[]) {

    // da_stress_test();
    // icu_test();
    // lexer_test();
    parser_test();
    // empty_parser_test();
    // tidy_test();
    // document_test();
    // memory_test();
    // window_test();
    // document_tidy_test();
    // ht_test();
    // large_document_test();
    // error_pool_test();
    // parser_quote_test();


    return 0;
}