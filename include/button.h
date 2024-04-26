#ifndef BUTTON_H
#define BUTTON_H

#include "drivers/gpio.h"

class Button {
    public:
        Button(uint8_t pin);

        bool is_pressed();
    private:
        GPIO _gpio;

        uint8_t _pin;
};


#endif // BUTTON_H