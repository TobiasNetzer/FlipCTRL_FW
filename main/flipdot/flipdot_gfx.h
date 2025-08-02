#ifndef __FLIPDOT_GFX_H__
#define __FLIPDOT_GFX_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

#include <stdint.h>
#include <stdbool.h>

#define PANEL_WIDTH 28
#define PANEL_HEIGHT 7
#define NUM_PANELS_VERTICAL 2

#define DISPLAY_WIDTH PANEL_WIDTH // Currently don't support multiple panels in horizontal layout
#define DISPLAY_HEIGHT(NUM_PANELS_VERTICAL) ((NUM_PANELS_VERTICAL) * PANEL_HEIGHT)

#define WHITE true
#define BLACK false

void flipdot_clear(void);
void flipdot_set_pixel(uint8_t x, uint8_t y, bool on);
bool flipdot_get_pixel(uint8_t x, uint8_t y);
void flipdot_draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool on);
void flipdot_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool on);
void flipdot_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool on);
void flipdot_draw_circle(uint8_t xc, uint8_t yc, uint8_t r, bool on);
void flipdot_fill_circle(uint8_t xc, uint8_t yc, uint8_t r, bool on);
void flipdot_draw_char(uint8_t x, uint8_t y, char c);
void flipdot_draw_text(uint8_t x, uint8_t y, const char *text);
void flipdot_draw_text_fixed_width(uint8_t x, uint8_t y, const char *text);
void flipdot_draw_bitmap(uint8_t x, uint8_t y, const uint16_t *bitmap, bool on);
void flipdot_display(void);

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __FLIPDOT_GFX_H__ */