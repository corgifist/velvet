#include "velvet/support/color.h"
#include <math.h>

// Based on https://stackoverflow.com/a/6930407

vl_hsv_t vl_rgb_to_hsv(vl_rgb_t rgb) {
    vl_hsv_t hsv;
    float min, max, delta;

    min = rgb.r < rgb.g ? rgb.r : rgb.g;
    min = min   < rgb.b ? min   : rgb.b;

    max = rgb.r > rgb.g ? rgb.r : rgb.g;
    max = max   > rgb.b ? max   : rgb.b;

    hsv.v = max;
    delta = max - min;
    if (delta < 0.00001) {
        hsv.s = 0;
        hsv.h = 0;
        return hsv;
    }

    if (max > 0.0) {
        hsv.s = (delta / max);
    } else {
        hsv.s = 0.0;
        hsv.h = NAN;
        return hsv;
    }

    if (rgb.r >= max) {
        hsv.h = (rgb.g - rgb.b) / delta;
    } else if (rgb.g >= max) {
        hsv.h = 2.0 + (rgb.b - rgb.r) / delta;
    } else {
        hsv.h = 4.0 + (rgb.r - rgb.g) / delta;
    }

    hsv.h *= 60.0;

    if (hsv.h < 0.0) {
        hsv.h += 360.0;
    }
    return hsv;
}

vl_rgb_t vl_hsv_to_rgb(vl_hsv_t hsv) {
    double hh, p, q, t, ff;
    long i;
    vl_rgb_t rgb;

    if (hsv.s <= 0.0) {
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
        return rgb;
    }

    hh = hsv.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = hsv.v * (1.0 - hsv.s);
    q = hsv.v * (1.0 - (hsv.s * ff));
    t = hsv.v * (1.0 - (hsv.s * (1.0 - ff)));

    switch(i) {
    case 0:
        rgb.r = hsv.v;
        rgb.g = t;
        rgb.b = p;
        break;
    case 1:
        rgb.r = q;
        rgb.g = hsv.v;
        rgb.b = p;
        break;
    case 2:
        rgb.r = p;
        rgb.g = hsv.v;
        rgb.b = t;
        break;

    case 3:
        rgb.r = p;
        rgb.g = q;
        rgb.b = hsv.v;
        break;
    case 4:
        rgb.r = t;
        rgb.g = p;
        rgb.b = hsv.v;
        break;
    case 5:
    default:
        rgb.r = hsv.v;
        rgb.g = p;
        rgb.b = q;
        break;
    }

    return rgb;     
}

vl_color_t vl_color_hue_shift(vl_color_t color, float hue) {
    vl_hsv_t hsv = vl_rgb_to_hsv(color.rgb);
    hsv.h += hue;
    color.rgb = vl_hsv_to_rgb(hsv);
    return color;
}