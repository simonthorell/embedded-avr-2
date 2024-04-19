//======================================================================
// Timer Driver Class Implementation
//======================================================================
#include "drivers/timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "drivers/config.h"

// Definition for static member variable
volatile bool Timer::_is_completed = false;
volatile uint32_t Timer::milliseconds = 0;

//======================================================================
// Timer Constructor
// Description: Accepts the timer number and CPU frequency as arguments
//              and initializes the timer based on the timer number.
//======================================================================
Timer::Timer(uint8_t timer_num, uint8_t timer_type, uint32_t f_cpu) 
  : _f_cpu(f_cpu), _timer_num(timer_num), _timer_type(timer_type) {

    switch (_timer_type) {
        case MILLIS:
            init_millis();
            break;
    }   
}

//======================================================================
// Timer Setup Methods
//======================================================================
void Timer::init_millis() {
    cli(); // Disable interrupts during setup
    switch (_timer_num) {
    case TIMER_0:
        TCCR0A = 0;
        TCCR0B = 0;
        TCCR0B |= (1 << CS01) | (1 << CS00);            // Prescaler 64
        TCNT0 = 256 - (_f_cpu / PRESCALER / MS_TO_SEC); // Preload for 1ms
        TIMSK0 |= (1 << TOIE0);                         // Enable overflow interrupt
        break;
    case TIMER_1:
        TCCR1A = 0;
        TCCR1B = 0;
        TCCR1B |= (1 << CS11) | (1 << CS10);              // Prescaler 64
        TCNT1 = 65536 - (_f_cpu / PRESCALER / MS_TO_SEC); // Preload for 1ms
        TIMSK1 |= (1 << TOIE1);                           // Enable overflow interrupt
        break;
    case TIMER_2:
        TCCR2A = 0;
        TCCR2B = 0;
        TCCR2B |= (1 << CS22); // Set prescaler to 64 for Timer 2
        TCNT2 = 256 - (_f_cpu / PRESCALER / MS_TO_SEC); // Preload for 1ms
        TIMSK2 |= (1 << TOIE2);                         // Enable overflow interrupt
        break;
    }
    sei(); // Re-enable interrupts
}

//======================================================================
// Timer Public Methods
//======================================================================
void Timer::increment() {
    milliseconds++;
}

bool Timer::checkInterval(uint16_t ms) {
    if (milliseconds >= ms) {
        milliseconds = 0; // Reset the timer
        return true;
    }
    return false;
}

void Timer::complete() {
    _is_completed = true;
}

//======================================================================
// Timer ISR Implementation
//======================================================================

ISR(TIMER0_OVF_vect) {
    Timer::increment();
    Timer::complete();
}

ISR(TIMER1_OVF_vect) {
    Timer::increment();
    Timer::complete();
    TCNT1 = 0xFF06; // Reload timer (only needed for Timer 1 - why?)
}

ISR(TIMER2_OVF_vect) {
    Timer::increment();
    Timer::complete();
}