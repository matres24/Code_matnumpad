#include "analog.h"
#include "quantum.h"
#include "gpio.h"
#include <stdint.h>
#include "matrix.h"
#include "print.h"
#include "_wait.h"

// Multiplexer control pins
#define MUX_S0_PIN A6
#define MUX_S1_PIN A7
#define MUX_S2_PIN A4
#define MUX_S3_PIN A3
#define MUX_ENABLE_PIN A5

// Analog input pins
#define MUX_ANALOG_PIN A1  // Multiplexer output
#define DIRECT_SENSOR_PIN A2 // Directly connected sensor



const uint8_t row_map[16] = {
    0,    // ch0  = /        (row 0)
    0,    // ch1  = TG1      (row 0)
    1,    // ch2  = 7        (row 1)
    1,    // ch3  = 8        (row 1)
    3,    // ch4  = 2        (row 3)
    3,    // ch5  = 1        (row 3)
    2,    // ch6  = 5        (row 2)
    2,    // ch7  = 4        (row 2)
    1,    // ch8  = 9        (row 1)
    2,    // ch9  = 6        (row 2)
    3,    // ch10 = 3        (row 3)
    4,    // ch11 = .        (row 4)
    4,    // ch12 = 0        (row 4)
    3,    // ch13 = Enter    (row 3) <-- Changed from 4 to 3
    1,    // ch14 = +        (row 1) <-- Changed from 2 to 1
    0     // ch15 = -        (row 0)
};

const uint8_t col_map[16] = {
    1,    // ch0  = /        (col 1)
    0,    // ch1  = TG1      (col 0)
    0,    // ch2  = 7        (col 0)
    1,    // ch3  = 8        (col 1)
    1,    // ch4  = 2        (col 1)
    0,    // ch5  = 1        (col 0)
    1,    // ch6  = 5        (col 1)
    0,    // ch7  = 4        (col 0)
    2,    // ch8  = 9        (col 2)
    2,    // ch9  = 6        (col 2)
    2,    // ch10 = 3        (col 2)
    2,    // ch11 = .        (col 2) <-- Changed from 1 to 2
    0,    // ch12 = 0        (col 0)
    3,    // ch13 = Enter    (col 3) <-- Changed from 2 to 3
    3,    // ch14 = +        (col 3)
    3     // ch15 = -        (col 3)
};

void matrix_init_custom(void) {
gpio_set_pin_output(MUX_S0_PIN);
gpio_set_pin_output(MUX_S1_PIN);
gpio_set_pin_output(MUX_S2_PIN);
gpio_set_pin_output(MUX_S3_PIN);

gpio_set_pin_output(MUX_ENABLE_PIN);

gpio_write_pin_low(MUX_ENABLE_PIN);

palSetLineMode(MUX_ANALOG_PIN, PAL_MODE_INPUT_ANALOG);
palSetLineMode(DIRECT_SENSOR_PIN, PAL_MODE_INPUT_ANALOG);
}

void select_mux_channel(uint8_t channel) {
    // Set multiplexer channel using binary encoding
    gpio_write_pin(MUX_S0_PIN, channel & 0x01);
    gpio_write_pin(MUX_S1_PIN, (channel >> 1) & 0x01);
    gpio_write_pin(MUX_S2_PIN, (channel >> 2) & 0x01);
    gpio_write_pin(MUX_S3_PIN, (channel >> 3) & 0x01);
    uprintf("Mux channel set: %d\n", channel);
}


bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    static matrix_row_t last_matrix[MATRIX_ROWS] = {0};
    bool matrix_has_changed = false;

    // Copy last state to current
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        current_matrix[row] = last_matrix[row];
    }

    // First read the direct analog input (A2) for the * key
    uint16_t direct_value = analogReadPin(DIRECT_SENSOR_PIN);
    if (direct_value > 370) {
        current_matrix[0] |= (1 << 2);  // * key at row 0, col 2
        uprintf("\n\nDIRECT KEY (*) PRESSED! ADC: %d\n\n", direct_value);
    } else {
        current_matrix[0] &= ~(1 << 2);
    }

    // Scan all multiplexer channels
    for (uint8_t channel = 0; channel < 16; channel++) {
        select_mux_channel(channel);
        wait_us(10);  // Short delay for mux to settle

        uint16_t sensor_value = analogReadPin(MUX_ANALOG_PIN);
        uint8_t row = row_map[channel];
        uint8_t col = col_map[channel];

        if (sensor_value > 370) {
            current_matrix[row] |= (1 << col);
            uprintf("\n\nKEY PRESSED!\n");
            uprintf("Channel: %d\n", channel);
            uprintf("Position: row %d, col %d\n", row, col);
            uprintf("ADC Value: %d\n\n", sensor_value);
        } else {
            current_matrix[row] &= ~(1 << col);
        }
    }

    // Check for changes
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        if (last_matrix[row] != current_matrix[row]) {
            matrix_has_changed = true;
            uprintf("Matrix row %d: 0x%02X\n", row, current_matrix[row]);
            last_matrix[row] = current_matrix[row];
        }
    }

    wait_us(125);  // 1 second delay between scans

    return matrix_has_changed;
}


