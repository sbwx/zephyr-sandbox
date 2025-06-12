/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

#define STACK_SIZE 1024
#define SHELL_THREAD_PRIORITY 1

const struct device* dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));

LOG_MODULE_REGISTER(led_module, LOG_LEVEL_DBG);

static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

void get_time_seconds(const struct shell *sh);
void get_time_f(const struct shell *sh);
void led_set(const struct shell *sh, int argc, char** argv);
void led_toggle(const struct shell *sh, int argc, char** argv);
void led_sub_err(int argc, char** argv);
void dt3_thread();

K_THREAD_DEFINE(shell_tid, STACK_SIZE, dt3_thread, NULL, NULL, NULL, SHELL_THREAD_PRIORITY, 0, 0);

SHELL_STATIC_SUBCMD_SET_CREATE(time_f, SHELL_CMD(f, NULL, "Displays the system time in hours, minutes and seconds.", &get_time_f), SHELL_SUBCMD_SET_END);
SHELL_CMD_ARG_REGISTER(time, &time_f, "Displays the system time in seconds.", &get_time_seconds, 1, 1);

SHELL_STATIC_SUBCMD_SET_CREATE(led_sub, SHELL_CMD_ARG(s, NULL, "Sets the LED on/off.", &led_set, 2, 0), SHELL_CMD_ARG(t, NULL, "Toggles the LED on/off.", &led_toggle, 2, 0), SHELL_SUBCMD_SET_END);
SHELL_CMD_ARG_REGISTER(led, &led_sub, "Set/toggle LEDs 1 and 2.", &led_sub_err, 3, 0);

static uint8_t led1Status = 0;
static uint8_t led2Status = 0;

void get_time_seconds(const struct shell *sh) {
	uint32_t seconds = k_uptime_seconds();
	shell_print(sh, "Time elapsed: %d seconds.", seconds);
}

void get_time_f(const struct shell *sh) {
	uint32_t seconds = k_uptime_seconds();
	uint32_t minutes = 0;
	uint32_t hours = 0;
	if (seconds >= 60) {
		minutes = seconds / 60;
		seconds = seconds % 60;
		if (minutes >= 60) {
			hours = minutes / 60;
			minutes = minutes % 60;
		}
	}
	shell_print(sh, "Time elapsed: %d hours, %d minutes and %d seconds.", hours, minutes, seconds);
}

void led_set(const struct shell *sh, int argc, char** argv) {
	uint8_t bitmask = 0;
	if (strlen(argv[1]) != 2) {
		LOG_ERR("invalid command");
		return;
	}
	for (int i = 0; i < 2; i++) {
		if ((argv[1][i] != '0') && (argv[1][i] != '1')) {
			LOG_ERR("invalid command");
			return;
		}
		bitmask |= ((argv[1][i] - '0') << (1 - i));

	}
	
	if ((bitmask & 0x03) == 0x00) {
		if (led1Status == 0) {
			LOG_WRN("led 1 already off");
		} else {
			gpio_pin_set_dt(&led1, 0);
			LOG_INF("led 1 is off");
			led1Status = 0;
		}
		if (led2Status == 0) {
			LOG_WRN("led 2 already off");
		} else {
			gpio_pin_set_dt(&led2, 0);
			LOG_INF("led 2 is off");
			led2Status = 0;
		}
	}
	if ((bitmask & 0x03) == 0x01) {
		if (led1Status == 0) {
			gpio_pin_set_dt(&led1, 1);
			LOG_INF("led 1 is on");
			led1Status = 1;
		} else {
			LOG_WRN("led 1 already on");
		}
		if (led2Status == 0) {
			LOG_WRN("led 2 already off");
		} else {
			gpio_pin_set_dt(&led2, 0);
			LOG_INF("led 2 is off");
			led2Status = 0;
		}
	}
	if ((bitmask & 0x03) == 0x02) {
		if (led1Status == 0) {
			LOG_WRN("led 1 already off");
		} else {
			gpio_pin_set_dt(&led1, 0);
			LOG_INF("led 1 is off");
			led1Status = 0;
		}
		if (led2Status == 0) {
			gpio_pin_set_dt(&led2, 1);
			LOG_INF("led 2 is on");
			led2Status = 1;
		} else {
			LOG_WRN("led 2 already on");
		}
	}
	if ((bitmask & 0x03) == 0x03) {
		if (led1Status == 0) {
			gpio_pin_set_dt(&led1, 1);
			LOG_INF("led 1 is on");
			led1Status = 1;
		} else {
			LOG_WRN("led 1 already on");
		}
		if (led2Status == 0) {
			gpio_pin_set_dt(&led2, 1);
			LOG_INF("led 2 is on");
			led2Status = 1;
		} else {
			LOG_WRN("led 2 already on");
		}
	}
}

void led_toggle(const struct shell *sh, int argc, char** argv) {
	uint8_t bitmask = 0;
	if (strlen(argv[1]) != 2) {
		LOG_ERR("invalid command");
		return;
	}
	for (int i = 0; i < 2; i++) {
		if ((argv[1][i] != '0') && (argv[1][i] != '1')) {
			LOG_ERR("invalid command");
			return;
		}
		bitmask |= ((argv[1][i] - '0') << (1 - i));

	}
	
	if ((bitmask & 0x03) == 0x01) {
		if (led1Status == 0) {
			gpio_pin_set_dt(&led1, 1);
			LOG_INF("led 1 is on");
			led1Status = 1;
		} else {
			gpio_pin_set_dt(&led1, 0);
			LOG_INF("led 1 is off");
			led1Status = 0;
		}
	}
	if ((bitmask & 0x03) == 0x02) {
		if (led2Status == 0) {
			gpio_pin_set_dt(&led2, 1);
			LOG_INF("led 2 is on");
			led2Status = 1;
		} else {
			gpio_pin_set_dt(&led2, 0);
			LOG_INF("led 2 is off");
			led2Status = 0;
		}
	}
	if ((bitmask & 0x03) == 0x03) {
		if (led1Status == 0) {
			gpio_pin_set_dt(&led1, 1);
			LOG_INF("led 1 is on");
			led1Status = 1;
		} else {
			gpio_pin_set_dt(&led1, 0);
			LOG_INF("led 1 is off");
			led1Status = 0;
		}
		if (led2Status == 0) {
			gpio_pin_set_dt(&led2, 1);
			LOG_INF("led 2 is on");
			led2Status = 1;
		} else {
			gpio_pin_set_dt(&led2, 0);
			LOG_INF("led 2 is off");
			led2Status = 0;
		}
	}
}

void led_sub_err(int argc, char** argv) {
	if (argv[1] != 's' && argv[1] != 't') {
		LOG_ERR("invalid command");
	}
}

void dt3_thread() {
	if (!gpio_is_ready_dt(&led1) || !gpio_is_ready_dt(&led2)) {
		LOG_ERR("gpio not ready");
	}
	LOG_DBG("led init OK");

	uint32_t dtr = 0;
	gpio_pin_configure_dt(&led1, GPIO_OUTPUT);
	gpio_pin_configure_dt(&led2, GPIO_OUTPUT);
	gpio_pin_set_dt(&led1, 0);
	gpio_pin_set_dt(&led2, 0);

	if (!device_is_ready(dev)) {
		LOG_ERR("device not ready");
	}	

	while (!dtr) {
		uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
		k_sleep(K_MSEC(100));
	}
}
