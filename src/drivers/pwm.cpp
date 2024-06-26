//==============================================================================
// PWM Interface Class Implementation
//==============================================================================
#include "drivers/pwm.h"

//==============================================================================
// PWM Configuration Macros
// Description: These are very difficult to make as efficient using methods...
//==============================================================================
#define PWM_DDR(pin) (*(&DDR##pin))
#define PWM_PORT(pin) (*(&PORT##pin))
#define PWM_PIN(pin) (*(&PIN##pin))

#define CONFIGURE_PWM_DDR(ddr, port) ((ddr) |= (1 << (port)))

#define CONFIGURE_PWM(ddr, port, ocr, com, wgm0, wgm1, cs) \
    ddr |= (1 << port),                      /* Set pin as output */ \
    TCCR##ocr##A |= (1 << com) | (1 << wgm0), /* Setup PWM mode */ \
    TCCR##ocr##B |= (1 << cs)  | (1 << wgm1)  /* Setup prescaler and additional mode bits */

#define SETUP_PWM_FOR_PIN(pin) \
    if      ((pin) == 3)  { CONFIGURE_PWM(DDRD, PORTD3, 2, COM2B1, WGM20, WGM21, CS21); } \
    else if ((pin) == 5)  { CONFIGURE_PWM(DDRD, PORTD5, 0, COM0B1, WGM00, WGM01, CS01); } \
    else if ((pin) == 6)  { CONFIGURE_PWM(DDRD, PORTD6, 0, COM0A1, WGM00, WGM01, CS01); } \
    else if ((pin) == 9)  { CONFIGURE_PWM(DDRB, PORTB1, 1, COM1A1, WGM10, WGM11, CS11); } \
    else if ((pin) == 10) { CONFIGURE_PWM(DDRB, PORTB2, 1, COM1B1, WGM10, WGM11, CS11); } \
    else if ((pin) == 11) { CONFIGURE_PWM(DDRB, PORTB3, 2, COM2A1, WGM20, WGM21, CS21); }

//==============================================================================
// Constructor
//==============================================================================
PWModulation::PWModulation(const uint8_t &pwm_pin) : _pin(pwm_pin) {
    if (_valid_pwm_pin(_pin)) {
        // Assign the correct output compare register based on the PWM pin
        switch (pwm_pin) {
            // Assign to _ocr8 or _ocr16 depending on the timer resolution
            case 3:  _ocr8  = &OCR2B; break;             // 8-bit
            case 5:  _ocr8  = &OCR0B; break;             // 8-bit
            case 6:  _ocr8  = &OCR0A; break;             // 8-bit
            case 9:  _ocr16 = (uint16_t*)&OCR1A; break;  // 16-bit
            case 10: _ocr16 = (uint16_t*)&OCR1B; break;  // 16-bit
            case 11: _ocr8  = &OCR2A; break;             // 8-bit
        }
    }
}

//==============================================================================
// Public Method: init, reset
// Description:   configure the PWM output pin and set the initial duty cycle
//==============================================================================
bool PWModulation::init() {
    if (_valid_pwm_pin(_pin)) {
        SETUP_PWM_FOR_PIN(_pin);   // Register bits defined in pwm.h
        reset();                   // Set the initial duty cycle
        return true;   // Valid pin
    } else {
        return false; // Invalid pin
    }
}

void PWModulation::reset() {
    set_duty_cycle(UINT8_MAX);  // Set duty cycle to 8-bit max (255)
}

//==============================================================================
// Public Method: setDutyCycle
// Description:   Set the duty cycle of the PWM signal. The duty cycle is
//                a value between 0 and 255, where 0 is 0% and 255 is 100%.
//==============================================================================
void PWModulation::set_duty_cycle(uint8_t duty) {
    _duty_cycle = duty; // Just store duty cycle value
    if (_ocr8) {
        *_ocr8 = (uint8_t)(duty & 0xFF);  // 8-bit OCR, cast to ensure no overflow
    } else {
        *_ocr16 = duty; // Update 16-bit OCR
    } 
}

//==============================================================================
// Public Method: rampOutput
// Description:   Ramp the PWM output up and down over a specified cycle time.
//                If you pass a cycle time of 1000ms and a millis timer, the
//                duty cycle will ramp up and down once every second.
//==============================================================================
void PWModulation::ramp_output(const uint16_t &cycle_time, Timer &timer) {
    // Ensure cycle_time is large enough to avoid division by zero
    if (cycle_time < 2 * UINT8_MAX) return;

    // Calculate the increase in overflow since last update
    uint16_t overflow_increase = timer.overflow_counter - _last_overflow_count;
    _overflow_counter += overflow_increase;

    // Update _last_overflow_count to the current timer.overflow_counter
    _last_overflow_count = timer.overflow_counter;

    // Calculate the number of timer overflows required for a single step
    uint16_t steps = (cycle_time / (UINT8_MAX * 2)) * 2; // *2 ramps per cycle

    if (_overflow_counter >= steps) {
        if (_ramp_up) {
            if (_duty_cycle < UINT8_MAX) {
                _duty_cycle++;
            } else {
                _ramp_up = false;
            }
        } else {
            if (_duty_cycle > 0) {
                _duty_cycle--;
            } else {
                _ramp_up = true;
            }
        }

        set_duty_cycle(_duty_cycle);
        _overflow_counter = 0; // Reset the overflow counter
    }
}

//==============================================================================
// Constexpr validation for PWM pins
// Description: Ensure the PWM pin is valid at compile time, the valid pins
//              are 3, 5, 6, 9, 10, and 11 on the Arduino Uno.
//==============================================================================
constexpr bool PWModulation::_valid_pwm_pin(const uint8_t _pin) {
    return _pin == 3 || _pin == 5 || _pin == 6 || 
           _pin == 9 || _pin == 10 || _pin == 11;
}