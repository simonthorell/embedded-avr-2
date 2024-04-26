#ifndef LED_H
#define LED_H

// #include <stdlib.h>
#include <stdio.h>

#include "drivers/gpio.h"
#include "drivers/timer.h"
#include "drivers/adc.h"
#include "drivers/pwm.h"
#include "drivers/serial.h"

class LED {
public:
    enum PWM_MODE { PWM_OFF, PWM_ON };

    // Constructor that specifies the pin connected to the LED
    LED(uint8_t pin, bool enable_pwm);

    // Public Methods
    void turn_on();
    void turn_off();
    void toggle();
    bool is_on();
    bool is_off();
    void blink(const uint16_t &blink_time, const Timer &timer);
    void adc_blink(const Timer &timer, Serial &serial, const uint8_t &adc_ch, 
                   const uint16_t &max_interval);
    void set_power(const uint16_t &cycle_time);
    void ramp_brightness(const uint16_t &cycle_time, Timer &timer);

private:
    GPIO _gpio;
    ADConverter _adc;
    PWModulation _pwm;

    bool _pwm_enabled;
    uint8_t _pwm_power;
    unsigned long _overflow_counter;
    uint16_t _blink_interval;
    uint16_t _prev_blink_interval;
};

#endif // LED_H