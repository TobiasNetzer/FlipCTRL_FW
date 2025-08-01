#ifndef __FLIPDOT_DRIVER_H__
#define __FLIPDOT_DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus */

#include <stdint.h>
#include <stddef.h>

#define FLIPDOT_MATRIX_WIDTH 28
#define FLIPDOT_MATRIX_HEIGHT 14

typedef enum flipdot_matrix_command_e{
    PANEL_REFRESH = 0x83,
    NO_PANEL_REFRESH = 0x84
} flipdot_matrix_command_t;

typedef struct __attribute__((packed)) flipdot_matrix_s {
    uint8_t start_of_frame;
    uint8_t command;
    uint8_t address;
    uint8_t data[28];
    uint8_t end_of_frame;
} flipdot_matrix_t;

void flipdot_init(void);

void flipdot_refresh(void);

void flipdot_write(uint8_t *data_frame, size_t length, uint8_t panel_address, flipdot_matrix_command_t command);

void flipdot_enable_transmit(void);

void flipdot_disable_transmit(void);

#ifdef __cplusplus
}
#endif /*__cplusplus */

#endif /* __FLIPDOT_DRIVER_H__ */