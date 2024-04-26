#ifndef PWM_H
#define PWM_H

#include <avr/io.h>
#include "drivers/timer.h"

//==============================================================================
// Macors for configuring PWM pins and registers
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
// Class definition
//==============================================================================
class PWModulation {
public:
    // Constructor
    PWModulation(const uint8_t &pwmPin); // Constructor

    bool init();
    void reset();
    void set_duty_cycle(uint8_t duty);
    void ramp_output(const uint16_t &cycle_time, Timer &timer);
    uint8_t _duty_cycle;

private:
    uint8_t _pin;
    volatile uint16_t* _ocr16; // 16-bit output compare register (timer 1)
    volatile uint8_t* _ocr8;   // 8-bit output compare register (timer 0, 2)
    // uint8_t _duty_cycle;

    // Variables for ramp method
    unsigned long _overflow_counter; 
    bool _ramp_up;

    // Validation methods (compile-time checks)
    static constexpr bool _valid_pwm_pin(uint8_t _pin);
};

#endif // PWM_H