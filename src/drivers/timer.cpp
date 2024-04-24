//=============================================================================
// Timer Driver Class Implementation
//=============================================================================
#include "drivers/timer.h"

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
// Timer Public Methods: start, stop, reset
//=============================================================================
void Timer::init() {
    switch (_num) {
        case TIMER_0: _init_timer_0(); break;
        case TIMER_1: _init_timer_1(); break;
        case TIMER_2: _init_timer_2(); break;
    }
    
    start(); // Start the timer
}

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

void Timer::set_prescaler(uint32_t interval) {
    // Prescaler settings for ms and us (threshold)
    static const PrescalerSetting ms_settings[] = {
        {3, 1}, {30, 8}, {250, 64}, {500, 128}, {1000, 256}, 
        {UINT32_MAX, 1024}
    };
    static const PrescalerSetting us_settings[] = {
        {3000, 1}, {30000, 8}, {250000, 64}, {500000, 128}, 
        {1000000, 256}, {UINT32_MAX, 1024}
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

    // Calculate the OCR value based on the prescaler
    uint32_t ocr_value = ((F_CPU / prescaler) * (interval /
                     (_unit == MICROS ? US_PER_SEC : MS_PER_SEC))) - 1;

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
}

//=============================================================================
// Timer Private Methods: _init_timer_0, _init_timer_1, _init_timer_2
//=============================================================================
void Timer::_init_timer_0() {
    TCCR0A = (1 << WGM01);  // CTC Mode
    TCCR0B = 0x00;          // Stop the timer

    if (_unit == MICROS) {
        // Set compare match register for ~1us @16MHz
        OCR0A = _ocr_micros;
        TCCR0B |= ((1 << CS00));  // No prescaling
    } else {
        // Set compare match register for ~1ms @16MHz
        OCR0A = _ocr_millis;
        TCCR0B |= ((1 << CS01) | (1 << CS00));  // Prescaler 64
    }
}

void Timer::_init_timer_1() {
    TCCR1A = 0x00;          // Stop the timer
    TCCR1B = (1 << WGM12);  // CTC Mode

    // Stop the timer
    TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));

    if (_unit == MICROS) {
        // Set compare match register for ~1us @16MHz
        OCR1A = _ocr_micros;
        TCCR1B |= (1 << CS10);  // No prescaling
    } else {
        // Set compare match register for ~1ms @16MHz
        OCR1A = _ocr_millis;
        TCCR1B |= (1 << CS10) | (1 << CS11);  // Prescaler 64
    }
}

void Timer::_init_timer_2() {
    TCCR2A = (1 << WGM21);  // CTC Mode
    TCCR2B = 0x00;          // Stop the timer

    if (_unit == MICROS) {
        // Set compare match register for ~1us @16MHz
        OCR2A = _ocr_micros;
        TCCR2B |= (1 << CS20);  // No prescaling
    } else if (_unit == MILLIS) {
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
}

ISR(TIMER1_COMPA_vect) {
    Timer::timer_ptr->overflow_counter++;
}

ISR(TIMER2_COMPA_vect) {
    Timer::timer_ptr->overflow_counter++;
}

//=============================================================================
// Constexpr Definitions
// Description: These definitions are used to calculate the OCR values
//              for the timer based on the CPU frequency and prescaler.
//=============================================================================
static constexpr uint32_t _ocr_micros = ((F_CPU / US_PER_SEC / PRESCALER_US) - 1);
static constexpr uint32_t _ocr_millis = ((F_CPU / MS_PER_SEC / PRESCALER_MS) - 1);