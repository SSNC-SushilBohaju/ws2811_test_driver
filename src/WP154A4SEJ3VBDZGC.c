#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

// Constants for memory-mapped I/O
#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) // GPIO controller
#define BLOCK_SIZE               (4 * 1024)

// GPIO setup
#define GPIO_IN   0
#define GPIO_OUT  1
#define GPIO_SET  7
#define GPIO_CLR  10
#define GPIO_LEV  13

volatile unsigned *gpio;

// Function to set GPIO mode
void set_gpio_mode(int pin, int mode) {
    int fsel = pin / 10;
    int shift = (pin % 10) * 3;
    *(gpio + fsel) &= ~(0b111 << shift);
    *(gpio + fsel) |= (mode << shift);
}

// Function to write to a GPIO pin
void write_gpio(int pin, int value) {
    if (value)
        *(gpio + GPIO_SET) = 1 << pin;
    else
        *(gpio + GPIO_CLR) = 1 << pin;
}

// Function to read a GPIO pin
int read_gpio(int pin) {
    return (*(gpio + GPIO_LEV) >> pin) & 1;
}

int main(int argc, char **argv) {
    int mem_fd;
    void *gpio_map;

    // Open /dev/mem
    if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Map GPIO
    gpio_map = mmap(
        NULL,             // Any address in our space will do
        BLOCK_SIZE,       // Map length
        PROT_READ | PROT_WRITE, // Enable reading & writing to mapped memory
        MAP_SHARED,       // Shared with other processes
        mem_fd,           // File to map
        GPIO_BASE         // Offset to GPIO peripheral
    );

    if (gpio_map == MAP_FAILED) {
        perror("mmap");
        close(mem_fd);
        exit(EXIT_FAILURE);
    }

    // Always use volatile pointer!
    gpio = (volatile unsigned *)gpio_map;

    // GPIO pin numbers for RGB LED (change these to your actual pins)
    int red_pin = 17;
    int green_pin = 27;
    int blue_pin = 22;

    // Set the pins as outputs
    set_gpio_mode(red_pin, GPIO_OUT);
    set_gpio_mode(green_pin, GPIO_OUT);
    set_gpio_mode(blue_pin, GPIO_OUT);

    // Function to set RGB color
    void set_rgb(int r, int g, int b) {
        write_gpio(red_pin, r);
        write_gpio(green_pin, g);
        write_gpio(blue_pin, b);
    }

    // Example: Set RGB to different colors
    set_rgb(1, 0, 0); // Red
    sleep(1);
    set_rgb(0, 1, 0); // Green
    sleep(1);
    set_rgb(0, 0, 1); // Blue
    sleep(1);
    set_rgb(1, 1, 0); // Yellow
    sleep(1);
    set_rgb(1, 0, 1); // Magenta
    sleep(1);
    set_rgb(0, 1, 1); // Cyan
    sleep(1);
    set_rgb(1, 1, 1); // White
    sleep(1);
    set_rgb(0, 0, 0); // Off

    // Unmap and close /dev/mem
    if (munmap(gpio_map, BLOCK_SIZE) == -1) {
        perror("munmap");
    }
    close(mem_fd);

    return 0;
}
