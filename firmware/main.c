#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

#define HOST_UART uart0
#define HOST_BAUD 115200
#define HOST_TX 0
#define HOST_RX 1

#define DAC_I2C i2c0
#define DAC_SDA 4
#define DAC_SCL 5
#define DAC_ADDR 0x60

static const uint8_t digital_gpio[] = {10, 11, 12, 13, 14, 15, 16, 17};
static const uint8_t led_gpio[] = {35, 34, 33};
static const uint8_t switch_gpio[] = {36, 37, 38, 39};
static const uint8_t relay_gpio[] = {24, 25};
static uint16_t dac_raw[] = {0, 0, 0, 0};

static void reply_ok(void) {
    uart_puts(HOST_UART, "OK\n");
}

static void reply_value(uint32_t value) {
    char out[24];
    snprintf(out, sizeof(out), "OK %lu\n", (unsigned long)value);
    uart_puts(HOST_UART, out);
}

static void reply_err(void) {
    uart_puts(HOST_UART, "ERR\n");
}

static bool parse_u32(char **p, uint32_t *value) {
    while (**p == ' ') {
        ++*p;
    }
    if (!isdigit((unsigned char)**p)) {
        return false;
    }
    char *end = NULL;
    unsigned long parsed = strtoul(*p, &end, 10);
    if (end == *p) {
        return false;
    }
    *p = end;
    *value = (uint32_t)parsed;
    return true;
}

static bool valid_index(uint32_t index, size_t count) {
    return index < count;
}

static void dac_write_all(void) {
    uint8_t buf[8];
    for (int ch = 0; ch < 4; ++ch) {
        uint16_t value = dac_raw[ch] & 0x0fff;
        buf[ch * 2] = (uint8_t)(value >> 8);
        buf[ch * 2 + 1] = (uint8_t)value;
    }
    i2c_write_blocking(DAC_I2C, DAC_ADDR, buf, sizeof(buf), false);
}

static void handle_command(char *line) {
    char *p = line;
    while (*p == ' ') {
        ++p;
    }

    if (!strcmp(p, "PING")) {
        uart_puts(HOST_UART, "OK RTSHIELD 1\n");
        return;
    }

    if (!strncmp(p, "AI", 2)) {
        p += 2;
        uint32_t ch;
        if (!parse_u32(&p, &ch) || ch > 7) {
            reply_err();
            return;
        }
        adc_select_input(ch);
        reply_value(adc_read());
        return;
    }

    if (!strncmp(p, "AO", 2)) {
        p += 2;
        uint32_t ch, value;
        if (!parse_u32(&p, &ch) || !parse_u32(&p, &value) || ch > 3 || value > 4095) {
            reply_err();
            return;
        }
        dac_raw[ch] = (uint16_t)value;
        dac_write_all();
        reply_ok();
        return;
    }

    if (!strncmp(p, "DI", 2)) {
        p += 2;
        uint32_t pin;
        if (!parse_u32(&p, &pin) || pin < 2 || pin > 9) {
            reply_err();
            return;
        }
        reply_value(gpio_get(digital_gpio[pin - 2]));
        return;
    }

    if (!strncmp(p, "DO", 2)) {
        p += 2;
        uint32_t pin, value;
        if (!parse_u32(&p, &pin) || !parse_u32(&p, &value) || pin < 2 || pin > 9 || value > 1) {
            reply_err();
            return;
        }
        gpio_set_dir(digital_gpio[pin - 2], GPIO_OUT);
        gpio_put(digital_gpio[pin - 2], value);
        reply_ok();
        return;
    }

    if (!strncmp(p, "DM", 2)) {
        p += 2;
        uint32_t pin, output;
        if (!parse_u32(&p, &pin) || !parse_u32(&p, &output) || pin < 2 || pin > 9 || output > 1) {
            reply_err();
            return;
        }
        gpio_set_dir(digital_gpio[pin - 2], output ? GPIO_OUT : GPIO_IN);
        reply_ok();
        return;
    }

    if (!strncmp(p, "LED", 3)) {
        p += 3;
        uint32_t led, value;
        if (!parse_u32(&p, &led) || !parse_u32(&p, &value) || !valid_index(led, 3) || value > 1) {
            reply_err();
            return;
        }
        gpio_put(led_gpio[led], value);
        reply_ok();
        return;
    }

    if (!strncmp(p, "SW", 2)) {
        p += 2;
        uint32_t sw;
        if (!parse_u32(&p, &sw) || !valid_index(sw, 4)) {
            reply_err();
            return;
        }
        reply_value(!gpio_get(switch_gpio[sw]));
        return;
    }

    if (!strncmp(p, "REL", 3)) {
        p += 3;
        uint32_t relay, value;
        if (!parse_u32(&p, &relay) || !parse_u32(&p, &value) || !valid_index(relay, 2) || value > 1) {
            reply_err();
            return;
        }
        gpio_put(relay_gpio[relay], value);
        reply_ok();
        return;
    }

    reply_err();
}

static void init_io(void) {
    uart_init(HOST_UART, HOST_BAUD);
    gpio_set_function(HOST_TX, GPIO_FUNC_UART);
    gpio_set_function(HOST_RX, GPIO_FUNC_UART);

    adc_init();
    for (uint8_t gpio = 40; gpio <= 47; ++gpio) {
        adc_gpio_init(gpio);
    }

    i2c_init(DAC_I2C, 400000);
    gpio_set_function(DAC_SDA, GPIO_FUNC_I2C);
    gpio_set_function(DAC_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(DAC_SDA);
    gpio_pull_up(DAC_SCL);

    for (size_t i = 0; i < sizeof(digital_gpio); ++i) {
        gpio_init(digital_gpio[i]);
        gpio_set_dir(digital_gpio[i], GPIO_IN);
    }
    for (size_t i = 0; i < sizeof(led_gpio); ++i) {
        gpio_init(led_gpio[i]);
        gpio_set_dir(led_gpio[i], GPIO_OUT);
        gpio_put(led_gpio[i], 0);
    }
    for (size_t i = 0; i < sizeof(switch_gpio); ++i) {
        gpio_init(switch_gpio[i]);
        gpio_set_dir(switch_gpio[i], GPIO_IN);
        gpio_pull_up(switch_gpio[i]);
    }
    for (size_t i = 0; i < sizeof(relay_gpio); ++i) {
        gpio_init(relay_gpio[i]);
        gpio_set_dir(relay_gpio[i], GPIO_OUT);
        gpio_put(relay_gpio[i], 0);
    }
    dac_write_all();
}

int main(void) {
    init_io();

    char line[64];
    size_t len = 0;
    while (true) {
        char c = uart_getc(HOST_UART);
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            line[len] = 0;
            handle_command(line);
            len = 0;
            continue;
        }
        if (len + 1 < sizeof(line)) {
            line[len++] = c;
        }
    }
}
