#include "core/system.h"
#include <linux/time.h>

static struct timespec start_time; // Declare start_time globally


// static volatile uint64_t ticks = 0;

void sys_tick_handler(void) {
}

static void rcc_setup(void) {
  
}

static void systick_setup(void) {
     clock_gettime(CLOCK_MONOTONIC, &start_time);

}

uint64_t system_get_ticks(void) {
  struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    uint64_t elapsed_ms = (current_time.tv_sec - start_time.tv_sec) * 1000;
    elapsed_ms += (current_time.tv_nsec - start_time.tv_nsec) / 1000000;

    return elapsed_ms;
}

void system_setup(void) {
  rcc_setup();
  systick_setup();
}
