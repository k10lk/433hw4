#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <math.h>

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); // FIXME
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); // FIXME
}

void send_spi(float voltage) {
    float f = voltage;
    f = f*1023/3.3;
    uint16_t v = (int)f; 
    uint16_t d = 0x00 | (0b0111<<12);
    d = d | (v << 2);

    uint8_t b[2];
    b[0] = d >> 8;
    b[1] = d & 0xFF;

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, b, 2);
    cs_deselect(PIN_CS);
}

int main() {
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    send_spi(1.7);

    int array_len = 50;
    float wave_array[array_len];
    wave_array[0] = 0.0;

    // Triangle Wave
    // for(int i = 1; i < array_len; i++) {
    //     wave_array[i] = wave_array[i-1] + 3.3/array_len;
    // }

    // Sine wave
    for(int i = 1; i < array_len; i++) {
        wave_array[i] = 3.3*sin(2.0*3.14159*i/array_len);
    }

    int hertz = 2;

    while (true) {
        for(int i = 0; i < array_len; i++) {
            send_spi(wave_array[i]);
            sleep_ms(round(hertz*1000/array_len));
            printf("%f", wave_array[i]);
        }
    }
}
