#ifndef GPIO_H
#define GPIO_H

#include <avr/io.h>

enum PinType { DIGITAL_PIN, ANALOG_PIN };

// Macro to obtain a pointer to the appropriate DDR register for a given pin
#define DDR_FOR_PIN(pinType, pin) \
    ((pinType) == DIGITAL_PIN ? \
        ((pin) <= 7 ? &DDRD : \
         (pin) >= 8 && (pin) <= 13 ? &DDRB : nullptr) : \
     (pinType) == ANALOG_PIN ? \
        ((pin) <= 5 ? &DDRC : nullptr) : nullptr)

// Macro to obtain a pointer to the appropriate PORT register for a given pin
#define PORT_FOR_PIN(pinType, pin) \
    ((pinType) == DIGITAL_PIN ? \
        ((pin) <= 7 ? &PORTD : \
         (pin) >= 8 && (pin) <= 13 ? &PORTB : nullptr) : \
     (pinType) == ANALOG_PIN ? \
        ((pin) <= 5 ? &PORTC : nullptr) : nullptr)

// Macro to determine the correct bit within a register for a given pin
#define BIT_FOR_PIN(pinType, pin) \
    ((pinType) == DIGITAL_PIN ? \
        ((pin) == 0 ?  PORTD0 : \
         (pin) == 1 ?  PORTD1 : \
         (pin) == 2 ?  PORTD2 : \
         (pin) == 3 ?  PORTD3 : \
         (pin) == 4 ?  PORTD4 : \
         (pin) == 5 ?  PORTD5 : \
         (pin) == 6 ?  PORTD6 : \
         (pin) == 7 ?  PORTD7 : \
         (pin) == 8 ?  PORTB0 : \
         (pin) == 9 ?  PORTB1 : \
         (pin) == 10 ? PORTB2 : \
         (pin) == 11 ? PORTB3 : \
         (pin) == 12 ? PORTB4 : \
         (pin) == 13 ? PORTB5 : 0) : \
     (pinType) == ANALOG_PIN ? \
        ((pin) == 0 ?  PORTC0 : \
         (pin) == 1 ?  PORTC1 : \
         (pin) == 2 ?  PORTC2 : \
         (pin) == 3 ?  PORTC3 : \
         (pin) == 4 ?  PORTC4 : \
         (pin) == 5 ?  PORTC5 : 0) : 0)

//=============================================================================
// Class definition
//=============================================================================
class GPIO {
public:
    GPIO(PinType pin_type, uint8_t pin);  // Constructor
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