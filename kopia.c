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

// Channel mapping array
// This allows you to define the exact order of sensors
const uint8_t channel_map[] = {
    1,    // TG1
    0,    // /
    16,   // * (direct input)
    15,   // -
    2,    // 7
    3,    // 8
    8,    // 9
    7,    // 4
    6,    // 5
    9,    // 6
    14,   // +
    5,    // 1
    4,    // 2
    10,   // 3
    12,   // 0
    11,   // .
    13    // Enter
};

// Matrix position mapping arrays
const uint8_t row_map[] = {0,0,0,0,1,1,1,2,2,2, 2, 3, 3, 3, 4, 4, 4};  // row for each index
const uint8_t col_map[] = {0,1,2,3,0,1,2,0,1,2, 3, 0, 1, 2, 0, 1, 2};  // col for each index

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

    // Then scan all multiplexed channels
    for (uint8_t channel_index = 0; channel_index < 16; channel_index++) {
        // Skip channel 16 as it's the direct input
        if (channel_map[channel_index] < 16) {
            select_mux_channel(channel_map[channel_index]);
            wait_us(10);  // Short delay for mux to settle

            uint16_t sensor_value = analogReadPin(MUX_ANALOG_PIN);
            uprintf("ADC Value: %d\n\n", sensor_value);
            // Use the mapping arrays instead of calculation
            uint8_t row = row_map[channel_index];
            uint8_t col = col_map[channel_index];

            if (sensor_value > 370) {
                current_matrix[row] |= (1 << col);
                uprintf("\n\nKEY PRESSED!\n");
                uprintf("Channel: %d (map %d)\n", channel_index, channel_map[channel_index]);
                uprintf("Position: row %d, col %d\n", row, col);
                uprintf("ADC Value: %d\n\n", sensor_value);
            } else {
                current_matrix[row] &= ~(1 << col);
            }
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

    wait_us(100000);  // 1 second delay between scans

    return matrix_has_changed;
}


