#ifndef GPIO_H
#define GPIO_H

#include <avr/io.h>

//=============================================================================
// GPIO Class Declaration
//=============================================================================
class GPIO {
public:
    enum PinType { DIGITAL_PIN, ANALOG_PIN };

    // Constructor
    GPIO(PinType pin_type, uint8_t pin);

    // Public methods
    void enable_output();
    void enable_input();
    void enable_pullup();
    void set_high();
    void set_low();
    void toggle();
    bool is_high();
    bool is_low();

private:
    PinType _pin_type; // Pin type (DIGITAL_PIN or ANALOG_PIN)
    uint8_t _pin;      // Pin number (0-13 for digital, 0-5 for analog)
};

#endif // GPIO_H