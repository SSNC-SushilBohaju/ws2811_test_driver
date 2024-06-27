#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "ws2812b.h"
#include <wiringPi.h>

#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

#define GPIO_SET *(gpio + 7)  // sets bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio + 10) // clears bits which are 1 ignores bits which are 0


void setup_io() {
    int mem_fd;
    void *gpio_map;

    if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC) ) < 0) {
        perror("can't open /dev/mem");
        exit(-1);
    }

    gpio_map = mmap(
        NULL,             // Any address in our space will do
        BLOCK_SIZE,       // Map length
        PROT_READ | PROT_WRITE, // Enable reading & writing to mapped memory
        MAP_SHARED,       // Shared with other processes
        mem_fd,           // File to map
        GPIO_BASE         // Offset to GPIO peripheral
    );

    close(mem_fd); // No need to keep mem_fd open after mmap

    if (gpio_map == MAP_FAILED) {
        perror("mmap error");
        exit(-1);
    }

    gpio = (volatile unsigned *)gpio_map;
}

void setup_gpio_output(int pin) {
    int reg = pin / 10;
    int shift = (pin % 10) * 3;
    gpio[reg] = (gpio[reg] & ~(7 << shift)) | (1 << shift);
}

static inline void send_zero(uint16_t pin) {
  GPIO_SET = 1 << pin; // Set pin high

  // +300ns
  __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");

  GPIO_CLR = 1 << pin; 

  // +680ns
  __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
  __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
  __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
}

static inline void send_one(uint16_t pin) {
  GPIO_SET = 1 << pin;

  // +680ns
  __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
  __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
  __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");

  GPIO_CLR = 1 << pin; 

  // +300ns
  __asm__ volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
}

void WS2812B_Write(WS2812B_t* ws2812b) {
  for (uint16_t i = 0; i < ws2812b->num_leds; i++) {
    RGB_t led = ws2812b->leds[i];

    // Green
    for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
      if (led.g & mask) {
        send_one(ws2812b->pin);
      } else {
        send_zero(ws2812b->pin);
      }
    }

    // Red
    for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
      if (led.r & mask) {
        send_one(ws2812b->pin);
      } else {
        send_zero(ws2812b->pin);
      }
    }

    // Blue
    for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
      if (led.b & mask) {
        send_one(ws2812b->pin);
      } else {
        send_zero(ws2812b->pin);
      }
    }
  }

  // Reset code
  for (uint16_t i = 0; i < 600; i++) {
    __asm__ volatile("nop\nnop\n");
  }
}