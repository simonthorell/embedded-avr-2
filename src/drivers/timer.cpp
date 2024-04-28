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
    TimerConfig config = _clear_tccr(); // Clear & store TCCR (PWM settings f.e.)

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

    _set_tccr(config);  // Revert TCCR settings (PWM settings f.e.)
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
// Timer ISR Implementations
// Description: Increment overflow counter on timer compare match interrupt.
//              Only the ISR started with start() (e.g., TMSK0 |= (1 << OCIE0A);) 
//              will be called. overflow_counter resets to 0 on overflow.
//=============================================================================
ISR(TIMER0_COMPA_vect) {
    Timer::instance->overflow_counter++;
}

ISR(TIMER1_COMPA_vect) {
    Timer::instance->overflow_counter++;
}

ISR(TIMER2_COMPA_vect) {
    Timer::instance->overflow_counter++;
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