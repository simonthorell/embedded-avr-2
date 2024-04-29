#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>         // size_t
#include <stdio.h>          // For sprintf 
#include "drivers/serial.h"

#ifndef F_CPU
#define F_CPU 16000000UL // Define default MCU clock speed if not defined
#endif

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

//==============================================================================
// Timer Class Declaration
//==============================================================================
class Timer {
public:
    enum TimerNum  { TIMER0, TIMER1, TIMER2 };
    enum TimeUnit  { MILLIS, MICROS };
    enum TimerMode { NORMAL, CTC, EXT_CLOCK };

    struct PrescalerSettings {
        uint32_t threshold;
        uint16_t prescaler;
    };

    // constexpr variables
    static constexpr uint32_t US_PER_SEC        = 1000000UL; // microseconds per second
    static constexpr double   MS_PER_SEC        = 1000.0;    // milliseconds per second
    static constexpr uint16_t MAX_INTERVAL      = 4000;      // Maximum interval for 16-bit timers (ms)
    static constexpr uint8_t  MAX_INTERVAL_8BIT = 255;       // Maximum interval for 8-bit timers

    // Static Singleton Instances
    static Timer timer_0;
    static Timer timer_1;
    static Timer timer_2;

    // Public methods
    static Timer* get_instance(TimerNum num); // Get the Singleton instance
    void configure(TimerMode mode, uint32_t interval, Serial &serial);
    void start();
    void stop();

    // Static variables
    volatile uint16_t overflow_counter;
    volatile uint16_t captured_value;
    volatile uint16_t interval_devisor;
    volatile uint16_t temp_interval_devisor;

    // Static Prescaler Settings
    static const PrescalerSettings ms_settings_8bit[];
    static const PrescalerSettings us_settings_8bit[];
    static const PrescalerSettings ms_settings_16bit[];
    static const PrescalerSettings us_settings_16bit[];

    // Timer interrupt handler
    static void handle_timer_interrupt(Timer* timer);

private:
    // Constructor
    Timer(TimerNum num, TimeUnit unit);

    // Private variables
    TimerNum _num;
    TimeUnit _unit;
    uint16_t _adjusted_interval;
    
    // Private methods
    uint16_t set_prescaler(uint32_t interval, Serial &serial);
    void set_mode(TimerMode mode);
    void _clear_prescaler_bits();
};

#endif // TIMER_H