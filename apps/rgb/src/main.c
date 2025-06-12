/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>

// Thread stack sizes
#define STACK_SIZE 1024

#define RGB_THREAD_PRIORITY 7
#define CONTROL_THREAD_PRIORITY 6

// Function prototypes
void rgb_thread();
void control_thread();

// Define the threads
K_THREAD_DEFINE(rgb_tid, STACK_SIZE, rgb_thread, NULL, NULL, NULL, RGB_THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(control_tid, STACK_SIZE, control_thread, NULL, NULL, NULL, CONTROL_THREAD_PRIORITY, 0, 0);

static const struct gpio_dt_spec clk = GPIO_DT_SPEC_GET(DT_NODELABEL(gpio_clk), gpios);
static const struct gpio_dt_spec data = GPIO_DT_SPEC_GET(DT_NODELABEL(gpio_data), gpios);

typedef struct {
    uint8_t b; // Blue
    uint8_t g; // Green
    uint8_t r; // Red
} color_t;

// Define message queue for color structs
K_MSGQ_DEFINE(color_msgq, sizeof(color_t), 1, 1);

//color values
color_t colors[] = {
    {0x00, 0x00, 0x00}, // Black
    {0x00, 0x00, 0xFF}, // Blue
    {0x00, 0xFF, 0x00}, // Green
    {0x00, 0xFF, 0xFF}, // Cyan
    {0xFF, 0x00, 0x00}, // Red
    {0xFF, 0x00, 0xFF}, // Magenta
    {0xFF, 0xFF, 0x00}, // Yellow
    {0xFF, 0xFF, 0xFF}  // White
};

void clk_pulse() {
    gpio_pin_set_dt(&clk, 0);
    k_msleep(10);            // 1 us delay
    gpio_pin_set_dt(&clk, 1);
	k_msleep(10);
}

int chainable_led_init() {
	if (!gpio_is_ready_dt(&clk) || !gpio_is_ready_dt(&data)) {
        printf("gpio not ready\r\n");
        return -ENODEV;
    }
	int ret;
    ret = gpio_pin_configure_dt(&clk, GPIO_OUTPUT);
    if (ret < 0) {
        printf("clk pin error\r\n");
        return ret;
    }

    ret = gpio_pin_configure_dt(&data, GPIO_OUTPUT);
	if (ret < 0) {
        printf("data pin error\r\n");
        return ret;
    }
    printf("Chainable LED initialized\r\n");
    return 0;

}

void send_byte(uint8_t b) {
    for (uint8_t i = 0; i < 8; i++) {
		if ((b & 0x80) != 0) {
			gpio_pin_set_dt(&data, 1);
		} else {
			gpio_pin_set_dt(&data, 0);
		}

        clk_pulse();  // Clock pulse
        b <<= 1;  // Shift to next bit
    }
}

void send_color(uint8_t red, uint8_t green, uint8_t blue) {
	uint8_t prefix = 0b11000000;
    if (!(blue & 0x80))  prefix |= 0b00100000;
    if (!(blue & 0x40))  prefix |= 0b00010000;
    if (!(green & 0x80)) prefix |= 0b00001000;
    if (!(green & 0x40)) prefix |= 0b00000100;
    if (!(red & 0x80))   prefix |= 0b00000010;
    if (!(red & 0x40))   prefix |= 0b00000001;

    send_byte(prefix);
    send_byte(blue);
    send_byte(green);
    send_byte(red);
}

void set_rgb_color(uint8_t red, uint8_t green, uint8_t blue) {
	// Start frame (32-bits of 0)
	send_byte(0x00);
	send_byte(0x00);
	send_byte(0x00);
	send_byte(0x00);

	// Send LED color
	send_color(red, green, blue);

	// End frame (32-bits of 0)
	send_byte(0x00);
	send_byte(0x00);
	send_byte(0x00);
	send_byte(0x00);
}

void rgb_thread() {
    color_t received_color; // Struct to store received color

    while (1) {
        k_msgq_get(&color_msgq, &received_color, K_FOREVER);
        set_rgb_color(received_color.b, received_color.g, received_color.r);
    }
}

void control_thread() {
    if (chainable_led_init() != 0) {
        return;
    }
    while (1) {
        for (int i = 0; i < 8; i++) {
            k_msgq_put(&color_msgq, &colors[i], K_NO_WAIT);
            k_msleep(2000);
        }
    }
}

