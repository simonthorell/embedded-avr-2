//=============================================================================
// Timer Driver Class Implementation
//=============================================================================
#include "drivers/timer.h"

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

#ifndef F_CPU
#define F_CPU 16000000UL // Define default MCU clock speed if not defined
#endif

#define US_PER_SEC   1000000UL   // us per second
#define MS_PER_SEC   1000.0      // ms per second
#define MAX_INTERVAL 4000        // Maximum interval for 8-bit timers

// Static pointer initialization
Timer* Timer::instance = nullptr;

//=============================================================================
// Timer Constructor
// Description: Accepts the timer type (0,1,2) and the needed time unit 
//              (MICROS, MILLIS) and initializes the timer accordingly.
//=============================================================================
Timer::Timer(TimerNum num, TimeUnit unit) 
  : overflow_counter(0), interval_devisor(0), _num(num), _unit(unit) {
    instance = this;
}

//=============================================================================
// Timer Public Method: init
// Description: Initialize the timer with the given interval and serial object.
//=============================================================================
void Timer::init(const uint32_t &interval, TimerMode mode, Serial &serial) {
    configure(mode, interval, serial); // Configure the timer
}

void Timer::configure(TimerMode mode, uint32_t interval, Serial &serial) {
    stop(); // Stop the timer before setting the prescaler
    cli();  // Disable interrupts temporarily
    _clear_prescaler_bits(); // Clear the prescaler bits
    set_mode(mode);          // Set Mode (NORMAL, CTC, EXT_CLOCK)

    // Prescaler settings for ms and us (threshold)
    uint16_t prescaler = set_prescaler(interval, serial);
    uint32_t ocr_value = ((F_CPU / prescaler) * (_adjusted_interval / 
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

    sei();             // Re-enable interrupts
    start();           // Start the timer again
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

//=============================================================================
// ISR External Clock (Counter Mode) Implementation
// Description: This is only available for TIMER1. The external clock source
//              is connected to the T1 pin. The timer will count the number of
//              external clock pulses. 
//=============================================================================
ISR(TIMER1_CAPT_vect) {
    // Read the captured value from the ICR1 register
    Timer::instance->captured_value = ICR1;

    // Reset the counter if specific conditions are met?
    // TCNT1 = 0;
}

//=============================================================================
// ISR Compare A Implementations
// Description: Increment overflow counter on timer compare match interrupt.
//              Only the ISR started with start() (e.g., TMSK0 |= (1 << OCIE0A);) 
//              will be called. overflow_counter resets to 0 on overflow.
//=============================================================================
ISR(TIMER0_COMPA_vect) {
    if (Timer::instance->interval_devisor <= 1) {
        Timer::instance->overflow_counter++;
        Timer::instance->interval_devisor = Timer::instance->temp_interval_devisor;
    } else {
        Timer::instance->interval_devisor--;
    }
}

ISR(TIMER1_COMPA_vect) {
    if (Timer::instance->interval_devisor <= 1) {
        Timer::instance->overflow_counter++;
        Timer::instance->interval_devisor = Timer::instance->temp_interval_devisor;
    } else {
        Timer::instance->interval_devisor--;
    }
}

ISR(TIMER2_COMPA_vect) {
    if (Timer::instance->interval_devisor <= 1) {
        Timer::instance->overflow_counter++;
        Timer::instance->interval_devisor = Timer::instance->temp_interval_devisor;
    } else {
        Timer::instance->interval_devisor--;
    }
}

//=============================================================================
// Timer Private Method: set_mode
// Description: Set the timer mode to NORMAL, CTC, or EXT_CLOCK.
//=============================================================================
void Timer::set_mode(TimerMode mode) {
    cli(); // Disable global interrupts

    switch (_num) {
        case TIMER0:
            // Reset relevant WGM bits for TIMER0
            TCCR0A &= ~((1 << WGM01) | (1 << WGM00));
            switch (mode) {
                case NORMAL:
                    // Normal mode is already set by clearing WGM01 and WGM00
                    break;
                case CTC:
                    TCCR0A |= (1 << WGM01); // Set CTC mode
                    break;
                case EXT_CLOCK:
                    // External Clock is not available for TIMER0
                    break;
            }
            break;
        case TIMER1:
            // Reset relevant WGM and CS bits for TIMER1
            TCCR1A &= ~((1 << WGM10));
            TCCR1B &= ~((1 << WGM12) | (1 << WGM13) | (1 << CS12) | (1 << CS11) | (1 << CS10));
            switch (mode) {
                case NORMAL:
                    // Normal mode: all WGM bits are cleared
                    break;
                case CTC:
                    TCCR1B |= (1 << WGM12); // Set CTC mode
                    break;
                case EXT_CLOCK:
                    // Set external clock mode
                    TCCR1B |= (1 << CS12) | (1 << CS11);
                    // External clock source on T1 pin (counter mode) 8
                    TIMSK1 = (1 << ICIE1); // Enable input capture interrupt
                    break;
            }
            break;
        case TIMER2:
            // Reset relevant WGM bits for TIMER2
            TCCR2A &= ~((1 << WGM21) | (1 << WGM20));
            switch (mode) {
                case NORMAL:
                    // Normal mode is already set by clearing WGM21 and WGM20
                    break;
                case CTC:
                    TCCR2A |= (1 << WGM21); // Set CTC mode
                    break;
                case EXT_CLOCK:
                    // External Clock is not available for TIMER2
                    break;
            }
            break;
    }

    sei(); // Enable global interrupts
}

//=============================================================================
// Timer Private Methods: set_prescaler
// Description: Calculate the prescaler value based on the interval and return
//              the prescaler value.
//=============================================================================
uint16_t Timer::set_prescaler(uint32_t interval, Serial &serial) {
    // Variable to keep track of the interval adjustments
    _adjusted_interval = interval;
    interval_devisor = 0;  // Default to 0

    // Check if the interval is over the timer max threshold, if so, find the best divisor
    if (interval > MAX_INTERVAL) {
        uint32_t best_result = 0;
        // Loop to find the largest divisor with a result under MAX_INTERVAL
        for (uint32_t divisor = 2; divisor <= interval; divisor++) {
            if (interval % divisor == 0) {
                uint32_t result = interval / divisor;
                if (result <= MAX_INTERVAL && result > best_result) {
                    best_result = result;
                    interval_devisor = divisor;
                    _adjusted_interval = result;
                }
            }
        }
        char buf[128];
        if (best_result > 0) {
            sprintf(buf, "Best divisor: %d gives result %d with modulus 0.\r\n", interval_devisor, _adjusted_interval);
        } else {
            sprintf(buf, "No suitable divisor found that divides evenly under the threshold.\r\n");
        }
        serial.uart_put_str(buf);
    } else {
        serial.uart_put_str("Interval is under the threshold. No division needed.\r\n");
    }
    
    // TODO: Implement correct pre-scaler settings for the 8-bit timers.

    // Prescaler settings for ms and us (threshold)
    static const Timer::PrescalerSettings ms_settings[] = {
        // Prescaler settings for milliseconds (no need for prescaler 1)
        {30, 8}, {250, 64}, {1000, 256}, {UINT32_MAX, 1024}
    };
    static const Timer::PrescalerSettings us_settings[] = {
        {3000, 1}, {30000, 8}, {250000, 64}, {1000000, 256}, {UINT32_MAX, 1024}
    };

    // Choose the correct settings based on the unit
    const Timer::PrescalerSettings* settings = (_unit == MICROS) ? 
        us_settings : ms_settings;
    const size_t num_settings = (_unit == MICROS) ? 
        (sizeof(us_settings) / sizeof(us_settings[0])) : 
        (sizeof(ms_settings) / sizeof(ms_settings[0]));
    
    uint16_t prescaler = 1024;  // Default to the highest prescaler

    // Check the interval vs thresholds to set the prescaler
    for (size_t i = 0; i < num_settings; ++i) {
        if (_adjusted_interval < settings[i].threshold) {
            prescaler = settings[i].prescaler;
            break;
        }
    }

    // Store the interval_devisor for the ISR
    Timer::instance->temp_interval_devisor = interval_devisor;

    // Print value of temp_interval_devisor
    char buf[128];
    sprintf(buf, "Temp interval devisor: %d\r\n", temp_interval_devisor);
    serial.uart_put_str(buf);

    return prescaler;
}

//=============================================================================
// Timer Private Methods: _clear_prescaler_bits
// Description: Clear the prescaler bits for the timer. This is used to reset
//              the prescaler bits before setting the new prescaler value.
//=============================================================================
void Timer::_clear_prescaler_bits() {
    // Clear only the prescaler bits
    switch (_num) {
        case TIMER0:
            TCCR0B &= ~((1 << CS02) | (1 << CS01) | (1 << CS00));
            break;
        case TIMER1:
            TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
            break;
        case TIMER2:
            TCCR2B &= ~((1 << CS22) | (1 << CS21) | (1 << CS20));
            break;
    }
}