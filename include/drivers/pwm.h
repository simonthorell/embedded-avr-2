#ifndef PWM_H
#define PWM_H

#include <avr/io.h>
#include "config.h"

class PWModulation {
public:
    // Constructor
    PWModulation(uint8_t &pwmPin); // Constructor

    bool init();
    void set_duty_cycle(uint8_t duty);
    void adjust_duty_cycle(int error);

private:
    uint8_t _pin;
    volatile uint16_t* _ocr16; // 16-bit output compare register (timer 1)
    volatile uint8_t* _ocr8;   // 8-bit output compare register (timer 0, 2)
    uint8_t _duty_cycle;

    // Validation methods (compile-time checks)
    static constexpr bool valid_pin(uint8_t _pin);
};

#endif // PWM_H