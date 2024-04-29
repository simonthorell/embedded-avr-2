//=============================================================================
// Timer Driver Class Implementation
//=============================================================================
#include "drivers/timer.h"

// Static pointer initialization (For Singleton Pattern)
Timer* Timer::timer_0_instance = nullptr;
Timer* Timer::timer_1_instance = nullptr;
Timer* Timer::timer_2_instance = nullptr;

//=============================================================================
// Timer Constructor
// Description: Accepts the timer type (0,1,2) and the needed time unit 
//              (MICROS, MILLIS) and initializes the timer accordingly.
//=============================================================================
Timer::Timer(TimerNum num, TimeUnit unit) 
  : overflow_counter(0), interval_devisor(0), _num(num), _unit(unit) {
    switch (num) {
        case TIMER0:
            if (timer_0_instance == nullptr) {
                timer_0_instance = this;
            }
            break;
        case TIMER1:
            if (timer_1_instance == nullptr) {
                timer_1_instance = this;
            }
            break;
        case TIMER2:
            if (timer_2_instance == nullptr) {
                timer_2_instance = this;
            }
            break;
    }
}

//=============================================================================
// Timer Public Method: configure
// Description: Configure the timer with the given mode and interval.
//=============================================================================
void Timer::configure(TimerMode mode, uint32_t interval, Serial &serial) {
    stop();                  // Stop the timer before setup 
    cli();                   // Disable interrupts temporarily
    _clear_prescaler_bits(); // Clear the prescaler bits

    // Prescaler settings for ms and us (threshold)
    uint16_t prescaler = set_prescaler(interval, serial);
    uint32_t ocr_value = ((F_CPU / prescaler) * (_adjusted_interval / 
                           (_unit == MICROS ? US_PER_SEC : MS_PER_SEC)) - 1);

    // Inform user about the set pre-scaler and OCR value
    char message[128];
    snprintf(
        message, sizeof(message),
        "Timer %d configured for interval %lu%s (Prescaler: %d, OCR: %lu)\r\n",
        _num, interval, (_unit == Timer::MICROS ? "us" : "ms"), prescaler, ocr_value
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

    set_mode(mode);    // Set Mode (NORMAL, CTC, EXT_CLOCK)
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
// ISR Timer Compare Match A Implementation
//=============================================================================
ISR(TIMER0_COMPA_vect) {
    Timer::handle_timer_interupt(Timer::timer_0_instance);
}

ISR(TIMER1_COMPA_vect) {
    Timer::handle_timer_interupt(Timer::timer_1_instance);
}

ISR(TIMER2_COMPA_vect) {
    Timer::handle_timer_interupt(Timer::timer_2_instance);
}

// Static method to handle timer interrupt
void Timer::handle_timer_interupt(Timer* timer) {
    if (timer->interval_devisor <= 1) {
        timer->overflow_counter++;
        timer->interval_devisor = timer->temp_interval_devisor;
    } else {
        timer->interval_devisor--;
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
            TCCR0A &= ~((1 << WGM01) | (1 << WGM00)); // Clear mode bits
            switch (mode) {
                case NORMAL: 
                    break; // No bits needed for normal mode
                case CTC:
                    TCCR0A |= (1 << WGM01); // Set CTC mode
                    break;
                case EXT_CLOCK: 
                    break; // Not applicable for Timer0
            }
            break;
        case TIMER1:
            TCCR1A &= ~((1 << WGM11) | (1 << WGM10)); // Clear mode bits
            TCCR1B &= ~((1 << WGM13) | (1 << WGM12));
            TCCR1B &= ~((1 << ICES1)); // Clear input capture edge select
            switch (mode) {
                case NORMAL: 
                    break; // No bits needed for normal mode
                case CTC:
                    TCCR1B |= (1 << WGM12); // Set CTC mode
                    break;
                case EXT_CLOCK:
                    TCCR1B |= (1 << ICES1); // Set input capture edge select
                    TCCR1B |= (1 << CS12) | (1 << CS11); // Set external clock mode
                    TCNT1 = 0; // Reset the counter
                    break;
            }
            break;
        case TIMER2:
            TCCR2A &= ~((1 << WGM21) | (1 << WGM20)); // Clear mode bits
            switch (mode) {
                case NORMAL:
                    break; // No bits needed for normal mode
                case CTC:
                    TCCR2A |= (1 << WGM21); // Set CTC mode
                    break;
                case EXT_CLOCK:
                    break; // Not applicable for Timer2
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
    interval_devisor = 1;  // Default to 0

    /* Check if the interval is greater than the maximum interval that the timer
       can handle. If so, find the largest divisor that will result in a value
       under the maximum interval for least CPU usage. ISR will decrement the
       divisor until it reaches 1 (no division). 
    */
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
        char buf[64];
        if (best_result > 0) {
            sprintf(buf, "Set Divisor: %d with result %d.\r\n", interval_devisor, 
                    _adjusted_interval);
            serial.uart_put_str(buf);
        }
    }

    const Timer::PrescalerSettings ms_settings_8bit[] = {
        {15, 64}, {125, 256}, {250, 1024}
    };

    const Timer::PrescalerSettings us_settings_8bit[] = {
        {500, 8}, {2000, 64}, {8000, 256}, {32000, 1024}
    };

    const Timer::PrescalerSettings ms_settings_16bit[] = {
        {30, 8}, {250, 64}, {1000, 256}, {UINT32_MAX, 1024}
    };

    const Timer::PrescalerSettings us_settings_16bit[] = {
        {3000, 1}, {30000, 8}, {250000, 64}, {1000000, 256}, {UINT32_MAX, 1024}
    };

    const Timer::PrescalerSettings* settings;
    size_t num_settings;

    if (_unit == MICROS) {
        if (_num == TIMER1) {
            settings = us_settings_16bit;
            num_settings = sizeof(us_settings_16bit) / sizeof(us_settings_16bit[0]);
        } else {
            settings = us_settings_8bit;
            num_settings = sizeof(us_settings_8bit) / sizeof(us_settings_8bit[0]);
        }
    } else {
        if (_num == TIMER1) {
            settings = ms_settings_16bit;
            num_settings = sizeof(ms_settings_16bit) / sizeof(ms_settings_16bit[0]);
        } else {
            settings = ms_settings_8bit;
            num_settings = sizeof(ms_settings_8bit) / sizeof(ms_settings_8bit[0]);
        }
    }

    uint16_t prescaler = 1024;  // Default to the highest prescaler
    // Check the interval vs thresholds to set the prescaler
    for (size_t i = 0; i < num_settings; ++i) {
        if (_adjusted_interval < settings[i].threshold) {
            prescaler = settings[i].prescaler;
            break;
        }
    }

    temp_interval_devisor = interval_devisor; // Save the divisor for ISR

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