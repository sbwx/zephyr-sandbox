/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/mutex.h>

// Structure to hold RNG data
struct rng_data {
    void *fifo_reserved;  // First member required by k_fifo
    uint32_t random_number;
};

// Define FIFO queue
K_FIFO_DEFINE(rng_fifo);

/* 2000 msec = 2 sec */
#define SLEEP_TIME_MS   2000

// Thread stack sizes
#define STACK_SIZE 1024

// Thread priorities
#define RNG_THREAD_PRIORITY 1
#define DISPLAY_THREAD_PRIORITY 2

// Function prototypes
void rng_thread();
void display_thread();

// Define the threads
K_THREAD_DEFINE(rng_tid, STACK_SIZE, rng_thread, NULL, NULL, NULL, RNG_THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(display_tid, STACK_SIZE, display_thread, NULL, NULL, NULL, DISPLAY_THREAD_PRIORITY, 0, 0);

void rng_thread() {
	while(1) {
		// wait for signal from display thread
		k_sleep(K_FOREVER);

		// create data struct to be passed into fifo
		struct rng_data *data = k_malloc(sizeof(struct rng_data));

		// malloc error check
		if (!data) {
			printf("Memory allocation failed.\r\n");
			continue;
		}

		// generate cs random number
		int err = sys_csrand_get(&data->random_number, sizeof(data->random_number));
		// random number error check
		if (err < 0) {
			printf("Failed to generate random number\n");
			k_free(data);
			continue;
		}

		// 8 digit number
		data->random_number = data->random_number % 100000000;

		// put data struct in fifo
		k_fifo_put(&rng_fifo, data);

	}
}

void display_thread() {
	while(1) {
		printf("Requesting random number...\r\n");
		// signal rng thread for random number
		k_wakeup(rng_tid);

		// get data struct from fifo
		struct rng_data *received_data = k_fifo_get(&rng_fifo, K_FOREVER);

		// print random number, 0 extended
		printf("%08u\r\n", received_data->random_number);

		// free struct
		k_free(received_data);

		// wait 2 seconds
		k_msleep(SLEEP_TIME_MS);
	}
}
