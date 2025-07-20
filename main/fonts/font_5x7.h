#ifndef __FONT5X7_H__
#define __FONT5X7_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

#include <stdint.h>

// Only ASCII 32 (' ') to 95 ('_') for brevity
#define FONT_CHAR_MIN 32
#define FONT_CHAR_MAX 126
#define FONT_WIDTH 5
#define FONT_HEIGHT 7

extern const uint8_t font5x7[][FONT_WIDTH];

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __FONT5X7_H__ */