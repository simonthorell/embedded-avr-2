#ifndef LED_H
#define LED_H

// #include <stdlib.h>
#include <stdio.h>

#include "drivers/gpio.h"
#include "drivers/timer.h"
#include "drivers/adc.h"
#include "drivers/serial.h"

class LED {
public:
    LED(uint8_t pin);  // Constructor that specifies the pin connected to the LED
    void turn_on();
    void turn_off();
    void toggle();
    bool is_on();
    bool is_off();
    void blink(const uint16_t &blink_time, const Timer &timer);
    void adc_blink(const Timer &timer, Serial &serial, const uint8_t &adc_ch, 
                   const uint16_t &max_interval);

private:
    GPIO _gpio;
    ADConverter adc;
    unsigned long _overflow_counter = 0;    // timer overflow counter
    uint16_t _blink_interval = 200;
    uint16_t _prev_blink_interval = 0;
};

#endif // LED_H