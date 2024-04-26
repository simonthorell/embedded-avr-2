

//=============================================================================
// Timer Driver Class Implementation
//=============================================================================
#include "drivers/timer.h"
#include <util/atomic.h>
#include <stdio.h> // For sprintf 

// Static pointer initialization
Timer* Timer::instance = nullptr;

//=============================================================================
// Timer Constructor
// Description: Accepts the timer type (0,1,2) and the needed time unit 
//              (MICROS, MILLIS) and initializes the timer accordingly.
//=============================================================================
Timer::Timer(TimerNum num, TimeUnit unit) 
  : overflow_counter(0), _num(num), _unit(unit) {
    instance = this;
}

//=============================================================================
// Timer Public Method: init
// Description: Initialize the timer with the given interval and serial object.
//=============================================================================
void Timer::init(const uint32_t &interval, Serial &serial) {
    // Clear the timer before setting the mode and prescaler
    _clear_tccr();
    // Set the timer mode to CTC
    switch (_num) {
        case TIMER0: TCCR0A = (1 << WGM01); break;
        case TIMER1: TCCR1B = (1 << WGM12); break;
        case TIMER2: TCCR2A = (1 << WGM21); break;
    }
    // Set the prescaler based on the interval
    set_prescaler(interval, serial);
}

//=============================================================================
// Timer Public Methods: set_prescaler
// Description: Set the prescaler based on the interval and the time unit
//              (MICROS, MILLIS). The prescaler is set based on the interval
//              and the threshold values for milliseconds and microseconds.
//=============================================================================
void Timer::set_prescaler(uint32_t interval, Serial &serial) {
    stop(); // Stop the timer before setting the prescaler
    cli();  // Disable interrupts temporarily
    TimerConfig config = _clear_tccr(); // Clear & store TCCR

    // Prescaler settings for ms and us (threshold)
    static const PrescalerSetting ms_settings[] = {
        // Prescaler settings for milliseconds (no need for prescaler 1)
        {30, 8}, {250, 64}, {1000, 256}, {UINT32_MAX, 1024}
    };
    static const PrescalerSetting us_settings[] = {
        {3000, 1}, {30000, 8}, {250000, 64}, {1000000, 256}, {UINT32_MAX, 1024}
    };

    // Choose the correct settings based on the unit
    const PrescalerSetting* settings = (_unit == MICROS) ? 
        us_settings : ms_settings;
    const size_t num_settings = (_unit == MICROS) ? 
        (sizeof(us_settings) / sizeof(us_settings[0])) : 
        (sizeof(ms_settings) / sizeof(ms_settings[0]));
    
    uint16_t prescaler = 1024;  // Default to the highest prescaler

    // Check the interval vs thresholds to set the prescaler
    for (size_t i = 0; i < num_settings; ++i) {
        if (interval < settings[i].threshold) {
            prescaler = settings[i].prescaler;
            break;
        }
    }

    uint32_t ocr_value = ((F_CPU / prescaler) * (interval / 
                           (_unit == MICROS ? US_PER_SEC : MS_PER_SEC)) - 1);

    // Inform user about the set pre-scaler and OCR value
    char message[64];
        snprintf(
        message, sizeof(message),
        "Timer %d configured (Prescaler: %d, OCR: %lu)\r\n",
        _num, prescaler, ocr_value
    );
    serial.uart_put_str(message);

    // Set the register values based on the set timer number
    switch (_num) {
        case TIMER0: 
            OCR0A = ocr_value;
            TCCR0B |= TIMER0_PS_BITS(prescaler);
            break;
        case TIMER1:
            OCR1A = ocr_value;
            TCCR1B |= TIMER1_PS_BITS(prescaler);
            break;
        case TIMER2: 
            OCR2A = ocr_value;
            TCCR2B |= TIMER2_PS_BITS(prescaler);
            break;
    }

    _set_tccr(config); // Revert TCCR (WGM) settings
    sei();            // Re-enable interrupts
    start();          // Start the timer again
}

//=============================================================================
// Timer Public Methods: start, stop, reset
//=============================================================================
void Timer::start() {
     // Enable Timer Compare Match A interrupt
    switch (_num) {
        case TIMER0: TIMSK0 |= (1 << OCIE0A); break;
        case TIMER1: TIMSK1 |= (1 << OCIE1A); break;
        case TIMER2: TIMSK2 |= (1 << OCIE2A); break;
    }
}

void Timer::stop() {
    // Disable Timer Compare Match A interrupt
    switch (_num) {
        case TIMER0: TIMSK0 &= ~(1 << OCIE0A); break;
        case TIMER1: TIMSK1 &= ~(1 << OCIE1A); break;
        case TIMER2: TIMSK2 &= ~(1 << OCIE2A); break;
    }
}

// Reset the overflow counter (Reset the timer)
void Timer::reset() {
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        overflow_counter = 0;
        timer_overflowed = false;
    }
}

//=============================================================================
// Timer ISR Implementations
// Description: Increment overflow counter on timer compare match interrupt.
//              Only the ISR started with start() (e.g., TMSK0 |= (1 << OCIE0A);) 
//              will be called. overflow_counter resets to 0 on overflow.
//=============================================================================
ISR(TIMER0_COMPA_vect) {
    Timer::instance->overflow_counter++;
    Timer::instance->timer_overflowed = true;
}

ISR(TIMER1_COMPA_vect) {
    Timer::instance->overflow_counter++;
    Timer::instance->timer_overflowed = true;
}

ISR(TIMER2_COMPA_vect) {
    Timer::instance->overflow_counter++;
    Timer::instance->timer_overflowed = true;
}

//=============================================================================
// Timer Private Methods: _clear_tccr, _set_tccr
// Description: These methods are used to clear and set the TCCR registers
//              for the timer. The TCCR registers are used to set the mode
//              and prescaler of the timer.
//=============================================================================
TimerConfig Timer::_clear_tccr() {
    TimerConfig config;
    switch (_num) {
        case TIMER0: 
            config.tccrA = TCCR0A;
            config.tccrB = TCCR0B;
            // Clear all the pre-scaler bits from the variables:
            config.tccrB &= ~((1 << CS02) | (1 << CS01) | (1 << CS00));

            TCCR0A = 0x00;
            TCCR0B = 0x00;
            break;
        case TIMER1: 
            config.tccrA = TCCR1A;
            config.tccrB = TCCR1B;
            // Clear all the pre-scaler bits from the variables:
            config.tccrB &= ~((1 << CS10) | (1 << CS11) | (1 << CS12));
            TCCR1A = 0x00;
            TCCR1B = 0x00;
            break;
        case TIMER2: 
            config.tccrA = TCCR2A;
            config.tccrB = TCCR2B;
            // Clear all the pre-scaler bits from the variables:
            config.tccrB &= ~((1 << CS20) | (1 << CS21) | (1 << CS22));
            TCCR2A = 0x00;
            TCCR2B = 0x00;
            break;
    }
    return config;
}

void Timer::_set_tccr(const TimerConfig &config) {
    // Adds the previous settings back to the TCCR registers
    // For example PWM settings!
    switch (_num) {
        case TIMER0: 
            TCCR0A |= config.tccrA;
            TCCR0B |= config.tccrB;
            break;
        case TIMER1: 
            TCCR1A |= config.tccrA;
            TCCR1B |= config.tccrB;
            break;
        case TIMER2: 
            TCCR2A |= config.tccrA;
            TCCR2B |= config.tccrB;
            break;
    }
}


/*
#include "drivers/timer.h"
#include <util/atomic.h>
#include <stdio.h> // For sprintf

// Definitions of prescaler bits macros for each timer
#define TIMER0_PS_BITS(prescaler) \
    ((prescaler) == 1 ?     (1 << CS00) : \
    (prescaler)  == 8 ?     (1 << CS01) : \
    (prescaler)  == 64 ?   ((1 << CS01) | (1 << CS00)) : \
    (prescaler)  == 256 ?   (1 << CS02) : \
    (prescaler)  == 1024 ? ((1 << CS02) | (1 << CS00)) : \
    0)

#define TIMER1_PS_BITS(prescaler) \
    ((prescaler) == 1 ?     (1 << CS10) : \
    (prescaler)  == 8 ?     (1 << CS11) : \
    (prescaler)  == 64 ?   ((1 << CS11) | (1 << CS10)) : \
    (prescaler)  == 256 ?   (1 << CS12) : \
    (prescaler)  == 1024 ? ((1 << CS12) | (1 << CS10)) : \
    0)

#define TIMER2_PS_BITS(prescaler) \
    ((prescaler) == 1 ?     (1 << CS20) : \
    (prescaler)  == 8 ?     (1 << CS21) : \
    (prescaler)  == 32 ?   ((1 << CS21) | (1 << CS20)) : \
    (prescaler)  == 64 ?    (1 << CS22) : \
    (prescaler)  == 128 ?  ((1 << CS22) | (1 << CS20)) : \
    (prescaler)  == 256 ?  ((1 << CS22) | (1 << CS21)) : \
    (prescaler)  == 1024 ? ((1 << CS22) | (1 << CS21) | (1 << CS20)) : \
    0)

#define US_PER_SEC   1000000UL   // us per second
#define MS_PER_SEC   1000.0      // ms per second

// Static instance initialization
Timer* Timer::instance = nullptr;

Timer::Timer(timer_num num, time_unit unit) 
  : overflow_counter(0), num(num), unit(unit) {
    instance = this;  // Assign this instance to the static pointer
}

Timer::~Timer() {
    stop(); // Ensure the timer is stopped on destruction
}

void Timer::init(uint32_t interval, Serial& serial) {
    clear_timer_registers();
    set_prescaler(interval);
    configure_timer();

    // Print value of prescaler and OCR value
    char message[64];
    sprintf(message, "Timer %d configured (Prescaler: %d, OCR: %lu)\r\n", num, _prescaler, _ocr_value);
    serial.uart_put_str(message);
}

void Timer::start() {
    sei(); // Enable global interrupts
    switch (num) {
        case TIMER0: TIMSK0 |= (1 << OCIE0A); break;
        case TIMER1: TIMSK1 |= (1 << OCIE1A); break;
        case TIMER2: TIMSK2 |= (1 << OCIE2A); break;
    }
}

void Timer::stop() {
    cli(); // Disable global interrupts
    switch (num) {
        case TIMER0: TIMSK0 &= ~(1 << OCIE0A); break;
        case TIMER1: TIMSK1 &= ~(1 << OCIE1A); break;
        case TIMER2: TIMSK2 &= ~(1 << OCIE2A); break;
    }
}

void Timer::reset() {
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        overflow_counter = 0;
        timer_overflowed = false;
    }
}

void Timer::set_prescaler(uint32_t interval) {
    _prescaler = 1024; // Default prescaler
    _ocr_value = 0;    // Default OCR value
    _ocr_value = ((F_CPU / _prescaler) * (interval / 
                            (unit == MICROS ? US_PER_SEC : MS_PER_SEC)) - 1);
    uint8_t prescaler_bits = 0;

    switch (num) {
        case TIMER0:
            prescaler_bits = TIMER0_PS_BITS(_prescaler);
            break;
        case TIMER1:
            prescaler_bits = TIMER1_PS_BITS(_prescaler);
            break;
        case TIMER2:
            prescaler_bits = TIMER2_PS_BITS(_prescaler);
            break;
    }
    set_timer_registers(prescaler_bits, _ocr_value);
}

void Timer::configure_timer() {
    // Configure timer in CTC mode
    switch (num) {
        case TIMER1:
            TCCR1B |= (1 << WGM12);  // Set CTC mode for Timer1
            break;
        case TIMER0:
        case TIMER2:
            // Set the specific mode for Timer0 and Timer2 if needed
            break;
    }
}

void Timer::clear_timer_registers() {
    // Clear configuration registers for Timer1
    switch (num) {
        case TIMER0:
            TCCR0A = 0; TCCR0B = 0;
            break;
        case TIMER1:
            TCCR1A = 0; TCCR1B = 0;
            break;
        case TIMER2:
            TCCR2A = 0; TCCR2B = 0;
            break;
    }
}

void Timer::set_timer_registers(uint8_t prescaler_bits, uint16_t ocr_value) {
    // Set prescaler and compare match register for Timer1
    switch (num) {
        case TIMER0:
            OCR0A = ocr_value; TCCR0B |= prescaler_bits;
            break;
        case TIMER1:
            OCR1A = ocr_value; TCCR1B |= prescaler_bits;
            break;
        case TIMER2:
            OCR2A = ocr_value; TCCR2B |= prescaler_bits;
            break;
    }
}

ISR(TIMER0_COMPA_vect) {
    Timer::instance->overflow_counter++;
    Timer::instance->timer_overflowed = true;
}

ISR(TIMER1_COMPA_vect) {
    Timer::instance->overflow_counter++;
    Timer::instance->timer_overflowed = true;
}

ISR(TIMER2_COMPA_vect) {
    Timer::instance->overflow_counter++;
    Timer::instance->timer_overflowed = true;
}
*/