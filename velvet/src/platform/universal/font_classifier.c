#include "velvet/platform/universal/stb_truetype.h"
#include "velvet/font/font.h"

static uint16_t read_u16be(const unsigned char *p) {
    return (uint16_t)((p[0] << 8) | p[1]);
}

static uint32_t read_u32be(const unsigned char *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static int find_table(const unsigned char *ttf, int font_offset, const char tag[4], uint32_t *out_offset, uint32_t *out_length)
{
    const unsigned char *p = ttf + font_offset;
    uint16_t numTables = read_u16be(p + 4);
    const unsigned char *dir = p + 12;

    for (uint16_t i = 0; i < numTables; i++) {
        const unsigned char *e = dir + i * 16;
        if (memcmp(e, tag, 4) == 0) {
            *out_offset = read_u32be(e + 8);
            *out_length = read_u32be(e + 12);
            return 1;
        }
    }
    return 0;
}

static int contains_ci(const char *haystack, const char *needle)
{
    size_t n = strlen(needle);
    for (; *haystack; haystack++) {
        size_t i = 0;
        while (i < n && haystack[i] &&
               tolower((unsigned char)haystack[i]) == tolower((unsigned char)needle[i])) {
            i++;
        }
        if (i == n) return 1;
    }
    return 0;
}

static vl_font_kind_t kind_from_name(const char *name)
{
    if (!name || !*name) return VL_FONT_KIND_UNKNOWN;

    if (contains_ci(name, "mono") || contains_ci(name, "courier") ||
        contains_ci(name, "consolas") || contains_ci(name, "fixed") ||
        contains_ci(name, "code") || contains_ci(name, "terminal") ||
        contains_ci(name, "monaco") || contains_ci(name, "lucida console") ||
        contains_ci(name, "menlo") || contains_ci(name, "input") ||
        contains_ci(name, "source code") || contains_ci(name, "dejavu sans mono") ||
        contains_ci(name, "noto sans mono") || contains_ci(name, "ubuntu mono"))
        return VL_FONT_KIND_MONOSPACED;

    if (contains_ci(name, "serif") || contains_ci(name, "times") ||
        contains_ci(name, "georgia") || contains_ci(name, "cambria") ||
        contains_ci(name, "garamond") || contains_ci(name, "palatino") ||
        contains_ci(name, "baskerville") || contains_ci(name, "bookman") ||
        contains_ci(name, "minion") || contains_ci(name, "merriweather"))
        return VL_FONT_KIND_SERIF;

    if (contains_ci(name, "sans") || contains_ci(name, "arial") ||
        contains_ci(name, "helvetica") || contains_ci(name, "grotesk") ||
        contains_ci(name, "univers") || contains_ci(name, "roboto") ||
        contains_ci(name, "open sans") || contains_ci(name, "noto sans") ||
        contains_ci(name, "dejavu sans") || contains_ci(name, "ubuntu") ||
        contains_ci(name, "inter") || contains_ci(name, "segoe") ||
        contains_ci(name, "tahoma") || contains_ci(name, "verdana") ||
        contains_ci(name, "fira sans") || contains_ci(name, "lato"))
        return VL_FONT_KIND_SANS_SERIF;

    return VL_FONT_KIND_UNKNOWN;
}

static vl_font_kind_t name_table_guess(const unsigned char *ttf, int font_offset)
{
    uint32_t off, len;
    if (!find_table(ttf, font_offset, "name", &off, &len)) return VL_FONT_KIND_UNKNOWN;

    const unsigned char *p = ttf + font_offset + off;
    if (len < 6) return VL_FONT_KIND_UNKNOWN;

    uint16_t count = read_u16be(p + 2);
    uint16_t stringOffset = read_u16be(p + 4);
    const unsigned char *records = p + 6;
    const unsigned char *strings = p + stringOffset;

    char buf[512];
    buf[0] = '\0';

    for (uint16_t i = 0; i < count; i++) {
        const unsigned char *r = records + i * 12;
        uint16_t nameID = read_u16be(r + 6);
        uint16_t length = read_u16be(r + 8);
        uint16_t offset = read_u16be(r + 10);

        if (nameID != 1 && nameID != 2 && nameID != 4) continue;
        if (offset + length > len - stringOffset) continue;

        const unsigned char *s = strings + offset;
        size_t cur = strlen(buf);
        size_t room = sizeof(buf) - cur - 1;
        if (!room) break;

        for (uint16_t j = 0; j + 1 < length && room > 0; j += 2) {
            unsigned char c = s[j + 1];
            if (isprint(c) || c == ' ') {
                buf[cur++] = (char)c;
                room--;
            }
        }
        buf[cur] = '\0';
    }

    return kind_from_name(buf);
}

static vl_font_kind_t os2_panose_guess(const unsigned char *ttf, int font_offset)
{
    uint32_t off, len;
    if (!find_table(ttf, font_offset, "OS/2", &off, &len)) return VL_FONT_KIND_UNKNOWN;
    if (len < 42) return VL_FONT_KIND_UNKNOWN;

    const unsigned char *p = ttf + font_offset + off;
    const unsigned char *panose = p + 32;

    uint8_t familyType = panose[0];
    uint8_t serifStyle = panose[1];
    uint8_t proportion = panose[3];

    if (familyType == 2) {
        if (proportion == 9) return VL_FONT_KIND_MONOSPACED;
        if (serifStyle == 11 || serifStyle == 12 || serifStyle == 13) return VL_FONT_KIND_SERIF;
        if (serifStyle == 2 || serifStyle == 3 || serifStyle == 4) return VL_FONT_KIND_SANS_SERIF;
    }

    return VL_FONT_KIND_UNKNOWN;
}

static vl_font_kind_t post_guess(const unsigned char *ttf, int font_offset)
{
    uint32_t off, len;
    if (!find_table(ttf, font_offset, "post", &off, &len)) return VL_FONT_KIND_UNKNOWN;
    if (len < 16) return VL_FONT_KIND_UNKNOWN;

    const unsigned char *p = ttf + font_offset + off;
    uint32_t fixedPitch = read_u32be(p + 12);
    if (fixedPitch != 0) return VL_FONT_KIND_MONOSPACED;
    return VL_FONT_KIND_UNKNOWN;
}

static vl_font_kind_t metrics_guess(stbtt_fontinfo *font)
{
    const char *chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.,:;!|iIl1mwMW";
    const int n = (int)strlen((const char*)chars);
    int min_adv = 1 << 30, max_adv = 0;

    for (int i = 0; i < n; i++) {
        int glyph = stbtt_FindGlyphIndex(font, chars[i]);
        int ax, lsb;
        stbtt_GetGlyphHMetrics(font, glyph, &ax, &lsb);
        if (ax < min_adv) min_adv = ax;
        if (ax > max_adv) max_adv = ax;
    }

    if (max_adv - min_adv <= 2) return VL_FONT_KIND_MONOSPACED;
    return VL_FONT_KIND_UNKNOWN;
}

vl_font_kind_t classify_font(const unsigned char *ttf, int font_offset, stbtt_fontinfo *font)
{
    vl_font_kind_t k;

    k = post_guess(ttf, font_offset);
    if (k != VL_FONT_KIND_UNKNOWN) return k;

    k = name_table_guess(ttf, font_offset);
    if (k != VL_FONT_KIND_UNKNOWN) return k;

    k = os2_panose_guess(ttf, font_offset);
    if (k != VL_FONT_KIND_UNKNOWN) return k;

    k = metrics_guess(font);
    if (k != VL_FONT_KIND_UNKNOWN) return k;

    return VL_FONT_KIND_UNKNOWN;
}
