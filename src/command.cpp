//==============================================================================
// CommandParser Class Implementation
//==============================================================================
#include "command.h"

namespace cmdlimit {
    constexpr uint8_t  max_power  = 255;
    constexpr uint16_t min_freq_t = 200;
    constexpr uint16_t max_freq_t = 5000;
    constexpr uint16_t max_ramp_t = 5000;
}

//==============================================================================
// Public Method: parseCommand
//==============================================================================
void Command::parse_cmd(const char* cmd_input) {
    const char* cmd_ledblink     = "ledblink";
    const char* cmd_ledadc       = "ledadc";
    const char* cmd_ledpowerfreq = "ledpowerfreq";
    const char* cmd_button       = "button";
    const char* cmd_ledramptime  = "ledramptime";

    // Parse the command string and prevent buffer overflow with max length 16
    int res = sscanf(cmd_input, "%16s %u %u", cmd_string, &cmd_val1, &cmd_val2);

    // Now compare the first word of the string to the cmd's
    if (strncmp(cmd_string, cmd_ledblink, strlen(cmd_ledblink)) == 0) {
        if (res == 1) cmd = LED_BLINK;
    }
    else if (strncmp(cmd_string, cmd_ledadc, strlen(cmd_ledadc)) == 0) {
        if (res == 1) cmd = LED_ADC;
    }
    else if (strncmp(cmd_string, cmd_ledpowerfreq, strlen(cmd_ledpowerfreq)) == 0) {
        if (res == 3 && 
            cmd_val1 <= cmdlimit::max_power && 
            cmd_val2 >= cmdlimit::min_freq_t && cmd_val2 <= cmdlimit::max_freq_t) { 
            cmd = LED_PWR;
        } else { cmd = NO_CMD; }
    }
    else if (strncmp(cmd_string, cmd_button, strlen(cmd_button)) == 0) {
        if (res == 1) cmd = BUTTON;
    }
    else if (strncmp(cmd_string, cmd_ledramptime, strlen(cmd_ledramptime)) == 0) {
        if (res == 2 && cmd_val1 <= cmdlimit::max_ramp_t) {
            cmd = LED_RAMP;
        } else { cmd = NO_CMD; } 
    } else {
        cmd = NO_CMD;
    }
}