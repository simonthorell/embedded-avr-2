//=============================================================================
// Timer Driver Class Implementation
//=============================================================================
#include "drivers/timer.h"
#include "stdio.h" // For snprintf

// Static pointer initialization
Timer* Timer::timer_ptr = nullptr;

//=============================================================================
// Timer Constructor
// Description: Accepts the timer type (0,1,2) and the needed time unit 
//              (MICROS, MILLIS) and initializes the timer accordingly.
//=============================================================================
Timer::Timer(TimerNum num, TimeUnit unit) 
  : _num(num), _unit(unit), overflow_counter(0) {
    timer_ptr = this;
}

//=============================================================================
// Timer Public Methods: init, set_prescaler, start, stop, reset
//=============================================================================
void Timer::init(const uint32_t &interval, Serial &serial) {
    // Disable the timer before setting the mode and prescaler
    switch (_num) {
        case TIMER_0: 
            TCCR0B = 0x00;
            TCCR0A = (1 << WGM01);  // CTC Mode
            break;
        case TIMER_1: 
            TCCR1A = 0x00;
            TCCR1B = 0x00;
            TCCR1B = (1 << WGM12);  // CTC Mode
            break;
        case TIMER_2: 
            TCCR2B = 0x00;
            TCCR2A = (1 << WGM21);  // CTC Mode
            break;
    }

    set_prescaler(interval, serial);
}

void Timer::set_prescaler(uint32_t interval, Serial &serial) {
    stop(); // Stop the timer before setting the prescaler
    cli();  // Disable interrupts temporarily

    // Prescaler settings for ms and us (threshold)
    static const PrescalerSetting ms_settings[] = {
        // Prescaler settings for milliseconds
        // {3, 1}, // No need for prescaler 1 resolution for millis
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

    // Check the interval against the thresholds to determine and set the prescaler
    for (size_t i = 0; i < num_settings; ++i) {
        if (interval < settings[i].threshold) {
            prescaler = settings[i].prescaler;
            break;
        }
    }

    uint32_t ocr_value = ((F_CPU / prescaler) * (interval / 
                           (_unit == MICROS ? US_PER_SEC : MS_PER_SEC)) - 1);

    // Inform user about the set pre-scaler and OCR value
    char message[50];
    snprintf(message, sizeof(message), "Timer %d configured (Prescaler: %d, OCR: %u)\r\n", _num, prescaler, ocr_value);
    serial.uart_put_str(message);

    // Set the register values based on the set timer number
    switch (_num) {
        case TIMER_0: 
            OCR0A = ocr_value;
            TCCR0B |= TIMER0_PS_BITS(prescaler);
            break;
        case TIMER_1:
            OCR1A = ocr_value;
            TCCR1B |= TIMER1_PS_BITS(prescaler);
            break;
        case TIMER_2: 
            OCR2A = ocr_value;
            TCCR2B |= TIMER2_PS_BITS(prescaler);
            break;
    }

    sei();   // Re-enable interrupts
    start(); // Start the timer again
}

void Timer::start() {
     // Enable Timer Compare Match A interrupt
    switch (_num) {
        case TIMER_0: TIMSK0 |= (1 << OCIE0A); break;
        case TIMER_1: TIMSK1 |= (1 << OCIE1A); break;
        case TIMER_2: TIMSK2 |= (1 << OCIE2A); break;
    }
}

void Timer::stop() {
    // Disable Timer Compare Match A interrupt
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

//=============================================================================
// Timer ISR Implementations
// Description: These functions are called when the timer compare match
//              interrupt is triggered. The overflow counter is then
//              incremented to keep track of the timer overflows. only 
//              the ISR that has been started with the start() method
//              will be called (for example TMSK0 |= (1 << OCIE0A);)
// Note: overflow_counter will automatically reset to 0 when overflowing
//       due to nature of C/C++ data types.
//=============================================================================
ISR(TIMER0_COMPA_vect) {
    Timer::timer_ptr->overflow_counter++;
    Timer::timer_ptr->timer_overflowed = true;
    // PORTD ^= (1 << PORTD3); // Debug Code: Toogle pin D3 on/off
}

ISR(TIMER1_COMPA_vect) {
    Timer::timer_ptr->overflow_counter++;
    Timer::timer_ptr->timer_overflowed = true;
    // PORTD ^= (1 << PORTD3); // Debug Code: Toogle pin D3 on/off
}

ISR(TIMER2_COMPA_vect) {
    Timer::timer_ptr->overflow_counter++;
    Timer::timer_ptr->timer_overflowed = true;
    // PORTD ^= (1 << PORTD3); // Debug Code: Toogle pin D3 on/off
}