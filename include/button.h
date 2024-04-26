#ifndef BUTTON_H
#define BUTTON_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include "drivers/gpio.h"
#include "drivers/timer.h"
#include "drivers/serial.h"

class Button {
    public:
        Button(uint8_t pin);

        bool is_pressed();

        void count_presses();
        void print_presses(uint16_t interval, Timer &timer, Serial &serial);

        static Button* instance;
        volatile uint32_t _button_presses;

    private:
        GPIO _gpio;
        uint32_t _prev_print_time;
        uint8_t _pin;
};


#endif // BUTTON_H