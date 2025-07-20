#include "flipdot_driver.h"
#include "flipdot_gfx.h"
#include "fonts/font_5x7.h"
#include <math.h>

static uint8_t framebuffer[NUM_PANELS_VERTICAL][DISPLAY_WIDTH];

static uint8_t flipdot_get_char_width(const uint8_t *glyph) {
    int8_t start = 0;
    int8_t end = FONT_WIDTH;

    // Trim leading 0x00 columns
    while (start < FONT_WIDTH && glyph[start] == 0x00) {
        start++;
    }

    // Trim trailing 0x00 columns
    while (end > start && glyph[end - 1] == 0x00) {
        end--;
    }

    return end - start;  // Effective width of the glyph
}

void flipdot_clear(void) {
    for (uint8_t pixel = 0; pixel < DISPLAY_WIDTH; pixel++) {
        for (uint8_t panel = 0; panel < NUM_PANELS_VERTICAL; panel++) {
            framebuffer[panel][pixel] = 0x00;
        }
    }
}

void flipdot_set_pixel(uint8_t x, uint8_t y, bool on) {
    if (x >= DISPLAY_WIDTH || y >= PANEL_HEIGHT *  NUM_PANELS_VERTICAL)
        return;

    uint8_t panel = y / PANEL_HEIGHT;
    uint8_t bit = y % PANEL_HEIGHT;

    if (on)
        framebuffer[panel][x] |= (1 << bit);
    else
        framebuffer[panel][x] &= ~(1 << bit);
}

bool flipdot_get_pixel(uint8_t x, uint8_t y) {
    if (x >= DISPLAY_WIDTH || y >= PANEL_HEIGHT * NUM_PANELS_VERTICAL)
        return false;

    uint8_t panel = y / PANEL_HEIGHT;
    uint8_t bit = y % PANEL_HEIGHT;

    return (framebuffer[panel][x] >> bit) & 0x01;
}

void flipdot_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool on) {
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (true) {
        flipdot_set_pixel(x0, y0, on);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void flipdot_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool on) {
    flipdot_draw_line(x, y, x + w - 1, y, on);               // Top
    flipdot_draw_line(x, y + h - 1, x + w - 1, y + h - 1, on); // Bottom
    flipdot_draw_line(x, y, x, y + h - 1, on);               // Left
    flipdot_draw_line(x + w - 1, y, x + w - 1, y + h - 1, on); // Right
}


void flipdot_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool on) {
    for (uint8_t i = 0; i < h; ++i) {
        flipdot_draw_line(x, y + i, x + w - 1, y + i, on);
    }
}


void flipdot_draw_circle(uint8_t xc, uint8_t yc, uint8_t r, bool on) {
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    while (y >= x) {
        flipdot_set_pixel(xc + x, yc + y, on);
        flipdot_set_pixel(xc - x, yc + y, on);
        flipdot_set_pixel(xc + x, yc - y, on);
        flipdot_set_pixel(xc - x, yc - y, on);
        flipdot_set_pixel(xc + y, yc + x, on);
        flipdot_set_pixel(xc - y, yc + x, on);
        flipdot_set_pixel(xc + y, yc - x, on);
        flipdot_set_pixel(xc - y, yc - x, on);
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}


void flipdot_fill_circle(uint8_t xc, uint8_t yc, uint8_t r, bool on) {
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    while (y >= x) {
        flipdot_draw_line(xc - x, yc - y, xc + x, yc - y, on);
        flipdot_draw_line(xc - y, yc - x, xc + y, yc - x, on);
        flipdot_draw_line(xc - x, yc + y, xc + x, yc + y, on);
        flipdot_draw_line(xc - y, yc + x, xc + y, yc + x, on);
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

void flipdot_draw_char(uint8_t x, uint8_t y, char c) {
    if (c < FONT_CHAR_MIN || c > FONT_CHAR_MAX) return;

    const uint8_t *glyph = font5x7[c - FONT_CHAR_MIN];

    uint8_t start_index = 0;
    while (start_index < FONT_WIDTH && glyph[start_index] == 0x00) {
        start_index++;
    }

    uint8_t character_width = flipdot_get_char_width(glyph);

    for (uint8_t dx = 0; dx < character_width; dx++) {
        uint8_t col_data = glyph[dx + start_index];
        for (uint8_t dy = 0; dy < FONT_HEIGHT; dy++) {
            bool on = (col_data >> dy) & 0x01;
            flipdot_set_pixel(x + dx, y + dy, on);
        }
    }
}

void flipdot_draw_text(uint8_t x, uint8_t y, const char *text) {
    while (*text && x + FONT_WIDTH <= DISPLAY_WIDTH) {
        uint8_t c = *text++;
        flipdot_draw_char(x, y, c);
        x += flipdot_get_char_width(font5x7[c - FONT_CHAR_MIN]) + 1;  // 1 pixel spacing between chars
    }
}

/*void flipdot_draw_bitmap(uint8_t x, uint8_t y, const uint16_t *bitmap, uint8_t width, uint8_t height, bool on) {
    for (uint8_t dx = 0; dx < width; dx++) {
        uint16_t col_data = bitmap[dx];
        for (uint8_t dy = 0; dy < height; dy++) {
            bool on = (col_data >> dy) & 0x01;
            flipdot_set_pixel(x + dx, y + dy, on);
        }
    }
}*/

void flipdot_draw_bitmap(uint8_t x, uint8_t y, const uint16_t *bitmap, bool on) {
    uint8_t width = 0;
    while (bitmap[width] != 0x0000 && bitmap[width] != 0xFFFF) {
        width++;
    }

    uint8_t min_height = 16;
    uint8_t max_height = 0;

    for (uint8_t dx = 0; dx < width; dx++) {
        uint16_t col_data = bitmap[dx];

        for (uint8_t dy = 0; dy < 16; dy++) {
            if ((col_data >> dy) & 0x01) {
                if (dy < min_height) {
                    min_height = dy;
                }
                if (dy > max_height) {
                    max_height = dy;
                }
            }
        }
    }

    for (uint8_t dx = 0; dx < width; dx++) {
        uint16_t col_data = bitmap[dx];
        for (uint8_t dy = min_height; dy <= max_height; dy++) {
            bool on_pixel = (col_data >> dy) & 0x01;
            flipdot_set_pixel(x + dx, y + (dy - min_height), on_pixel);
        }
    }
}


void flipdot_display(void) {
    flipdot_enable_transmit();
    for (uint8_t panel = 0; panel < NUM_PANELS_VERTICAL; panel++) {
        flipdot_write(&framebuffer[panel][0], PANEL_WIDTH, (panel + 1), NO_PANEL_REFRESH);
    }
    flipdot_refresh();
    flipdot_disable_transmit();
}