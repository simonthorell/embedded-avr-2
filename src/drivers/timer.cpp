//======================================================================
// Timer Driver Class Implementation
//======================================================================
#include "drivers/timer.h"

// Static pointer initialization
Timer* Timer::timer_0_ptr = nullptr;
Timer* Timer::timer_1_ptr = nullptr;
Timer* Timer::timer_2_ptr = nullptr;

//======================================================================
// Timer Constructor
// Description: Accepts the timer number and CPU frequency as arguments
//              and initializes the timer based on the timer number.
//======================================================================
Timer::Timer(TimerType type, TimeUnit unit) 
  : _type(type), _unit(unit), overflow_counter(0) {
    /* Initialize the timer based on the timer number
       and set the timer pointer based on the timer type */
    switch (_type) {
        case TIMER_0:
            _init_timer_0(_unit);
            timer_0_ptr = this;
            break;
        case TIMER_1:
            _init_timer_1(_unit);
            timer_1_ptr = this;
            break;
        case TIMER_2:
            _init_timer_2(_unit);
            timer_2_ptr = this;
            break;
    }
}

//======================================================================
// Timer Public Methods
//======================================================================
void Timer::start() {
    // Enable Timer Compare Match A interrupt
    switch (_type) {
        case TIMER_0: TIMSK0 |= (1 << OCIE0A); break;
        case TIMER_1: TIMSK1 |= (1 << OCIE1A); break;
        case TIMER_2: TIMSK2 |= (1 << OCIE2A); break;
    }
}

void Timer::stop() {
    // Disable Timer Compare Match A interrupt
    switch (_type) {
        case TIMER_0: TIMSK0 &= ~(1 << OCIE0A); break;
        case TIMER_1: TIMSK1 &= ~(1 << OCIE1A); break;
        case TIMER_2: TIMSK2 &= ~(1 << OCIE2A); break;
    }
}

void Timer::_init_timer_0(TimeUnit unit) {
    TCCR0A = (1 << WGM01);  // CTC Mode
    TCCR0B = 0x00;          // Stop the timer

    if (unit == MICROS) {
        // Set compare match register for ~1us @16MHz
        OCR0A = OCR_MICROS;
        TCCR0B |= ((1 << CS00));  // No prescaling
    } else {
        // Set compare match register for ~1ms @16MHz
        OCR0A = OCR_MILLIS;
        TCCR0B |= ((1 << CS01) | (1 << CS00));  // Prescaler 64
    }
}

void Timer::_init_timer_1(TimeUnit unit) {
    TCCR1A = 0x00;          // Stop the timer
    TCCR1B = (1 << WGM12);  // CTC Mode

    // Stop the timer
    TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));

    if (unit == MICROS) {
        // Set compare match register for ~1us @16MHz
        OCR1A = OCR_MICROS;
        TCCR1B |= (1 << CS10);  // No prescaling
    } else {
        // Set compare match register for ~1ms @16MHz
        OCR1A = OCR_MILLIS;
        TCCR1B |= (1 << CS10) | (1 << CS11);  // Prescaler 64
    }
}

void Timer::_init_timer_2(TimeUnit unit) {
    TCCR2A = (1 << WGM21);  // CTC Mode
    TCCR2B = 0x00;          // Stop the timer

    if (unit == MICROS) {
        // Set compare match register for ~1us @16MHz
        OCR2A = OCR_MICROS;
        TCCR2B |= (1 << CS20);  // No prescaling
    } else {
        // Set compare match register for ~1ms @16MHz
        OCR2A = OCR_MILLIS;
        TCCR2B |= (1 << CS22);  // Prescaler 64
    }
}

//======================================================================
// Timer ISR Implementations
//======================================================================
ISR(TIMER0_COMPA_vect) {
    // Only increment the overflow counter if Timer0 is initialized
    if (Timer::timer_0_ptr) {
        Timer::timer_0_ptr->overflow_counter++;
    }
}

ISR(TIMER1_COMPA_vect) {
    // Only increment the overflow counter if Timer1 is initialized
    if (Timer::timer_1_ptr) {
        Timer::timer_1_ptr->overflow_counter++;
    }
}

ISR(TIMER2_COMPA_vect) {
    // Only increment the overflow counter if Timer2 is initialized
    if (Timer::timer_2_ptr) {
        Timer::timer_2_ptr->overflow_counter++;
    }
}