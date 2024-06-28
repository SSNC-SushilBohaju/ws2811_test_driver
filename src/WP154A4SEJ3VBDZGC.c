#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

// Constants for memory-mapped I/O
#define BCM2708_PERI_BASE 0x20000000
#define GPIO_BASE (BCM2708_PERI_BASE + 0x200000) // GPIO controller
#define BLOCK_SIZE (4 * 1024)

/* GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x)
   or SET_GPIO_ALT(x,y) */
#define INP_GPIO(g) *(ugpio + ((g) / 10)) &= ~(7 << (((g) % 10) * 3))
#define OUT_GPIO(g) *(ugpio + ((g) / 10)) |= (1 << (((g) % 10) * 3))
#define SET_GPIO_ALT(g, a)                                          \
    *(ugpio + (((g) / 10))) |= (((a) <= 3 ? (a) + 4 : (a) == 4 ? 3  \
                                                               : 2) \
                                << (((g) % 10) * 3))

#define GPIO_SET *(ugpio + 7)  /* sets   bits */
#define GPIO_CLR *(ugpio + 10) /* clears bits */
#define GPIO_GET *(ugpio + 13) /* gets   all GPIO input levels */

typedef enum
{
    Input = 0, /* GPIO is an Input */
    Output     /* GPIO is an Output */
} direction_t;

static volatile unsigned *ugpio;

// Perform initialization to access GPIO registers
static void gpio_init()
{
    int fd;
    char *map;

    fd = open("/dev/mem", O_RDWR | O_SYNC); /* Needs root access */
    if (fd < 0)
    {
        perror("Opening /dev/mem");
        exit(1);
    }

    map = (char *)mmap(
        NULL,       /* Any address */
        BLOCK_SIZE, /* # of bytes */
        PROT_READ | PROT_WRITE,
        MAP_SHARED, /* Shared */
        fd,         /* /dev/mem */
        GPIO_BASE   /* Offset to GPIO */
    );

    if ((long)map == -1L)
    {
        perror("mmap(/dev/mem)");
        exit(1);
    }

    close(fd);
    ugpio = (volatile unsigned *)map;
}

/*********************************************************************
 * Configure GPIO as Input or Output
 *********************************************************************/
static inline void gpio_config(int gpio, direction_t output)
{
    INP_GPIO(gpio);
    if (output)
    {
        OUT_GPIO(gpio);
    }
}

/*********************************************************************
 * Write a bit to the GPIO pin
 *********************************************************************/
static inline void gpio_write(int gpio, int bit)
{
    unsigned sel = 1 << gpio;

    if (bit)
    {
        GPIO_SET = sel;
    }
    else
    {
        GPIO_CLR = sel;
    }
}

/*********************************************************************
 * Read a bit from a GPIO pin
 *********************************************************************/
static inline int gpio_read(int gpio)
{
    unsigned sel = 1 << gpio;

    return (GPIO_GET & sel) ? 1 : 0;
}

// GPIO pin numbers for RGB LED (change these to your actual pins)
int red_pin = 17;
int green_pin = 27;
int blue_pin = 22;

// Function to set RGB color
void set_rgb(int r, int g, int b)
{
    gpio_write(red_pin, r);
    gpio_write(green_pin, g);
    gpio_write(blue_pin, b);
}

int main(void)
{

    // Initialize GPIO
    gpio_init();

    // Set the pins as outputs
    gpio_config(red_pin, Output);
    gpio_config(green_pin, Output);
    gpio_config(blue_pin, Output);

    // Example: Set RGB to different colors
    set_rgb(255, 0, 0); // Red
    sleep(1);
    set_rgb(0, 255, 0); // Green
    sleep(1);
    set_rgb(0, 0, 255); // Blue
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
    if (munmap((void *)ugpio, BLOCK_SIZE) == -1)
    {
        perror("munmap");
    }

    return 0;
}
