#pragma once

#define MATRIX_ROWS 5
#define MATRIX_COLS 4
#define DEBUG_MATRIX_SCAN_RATE
#define RGBLIGHT_DEFAULT_MODE 15
#define WS2812_DI_PIN A8  // Specify the data input pin for WS2812 LEDs

// PWM Driver Configuration
#define WS2812_PWM_DRIVER PWMD1     // TIM1
#define WS2812_PWM_CHANNEL 1        // Channel 1 of TIM1

// DMA Configuration
#define WS2812_DMA_STREAM STM32_DMA1_STREAM5  // DMA1 Stream 5 is typically used with TIM1
#define WS2812_DMA_CHANNEL 6                  // DMA channel 6 for TIM1

// Pin Configuration
#define WS2812_PWM_PAL_MODE 2       // Alternate function 2 for TIM1 on this pin

