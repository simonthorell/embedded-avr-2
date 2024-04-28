//======================================================================
// GPIO Driver Class Implementation
//======================================================================
#include "drivers/gpio.h"

//=============================================================================
// Macros for GPIO Configuration
//=============================================================================
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

#define PIN_FOR_PIN(pinType, pin) \
    ((pinType) == DIGITAL_PIN ? (pin < 8 ? &PIND : \
    (pin < 16 ? &PINB : &PINC)) : nullptr)

//======================================================================
// GPIO Constructor
//======================================================================
GPIO::GPIO(PinType pin_type, uint8_t pin) 
    : _pin_type(pin_type), _pin(pin) {}

//======================================================================
// GPIO Public Methods: enable_output, enable_input, set_high, set_low,
//                      toggle, is_high, is_low
//======================================================================
void GPIO::enable_output() {
    (*(DDR_FOR_PIN(_pin_type, _pin))  |=  (1 << BIT_FOR_PIN(_pin_type, _pin)));
}

void GPIO::enable_input() {
    (*(DDR_FOR_PIN(_pin_type, _pin))  &= ~(1 << BIT_FOR_PIN(_pin_type, _pin)));
}

void GPIO::enable_pullup() {
    (*(PORT_FOR_PIN(_pin_type, _pin)) |=  (1 << BIT_FOR_PIN(_pin_type, _pin)));
}

void GPIO::set_high() {
    (*(PORT_FOR_PIN(_pin_type, _pin)) |=  (1 << BIT_FOR_PIN(_pin_type, _pin)));
}

void GPIO::set_low() {
    (*(PORT_FOR_PIN(_pin_type, _pin)) &= ~(1 << BIT_FOR_PIN(_pin_type, _pin)));
}

void GPIO::toggle() {
    (*(PORT_FOR_PIN(_pin_type, _pin)) ^=  (1 << BIT_FOR_PIN(_pin_type, _pin)));
}

bool GPIO::is_high() {
    return (*(PIN_FOR_PIN(_pin_type, _pin)) & (1 << BIT_FOR_PIN(_pin_type, _pin)));
}

bool GPIO::is_low() {
    return !(*(PIN_FOR_PIN(_pin_type, _pin)) & (1 << BIT_FOR_PIN(_pin_type, _pin)));
}