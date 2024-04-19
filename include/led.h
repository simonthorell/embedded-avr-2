#ifndef LED_H
#define LED_H

#include "drivers/gpio.h"

class LED {
public:
    LED(uint8_t pin);  // Constructor that specifies the pin connected to the LED
    void turn_on();
    void turn_off();
    void toggle();
    bool is_on();
    bool is_off();

private:
    GPIO _gpio;  // GPIO object to manage pin operations
};

#endif // LED_H