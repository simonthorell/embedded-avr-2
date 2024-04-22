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
// Description: Accepts the timer type (0,1,2) and the needed time unit 
//              (MICROS, MILLIS) and initializes the timer accordingly.
//======================================================================
Timer::Timer(TimerNum num, TimeUnit unit) 
  : _num(num), _unit(unit), overflow_counter(0) {
    switch (_num) {
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
// Timer Public Methods: start, stop, reset
//======================================================================

 // Enable Timer Compare Match A interrupt
void Timer::start() {
    switch (_num) {
        case TIMER_0: TIMSK0 |= (1 << OCIE0A); break;
        case TIMER_1: TIMSK1 |= (1 << OCIE1A); break;
        case TIMER_2: TIMSK2 |= (1 << OCIE2A); break;
    }
}

// Disable Timer Compare Match A interrupt
void Timer::stop() {
    switch (_num) {
        case TIMER_0: TIMSK0 &= ~(1 << OCIE0A); break;
        case TIMER_1: TIMSK1 &= ~(1 << OCIE1A); break;
        case TIMER_2: TIMSK2 &= ~(1 << OCIE2A); break;
    }
}

// Reset the overflow counter (Reset the timer)
void Timer::reset() {
    cli(); // Disable interrupts temporarily
    overflow_counter = 0;
    sei(); // Re-Enable interrups
}

//======================================================================
// Timer Private Methods: _init_timer_0, _init_timer_1, _init_timer_2
//======================================================================
void Timer::_init_timer_0(TimeUnit unit) {
    TCCR0A = (1 << WGM01);  // CTC Mode
    TCCR0B = 0x00;          // Stop the timer

    if (unit == MICROS) {
        // Set compare match register for ~1us @16MHz
        OCR0A = _ocr_micros;
        TCCR0B |= ((1 << CS00));  // No prescaling
    } else {
        // Set compare match register for ~1ms @16MHz
        OCR0A = _ocr_millis;
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
        OCR1A = _ocr_micros;
        TCCR1B |= (1 << CS10);  // No prescaling
    } else {
        // Set compare match register for ~1ms @16MHz
        OCR1A = _ocr_millis;
        TCCR1B |= (1 << CS10) | (1 << CS11);  // Prescaler 64
    }
}

void Timer::_init_timer_2(TimeUnit unit) {
    TCCR2A = (1 << WGM21);  // CTC Mode
    TCCR2B = 0x00;          // Stop the timer

    if (unit == MICROS) {
        // Set compare match register for ~1us @16MHz
        OCR2A = _ocr_micros;
        TCCR2B |= (1 << CS20);  // No prescaling
    } else if (unit == MILLIS) {
        // Set compare match register for ~1ms @16MHz
        OCR2A = _ocr_millis;
        TCCR2B |= (1 << CS22);  // Prescaler 64
    } else {
        // Overflow Timer
        TCCR2A = 0;            // Stop Timer2A
        TCCR2B = 0;            // Stop Timer2B
        TCCR2B |= 0B00000111;  // Prescaler = 1024
        TCNT2 = 5;             // Timer Preloading (256 - 5) = 251
    }
}

//======================================================================
// Timer ISR Implementations
// Description: These functions are called when the timer compare match
//              interrupt is triggered. The overflow counter is then
//              incremented to keep track of the timer overflows.
// Note: overflow_counter will automatically reset to 0 when overflowing
//       due to nature of C/C++ data types.
//======================================================================
ISR(TIMER0_COMPA_vect) {
    if (Timer::timer_0_ptr) {
        Timer::timer_0_ptr->overflow_counter++;
    }
}

ISR(TIMER1_COMPA_vect) {
    if (Timer::timer_1_ptr) {
        Timer::timer_1_ptr->overflow_counter++;
    }
}

ISR(TIMER2_COMPA_vect) {
    if (Timer::timer_2_ptr) {
        Timer::timer_2_ptr->overflow_counter++;
    }
}

//======================================================================
// Constexpr Definitions
// Description: These definitions are used to calculate the OCR values
//              for the timer based on the CPU frequency and prescaler.
//======================================================================
static constexpr uint32_t _ocr_micros = ((F_CPU / SEC_TO_US / PRESCALER_US) - 1);
static constexpr uint32_t _ocr_millis = ((F_CPU / SEC_TO_MS / PRESCALER_MS) - 1);