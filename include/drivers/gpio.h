#ifndef GPIO_H
#define GPIO_H

#include "config.h"  // Include the definitions and macros from config.h

class GPIO {
public:
    GPIO(PinType pin_type, uint8_t pin);  // Constructor
    void enable_output();
    void enable_input();
    void set_high();
    void set_low();
    void toggle();
    bool is_high();
    bool is_low();

private:
    PinType _pin_type; // Pin type (DIGITAL_PIN or ANALOG_PIN)
    uint8_t _pin;      // Pin number
};

#endif // GPIO_H