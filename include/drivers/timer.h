#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define PRESCALER 64 // Timer prescaler value
#define MS_TO_SEC 1000     // Milliseconds to seconds conversion

enum TimerNumber {
    TIMER_0,
    TIMER_1,
    TIMER_2
};

enum TimerType {
    MILLIS,
    MICROS,
    OVERFLOW
};

class Timer {
public:
    Timer(uint8_t timer_num, uint8_t timer_type, uint32_t f_cpu);  // Constructor accepts the timer number and CPU frequency
    static void increment();   // Increment the milliseconds counter
    static bool checkInterval(uint16_t ms);
    static void complete(); // Static method for ISR to set the completion flag

private:
    static volatile bool _is_completed; // ISR flag to indicate timer completion
    static volatile uint32_t milliseconds; // Stores elapsed milliseconds

    uint32_t _f_cpu;     // Store the CPU frequency
    uint8_t _timer_num;  // Timer number (0, 1, or 2)
    uint8_t _timer_type; // Timer type (setup for millis, micros or overflow)
    void init_millis();  // Private method to setup timer specifics
};

#endif // TIMER_H
