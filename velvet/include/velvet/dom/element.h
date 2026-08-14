#ifndef VL_DOM_ELEMENT_H
#define VL_DOM_ELEMENT_H

#include "velvet/css/style.h"
#include "velvet/support/memory.h"
#include "velvet/support/api.h"
#include "velvet/support/result.h"
#include "velvet/graphics/geometry.h"
#include "velvet/support/da.h"
#include "velvet/dom/render.h"

enum vl_dom_element_metric_type {
    // for vl_dom_element_get_metric2
    VL_DOM_ELEMENT_METRIC_SIZE
};

typedef enum vl_dom_element_metric_type vl_dom_element_metric_type_t;

enum vl_dom_element_property_type {
    VL_DOM_ELEMENT_PROPERTY_STRING
};

typedef enum vl_dom_element_property_type vl_dom_element_property_type_t;

struct vl_dom_element;
typedef vl_result_t (*vl_dom_element_render_func)(struct vl_dom_element *element, vl_dom_render_opts_t *opts);
typedef vl_result_t (*vl_dom_element_set_property_func)(struct vl_dom_element *element, const char *property, vl_dom_element_property_type_t type, const void *value);
typedef vl_vec2_t (*vl_dom_element_get_metric2_func)(struct vl_dom_element *element, vl_dom_element_metric_type_t metric);
typedef vl_result_t (*vl_dom_element_free_func)(struct vl_dom_element *element);

struct vl_dom_element_funcs {
    vl_dom_element_render_func render;
    vl_dom_element_set_property_func set_property;
    vl_dom_element_get_metric2_func get_metric2;
    vl_dom_element_free_func free;
};

typedef struct vl_dom_element_funcs vl_dom_element_funcs_t;

struct vl_dom;
struct vl_dom_element {
    struct vl_dom *owner;
    struct vl_dom_element *parent;

    const char *tag;
    VL_DA(struct vl_dom_element*) children;
    VL_DA(char) class_name;
    vl_css_style_t style;
    vl_css_class_selector_t element_selector;
};

typedef struct vl_dom_element vl_dom_element_t;

typedef vl_dom_element_t* (*vl_dom_element_new_func)(vl_source_location_t loc);

#define VL_DOM_ELEMENT_FUNCS(ELEMENT) \
    ((vl_dom_element_funcs_t*) VL_PTR_BACKWARD((ELEMENT), sizeof(vl_dom_element_funcs_t)))

#define vl_dom_element_new_va_expand(tag, loc, ...) \
    vl_dom_element_new_(tag, loc)
#define vl_dom_element_new(...) \
    vl_dom_element_new_va_expand(__VA_ARGS__, VL_SOURCE_LOCATION_HERE)
VL_API vl_dom_element_t *vl_dom_element_new_(const char *tag, vl_source_location_t loc);
VL_API vl_result_t vl_dom_element_render(vl_dom_element_t *element, vl_dom_render_opts_t *opts);
VL_API vl_result_t vl_dom_element_update_style(vl_dom_element_t *element);
VL_API vl_result_t vl_dom_element_set_string(vl_dom_element_t *element, const char *property, const char *value);
VL_API vl_result_t vl_dom_element_set_property(vl_dom_element_t *element, const char *property, vl_dom_element_property_type_t type, const void *value);
VL_API vl_result_t vl_dom_element_free(vl_dom_element_t *element);

#endif // VL_DOM_ELEMENT_H