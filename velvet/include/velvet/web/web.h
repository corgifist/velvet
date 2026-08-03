#ifndef VELVET_WEB_H
#define VELVET_WEB_H

#include "velvet/dom/dom.h"

struct vl_web {
    const char *title;
    vl_dom_t dom;
};

typedef struct vl_web vl_web_t;

#endif // VELVET_WEB_H