#ifndef VELVET_DOM_H
#define VELVET_DOM_H

#include "velvet/dom/element.h"

struct vl_web;
struct vl_dom {
    struct vl_web *owner;
    vl_dom_element_t *root;
};

typedef struct vl_dom vl_dom_t;

#endif // VELVET_DOM_H