#ifndef PWM_H
#define PWM_H

#include <avr/io.h>
#include "drivers/timer.h"

//==============================================================================
// PWM Class Declaration
//==============================================================================
class PWModulation {
friend class LED;
public:
    PWModulation(const uint8_t &pwmPin); // Constructor
    bool init();
    void reset();
    void set_duty_cycle(uint8_t duty);
    void ramp_output(const uint16_t &cycle_time, Timer &timer);

protected:
    uint8_t _duty_cycle;

private:
    uint8_t _pin;
    volatile uint16_t* _ocr16; // 16-bit output compare register (timer 1)
    volatile uint8_t* _ocr8;   // 8-bit output compare register (timer 0, 2)

    // Variables for ramp method
    unsigned long _overflow_counter; 
    bool _ramp_up;

    // Validation methods (compile-time checks)
    static constexpr bool _valid_pwm_pin(uint8_t _pin);
};

#endif // PWM_H