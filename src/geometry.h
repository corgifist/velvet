#pragma once

namespace velvet {

class Rect {
public:
    Rect(int _x, int _y, int _w, int _h) : x(_x), y(_y), w(_w), h(_h) {} 

    int x, y, w, h;
};

}