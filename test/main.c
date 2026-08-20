
#include <cglm/mat4.h>
#include <complex.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unicode/umachine.h>

#include "large_test.h"
#include "velvet/css/lexer.h"
#include "velvet/css/parser.h"
#include "velvet/css/style.h"
#include "velvet/dom/dom.h"
#include "velvet/dom/element.h"
#include "velvet/font/atlas.h"
#include "velvet/font/shaper.h"
#include "velvet/graphics/bitmap.h"
#include "velvet/graphics/brush.h"
#include "velvet/support/color.h"
#include "velvet/graphics/geometry.h"
#include "velvet/graphics/presentation.h"
#include "velvet/graphics/render.h"
#include "velvet/html/document.h"
#include "velvet/html/tidy.h"
#include "velvet/platform/context.h"
#include "velvet/support/allocator.h"
#include "velvet/support/da.h"

#include "velvet/html/parser.h"
#include "velvet/support/error_pool.h"
#include "velvet/support/memory.h"
#include "velvet/support/result.h"
#include "velvet/support/variadic.h"
#include "velvet/velvet.h"
#include "velvet/support/managed_assert.h"

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
    VL_DA(int) da = VL_DA_INIT_WITH_ALLOCATOR(int, VL_ALLOCATOR_DEFAULT());
    vl_da_header_t *header = VL_DA_HEADER(da);
    VL_ASSERT(da);
    srand(time(NULL));
    for (int i = 0; i < 100; ++i) {
        header = VL_DA_HEADER(da);
        *VL_DA_PUSH(da, int) = rand();
    }
    vl_memory_print_allocations();
    printf("the numbers are:\n");
    for (int i = 0; i < 100; i++) {
        printf("%i\n", da[i]);
    }
    printf("added 100 random numbers: %zu %zu %zu\n", VL_DA_HEADER(da)->capacity, VL_DA_HEADER(da)->count, VL_DA_HEADER(da)->element_size);
    for (int i = 0; i < 99; ++i) {
        VL_DA_DELETE(da, 0);
    }
    printf("deleted all numbers except one: %i\n", da[0]);
    VL_DA_FREE(da);
    vl_memory_print_allocations();
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

    vl_platform_context_t *ctx = vl_platform_context_new(VL_PLATFORM_CONTEXT_TYPES());

    vl_os_window_t *window = vl_os_window_new(ctx, "velvet", 640, 480);
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
    if (!present) {
        printf("failed to create vl_graphics_presentation_t\n");
        return;
    }
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
        vl_os_window_poll_events(ctx);

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
    vl_platform_context_free(ctx);
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
    vl_memory_print_allocations();
    VL_HT_FREE(int_map);
}

void large_document_test() {
    vl_error_pool_t ep = {0};
    vl_html_document_t *doc = vl_html_document_new_with_ep(s_large_test, &ep);
    if (!doc) {
        printf("failed to parse large document\n");
        vl_error_pool_dump(&ep);
        return;
    }
    vl_html_document_print(doc);
    vl_memory_print_allocations();
    vl_html_document_free(doc);
    vl_memory_print_allocations();
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
    const char *input = "<body>Alice's friend</body><footer>Bob's friend</footer>";

    vl_error_pool_t *ep = vl_error_pool_new();
    vl_html_document_t *doc = vl_html_document_new_with_ep(input, ep);
    if (!doc) {
        vl_error_pool_dump(ep);
        return;
    }
    vl_html_document_print(doc);
}

void bitmap_test() {
    uint8_t pixels[64 * 64 * 4];
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            pixels[(y * 64 * 4) + x * 4] = (uint8_t) (((float) x) / 64.0f * 255.0f);
            pixels[(y * 64 * 4) + x * 4 + 1] = (uint8_t) (((float) x) / 64.0f * 255.0f);
            pixels[(y * 64 * 4) + x * 4 + 2] = (uint8_t) (((float) x) / 64.0f * 255.0f);
            pixels[(y * 64 * 4) + x * 4 + 3] = (uint8_t) (((float) x) / 64.0f * 255.0f);
        }
    }

    uint8_t red_pixels[64 * 64 * 4];
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            red_pixels[(y * 64 * 4) + x * 4] = (uint8_t) (((float) x) / 64.0f * 255.0f);
            red_pixels[(y * 64 * 4) + x * 4 + 1] = 0;
            red_pixels[(y * 64 * 4) + x * 4 + 2] = 0;
            red_pixels[(y * 64 * 4) + x * 4 + 3] = 1;
        }
    }

    vl_platform_context_t *ctx = vl_platform_context_new(VL_PLATFORM_CONTEXT_TYPES());
    VL_ASSERT(ctx);

    vl_os_window_t *win = vl_os_window_new(ctx, "Bitmap test", 640, 480);
    VL_ASSERT(win);

    vl_graphics_render_t *render = vl_graphics_render_new(win);
    VL_ASSERT(render);

    vl_graphics_presentation_t *present = vl_graphics_presentation_new(win, render);
    VL_ASSERT(present);

    vl_graphics_bitmap_t *bitmap = vl_graphics_bitmap_new(render, 64, 64, VL_GRAPHICS_BITMAP_FORMAT_RGBA8, pixels);
    VL_ASSERT(bitmap);
    vl_graphics_brush_t *bitmap_brush = vl_graphics_brush_new_bitmap(render, bitmap);
    VL_ASSERT(bitmap_brush);
    vl_graphics_brush_bitmap_t *br = (vl_graphics_brush_bitmap_t*) bitmap_brush;
    br->filter = VL_GRAPHICS_BRUSH_BITMAP_FILTER_NEAREST;
    br->base.extend_x = VL_GRAPHICS_BRUSH_EXTEND_MIRROR;
    br->base.extend_y = VL_GRAPHICS_BRUSH_EXTEND_MIRROR;

    vl_graphics_bitmap_t *red_bitmap = vl_graphics_bitmap_new(render, 64, 64, VL_GRAPHICS_BITMAP_FORMAT_RGBA8, red_pixels);
    VL_ASSERT(red_bitmap);
    vl_graphics_brush_t *red_brush = vl_graphics_brush_new_bitmap(render, red_bitmap);
    VL_ASSERT(red_brush);

    bool close;
    while (!vl_os_window_should_close(win, &close) && !close) {
        vl_os_window_poll_events(ctx);

        vl_graphics_presentation_begin(present);
        vl_graphics_render_batch_begin(render);
            vl_graphics_render_batch_rect_colored_uv(render, VL_RECT_EX(0, 0, 640, 480), bitmap_brush, VL_QUAD_WHITE, VL_QUAD_UV_MUL(VL_QUAD_UV_DEFAULT, VL_QUAD_UV1(2)));
            vl_graphics_render_batch_rect(render, VL_RECT_EX(0, 0, 200, 200), red_brush);
        vl_graphics_render_batch_end(render);
        vl_graphics_presentation_end(present);
    }

    vl_graphics_presentation_free(present);
    vl_graphics_brush_free(bitmap_brush);
    vl_graphics_bitmap_free(bitmap);
    vl_graphics_render_free(render);
    vl_os_window_free(win);
    vl_platform_context_free(ctx);
    vl_memory_print_allocations();
}

void memory_allocator_test() {
    VL_DA(int) arr = VL_DA_INIT(int);
    for (int i = 0; i < 100; i++) {
        VL_DA_APPEND(arr, i);
    }
    for (int i = 0; i < VL_DA_LENGTH(arr); i++) {
        printf("arr[%i] = %i\n", i, arr[i]);
    }
    vl_memory_print_allocations();
}

void malloc_test() {
    int *ptrs[] = {
        VL_NEW(int), VL_NEW(int), VL_NEW(int), VL_NEW(int), VL_NEW(int)
    };
    for (int i = 0; i < VL_ARR_LEN(ptrs); i++) {
        printf("allocated %p\n", ptrs[i]);
    }
    vl_free(ptrs[0]);
    vl_free(ptrs[2]);
    vl_memory_print_allocations();
}

void html_realloc_test() {
    const char *input = VL_STRINGIFY(
        <body>
            <p>
                <p/> <p/> <p/> <p/> <p/> <p/> <p/> <p/> <p/> <p/> <p/> <p/> <p/> <p/>
            </p>
        </body>
    );
    vl_error_pool_t ep = {0};
    vl_html_document_t *doc = vl_html_document_new_with_ep(input, &ep);
    if (!doc) {
        vl_error_pool_dump(&ep);
        return;
    }
    vl_html_document_print(doc);
}

#include <memory.h>

void misalign_test() {
    VL_DA(bool) b = VL_DA_INIT_WITH_CAPACITY(bool, 5);
    *VL_DA_PUSH(b, bool) = true;
    *VL_DA_PUSH(b, bool) = false;
    *VL_DA_PUSH(b, bool) = true;
    *VL_DA_PUSH(b, bool) = false;
    for (int i = 0; i < VL_DA_LENGTH(b); i++) {
        printf("%s\n", b[i] ? "true" : "false");
    }
}

#include "font.h"
#include "velvet/font/font.h"
#include "roboto.h"

void font_test() {
    vl_platform_context_t *ctx = vl_platform_context_new(VL_PLATFORM_CONTEXT_TYPES());
    VL_ASSERT(ctx);

    vl_font_t *font1 = vl_font_new(ctx, "simple font", 16, 1, Roboto_Regular, VL_ARR_LEN(Roboto_Regular));
    VL_ASSERT(font1);
    vl_font_t *font2 = vl_font_new(ctx, "simple font 2", 16, 2, Roboto_Regular, VL_ARR_LEN(Roboto_Regular));
    VL_ASSERT(font2);
    // vl_memory_print_allocations();

    vl_font_atlas_t *atlas = vl_font_atlas_new(VL_FONT_ATLAS_FORMAT_RRRR8, 512, 512);
    // vl_memory_print_allocations();

    const char *text = "Roboto has a dual nature.\nIt has a mechanical skeleton and the forms are largely geometric.\nAt the same time, the font features friendly and open curves\nWhile some grotesks distort their letterforms to force a rigid rhythm, Roboto doesn\'t compromise,\nallowing letters to be settled into their natural width. \nThis makes for a more natural reading rhythm more commonly found in humanist and serif types.";
    // const char *text = "A font that whispers tales of unfolding events VA AV";
    // const char *text = "Hello, World!";
    // const char *text = "Kerning: VA AV";
    vl_font_rasterize_codepoints(font1, atlas, text);
    vl_font_rasterize_codepoints(font2, atlas, text);

    vl_os_window_t *win = vl_os_window_new(ctx, "Font rendering", 640, 480);
    printf("scale: %f\n", win->io.content_scale);
    vl_graphics_render_t *r = vl_graphics_render_new(win);
    vl_graphics_presentation_t *present = vl_graphics_presentation_new(win, r);

    vl_graphics_bitmap_t *bitmap = vl_graphics_bitmap_new(r, atlas->width, atlas->width, VL_GRAPHICS_BITMAP_FORMAT_RRRR8_MIPMAP, atlas->data);
    VL_ASSERT(bitmap);
    vl_graphics_brush_t *brush = vl_graphics_brush_new_bitmap(r, bitmap);
    VL_ASSERT(brush);
    // ((vl_graphics_brush_bitmap_t*) brush)->filter = VL_GRAPHICS_BRUSH_BITMAP_FILTER_NEAREST;

    bool close;
    while (!vl_os_window_should_close(win, &close) && !close) {
        vl_os_window_poll_events(ctx);
        vl_graphics_presentation_begin(present);
        vl_graphics_render_batch_begin(r); 
            vl_graphics_render_clear(r, VL_BLACK);
            bool pressed = win->io.mouse_down[VL_MOUSE_BUTTON_LEFT];
            vl_graphics_render_batch_rect(r, VL_RECT_EX(0, 0, 512, 512), brush);
            vl_font_t *font = pressed ? font1 : font2;
            float base_x = 0;
            float base_y = 200;
            const char *ptr = text;
            vl_vec2_t size = vl_font_get_text_size(font, ptr);
            if (pressed)             
                vl_graphics_render_batch_rect_colored(r, VL_RECT_EX(
                    base_x, base_y,
                    base_x + size.x, base_y + size.y
                ), NULL, VL_QUAD_GREEN);
            while (*ptr != '\0') {
                int c = *ptr++;
                vl_font_atlas_codepoint_t *code = vl_font_atlas_find_glyph_id(atlas, font, vl_font_get_glyph_id_by_codepoint(font, c));
                VL_ASSERT(code);
                if (c == '\n' || base_x + code->x1 >= 630) {
                    base_x = 0;
                    base_y += font2->newline_advance;
                    continue;
                }
                vl_graphics_render_batch_rect_colored_uv(r, VL_RECT_EX(
                    base_x + code->x1, base_y + code->y1, 
                    base_x + code->x2, base_y + code->y2), 
                brush, VL_QUAD_WHITE, code->uv);
                base_x += code->advance_x + vl_font_get_kern_advance(font, c, *ptr);
            }
        vl_graphics_render_batch_end(r);
        vl_graphics_presentation_end(present);
    }

    // vl_memory_print_allocations();
}

#include "velvet/support/math.h"

void shaper_test() {
    vl_platform_context_t *ctx = vl_platform_context_new(VL_PLATFORM_CONTEXT_DEFAULT);
    VL_ASSERT(ctx);

    vl_os_window_t *win = vl_os_window_new(ctx, "Text shaper test", 640, 480);
    VL_ASSERT(win);
    win->callback_resize = window_resize;

    vl_graphics_render_t *r = vl_graphics_render_new(win);
    VL_ASSERT(r);

    vl_graphics_presentation_t *present = vl_graphics_presentation_new(win, r);
    VL_ASSERT(present);

    vl_font_t *proggy = vl_font_new(ctx, "proggy", 16, 2, Roboto_Regular, VL_ARR_LEN(Roboto_Regular));
    VL_ASSERT(proggy);
    vl_font_atlas_t *atlas = vl_font_atlas_new(VL_FONT_ATLAS_FORMAT_RRRR8, 512, 512);
    VL_ASSERT(atlas);
    const char *text = "Roboto has a dual nature.\nIt has a mechanical skeleton and the forms are largely geometric.\nAt the same time, the font features friendly and open curves.\nWhile some grotesks distort their letterforms to force a rigid rhythm,\nRoboto doesn\'t compromise, allowing letters to be settled into their natural width.\nThis makes for a more natural reading rhythm more commonly found in humanist and serif types.";
    // const char *text = "A font that whispers tales of unfolding events VA AV";
    // const char *text = "Hello, World!";
    // const char *text = "Kerning: VA AV";
    // const char *text = "Kerning: VA AV";
    vl_font_rasterize_codepoints(proggy, atlas, text);
    vl_graphics_bitmap_t *bitmap = vl_graphics_bitmap_new(r, 512, 512, VL_GRAPHICS_BITMAP_FORMAT_RRRR8, atlas->data);
    vl_graphics_brush_t *brush = vl_graphics_brush_new_bitmap(r, bitmap);

    vl_font_shaper_t *shaper = vl_font_shaper_new(ctx);
    VL_ASSERT(shaper);
    VL_ASSERT(!vl_font_shaper_add_font(shaper, proggy));

    vl_font_shaper_run_t *run = vl_font_shaper_run_new(shaper);
    VL_ASSERT(run);

    bool close;
    while (!vl_os_window_should_close(win, &close) && !close) {
        vl_os_window_poll_events(ctx);
        vl_graphics_presentation_begin(present);
        vl_graphics_render_batch_begin(r);
            // const char *text = "hello, World! VA AV";
            vl_graphics_render_clear(r, VL_BLACK);
            float base_x = 0;
            float base_y = 0;
            const char *ptr = text;
            vl_font_shaper_run_reset(run);
            vl_font_shaper_process(shaper, ptr, strlen(ptr));
            while (vl_font_shaper_shape(shaper, run)) {
                if (run->newline) {
                    base_y += proggy->newline_advance;
                    base_x = 0;
                }
                vl_font_shaper_glyph_t glyph = {0};
                while (vl_font_shaper_iterate(run, &glyph)) {
                    vl_font_atlas_codepoint_t *code = vl_font_atlas_find_glyph_id(atlas, proggy, glyph.id);
                    VL_ASSERT(code);
                    if (glyph.codepoint == 10) continue;
                    float lx = base_x + glyph.x + code->x1;
                    float ly = base_y - glyph.y + code->y1;
                    if (code->glyph_id != 10) {
                        vl_graphics_render_batch_rect_colored_uv(r, VL_RECT_EX(
                            lx, ly,
                            lx + code->w, ly + code->h
                        ), brush, VL_QUAD_WHITE, code->uv);
                    }
                    // printf("base: %i %i\b; glyph: %i %i; advance: %i %i; char: %c\n", base_x, base_y, glyph.x, glyph.y, glyph.advance_x, glyph.advance_y, (char) glyph.codepoint);
                    base_x += glyph.advance_x;
                    base_y += glyph.advance_y;
                }
            }
        vl_graphics_render_batch_end(r);
        vl_graphics_presentation_end(present);
    }
}

#include "notosans_arabic.h"
#include "rubik.h"

void arabic_test() {
    vl_platform_context_t *ctx = vl_platform_context_new(VL_PLATFORM_CONTEXT_DEFAULT);
    vl_os_window_t *win = vl_os_window_new(ctx, "Arabic test", 640, 480);
    vl_graphics_render_t *r = vl_graphics_render_new(win);
    vl_graphics_presentation_t *present = vl_graphics_presentation_new(win, r);

    vl_font_t *arabic = vl_font_new(ctx, "arabic", 72, 2, NotoSansArabic_Regular, VL_ARR_LEN(NotoSansArabic_Regular));
    vl_font_atlas_t *atlas = vl_font_atlas_new(VL_FONT_ATLAS_FORMAT_RRRR8, 512, 512);
    const char *text = u8"لمّا كان الاعتراف بالكرامة المتأصلة في جميع";
    // const char *text = u8"Hello";
    // const char *text = u8"لمّا";
    // const char *text = u8"Préférer";
    // const char *text = u8"كان";
    // const char *text = u8"لد";
    // const char *text = u8"بسم الله الرحمن الرحيم";
    vl_font_shaper_t *shaper = vl_font_shaper_new(ctx);
    vl_font_shaper_add_font(shaper, arabic);
    vl_font_shaper_run_t *run = vl_font_shaper_run_new(shaper);
    vl_font_shaper_process(shaper, text, strlen(text));
    while (vl_font_shaper_shape(shaper, run)) {
        vl_font_shaper_glyph_t glyph = {0};
        while (vl_font_shaper_iterate(run, &glyph)) {
            if (!vl_font_atlas_find_glyph_id(atlas, arabic, glyph.id)) {
                vl_font_rasterize_glyph_id(arabic, atlas, glyph.id);
            }
        }
    }
    vl_font_shaper_run_reset(run);
    vl_graphics_bitmap_t *bitmap = vl_graphics_bitmap_new(r, atlas->width, atlas->height, VL_GRAPHICS_BITMAP_FORMAT_RRRR8, atlas->data);
    vl_graphics_brush_t *brush = vl_graphics_brush_new_bitmap(r, bitmap);
    // ((vl_graphics_brush_bitmap_t*) brush)->filter = VL_GRAPHICS_BRUSH_BITMAP_FILTER_NEAREST;

    bool close;
    while (!vl_os_window_should_close(win, &close) && !close) {
        vl_os_window_poll_events(ctx);
        vl_graphics_presentation_begin(present);
        vl_graphics_render_clear(r, VL_BLACK);
        vl_graphics_render_batch_begin(r);
        vl_graphics_render_batch_rect(r, VL_RECT_EX(
            0, 0, 512, 512
        ), brush);
        float base_x = 0;
        float base_y = 300;
        vl_font_shaper_process(shaper, text, strlen(text));
        int index = 0;
        VL_DA(vl_font_shaper_glyph_t) glyphs = VL_DA_INIT(vl_font_shaper_glyph_t);
        while (vl_font_shaper_shape(shaper, run)) {
            vl_font_shaper_glyph_t glyph = {0};
            while (vl_font_shaper_iterate(run, &glyph)) {
                VL_DA_APPEND(glyphs, glyph);
                // base_y += glyph.advance_y;
            }
        }
        int len = VL_DA_LENGTH(glyphs);
        for (int i = 0; i < len; i++) {
            vl_font_shaper_glyph_t glyph = glyphs[i];
            vl_font_atlas_codepoint_t *code = vl_font_atlas_find_glyph_id(atlas, arabic, glyph.id);
            vl_quad_uv_t uv = code->uv;
            float lx = base_x + glyph.x + code->x1;
            float ly = base_y - glyph.y + code->y1;
            vl_graphics_render_batch_rect_colored_uv(r, VL_RECT_EX(
                lx, ly,
                lx + code->w, ly + code->h
            ), brush, VL_QUAD_WHITE, uv);
            vl_graphics_render_batch_rect_colored_uv(r, VL_RECT_EX(
                lx, ly + 100,
                lx + code->w, ly + code->h + 100
            ), brush, VL_QUAD_BLACK, uv);
            base_x += glyph.advance_x;
            base_y += glyph.advance_y;
        }
        vl_font_shaper_run_reset(run);
        vl_graphics_render_batch_end(r);
        vl_graphics_presentation_end(present);
    }
}

#include "velvet/web/web.h"
#include <cglm/affine.h>

void simple_dom_test() {
    vl_platform_context_t *ctx = vl_platform_context_new(VL_PLATFORM_CONTEXT_DEFAULT);
    const char *input = "Hello, World!\nHello, Velvet! لمّا كان الاعتراف بالكرامة المتأصلة في جميع";
    vl_html_document_t *doc = vl_html_document_new(input);
    vl_html_document_print(doc);
    VL_ASSERT(doc);
    vl_web_t web = {0};
    vl_web_init(ctx, &web, doc);

    vl_os_window_t *win = vl_os_window_new(ctx, "Web test", 640, 480);
    vl_graphics_render_t *render = vl_graphics_render_new(win);
    vl_graphics_presentation_t *present = vl_graphics_presentation_new(win, render);
    web.render = render;

    bool close;
    while (!vl_os_window_should_close(win, &close) && !close) {
        vl_os_window_poll_events(ctx);
        vl_graphics_presentation_begin(present);
        vl_graphics_render_clear(render, VL_BLACK);
        vl_graphics_render_batch_begin(render);
            // vl_graphics_render_push_translate(render, VL_VEC2(50, 50));
            static float angle = 0;
            angle += 3;
            vl_graphics_render_push_rotate(render, angle);
            vl_web_render(&web, NULL);
            vl_graphics_render_clear_transform(render);
            vl_graphics_render_batch_point(render, VL_POINT(50, 50), 5, VL_RED);
         vl_graphics_render_batch_end(render);
        vl_graphics_presentation_end(present);
    }
    vl_web_deinit(&web);
    vl_html_document_free(doc);
    vl_graphics_presentation_free(present);
    vl_graphics_render_free(render);
    vl_os_window_free(win);
    vl_platform_context_free(ctx);
    vl_memory_print_allocations();
}

#include "velvet/font/segmentation.h"

void segmentation_test() {
    const char *text = "Hello, world! こんにちは世界";
    vl_font_segmentation_breaks_t breaks = NULL;
    vl_font_segmentation_process_string(text, strlen(text), VL_FONT_SEGMENTATION_LINE, &breaks);
    for (int i = 0; i < VL_DA_LENGTH(breaks); i++) {
        vl_font_segmentation_break_t *br = breaks + i;
        printf("%i %.*s %zu %zu\n", i, (int) br->end - (int) br->begin, text + br->begin, br->begin, br->end);
    }
}

void css_test() {
    vl_css_lexer_t lexer = {0};
    const char *text = "html { padding: 16px; margin-left: 8px; color: red; outline-color: rgba(128 128 128 0.6); padding: 8px }";
    vl_css_parser_t parser = {0};
    vl_error_pool_t ep = {0};
    vl_css_parser_init(&parser, text, VL_SOURCE_LOCATION_HERE, &ep);
    vl_css_class_t class = {0};
    VL_ASSERT(!vl_css_parser_get(&parser, &class));
    vl_css_class_print(&class);
    vl_error_pool_dump(&ep);
}

void styling_test() {
    vl_platform_context_t *ctx = vl_platform_context_new(VL_PLATFORM_CONTEXT_DEFAULT);
    const char *input = VL_STRINGIFY(
        <style>
            body {
                color: aqua;
            }
            p {
                color: red;
                background-color: brown;
            }
            .green {
                background-color: salmon;
                color: green;
            }
            .blue {
                background-color: snow;
                color: blue;
            }
        </style>
        <body>
            Aqua
            <p>Red</p>
            <p class="green">Green</p>
            <p class="blue">Blue</p>
        </body>
    );
    vl_html_document_t *doc = vl_html_document_new(input);
    vl_html_document_print(doc);
    VL_ASSERT(doc);
    vl_web_t web = {0};
    vl_web_init(ctx, &web, doc);

    vl_os_window_t *win = vl_os_window_new(ctx, "Web test", 640, 480);
    vl_graphics_render_t *render = vl_graphics_render_new(win);
    vl_graphics_presentation_t *present = vl_graphics_presentation_new(win, render);
    web.render = render;

    bool close;
    while (!vl_os_window_should_close(win, &close) && !close) {
        vl_os_window_poll_events(ctx);
        vl_graphics_presentation_begin(present);
        vl_graphics_render_clear(render, VL_BLACK);
        vl_graphics_render_batch_begin(render);
            vl_web_render(&web, NULL);
         vl_graphics_render_batch_end(render);
        vl_graphics_presentation_end(present);
    }
    vl_web_deinit(&web);
    vl_html_document_free(doc);
    vl_graphics_presentation_free(present);
    vl_graphics_render_free(render);
    vl_os_window_free(win);
    vl_platform_context_free(ctx);
    vl_memory_print_allocations();
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
    // window_test();
    // document_tidy_test();
    // ht_test();
    // large_document_test();
    // error_pool_test();
    // parser_quote_test();
    // bitmap_test();
    // memory_allocator_test();
    // malloc_test();
    // html_realloc_test();
    // misalign_test();
    // font_test();
    // shaper_test();
    // arabic_test();
    // printf("feature: %i\n", VL_FEATURE(DOM_TEXT_NODE));
    // simple_dom_test();
    // segmentation_test();
    // css_test();
    styling_test();

    return 0;
}