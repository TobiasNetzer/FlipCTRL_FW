#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "flipdot_driver.h"
#include <string.h>
#include <stdio.h>

#define UART_NUM UART_NUM_1
#define BUF_SIZE 1024
#define UART_TXD_PIN 17
#define UART_RXD_PIN 18
#define RS485_DE_PIN 16

static flipdot_matrix_t flipdotmatrix = {
    .start_of_frame = 0x80,
    .end_of_frame = 0x8F
};

void flipdot_init(void) {
    const uart_config_t uart_config = {
        .baud_rate = 57600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, UART_TXD_PIN, UART_RXD_PIN, RS485_DE_PIN, UART_PIN_NO_CHANGE);
}

void flipdot_refresh(void) {
    const uint8_t refresh_command[3] = {0x80, 0x82, 0x8F};

    int bytes_written = uart_write_bytes(UART_NUM, (const void *)refresh_command, sizeof(refresh_command));
    ESP_LOGI("UART1", "Sent %d bytes (Refresh Panels)", bytes_written);
}

void flipdot_write(uint8_t *data_frame, size_t length, uint8_t panel_address, flipdot_matrix_command_t command) {

    if (length != 28) {
        ESP_LOGE("flipdot", "Invalid data frame size: %d (expected 28)", length); 
        return; // this just returns for now, would probably be better to fill the remaining bytes with 0 and continue.
    }

    flipdotmatrix.command = command;
    flipdotmatrix.address = panel_address;
    memcpy(flipdotmatrix.data, data_frame, 28);

    int bytes_written = uart_write_bytes(UART_NUM, (const void *)&flipdotmatrix, sizeof(flipdotmatrix));
    ESP_LOGI("UART1", "Sent %d bytes (Data Frame)", bytes_written);
}

void flipdot_enable_transmit(void) {
    uart_set_rts(UART_NUM, 0);
}

void flipdot_disable_transmit(void) {
    uart_wait_tx_done(UART_NUM, pdMS_TO_TICKS(500));
    uart_set_rts(UART_NUM, 1);
}