//==============================================================================
// CommandParser Class Implementation
//******************************************************************************
// Application Commands:
// Part 1: ledblink
// Part 2: ledadc
// Part 3: ledpowerfreq <power> <freq>  (power: 0-255, freq: 200-5000)
// Part 4: button
// Part 5: ledramptime <time>           (time(ms): 0-5000)
//==============================================================================
#include "cmd_parser.h"

//==============================================================================
// Constructor
//==============================================================================
CmdParser::CmdParser(){}

//==============================================================================
// Public Method: parseCommand
//==============================================================================
void CmdParser::parse_cmd(const char* cmd_input) {
    // Parse the command string and prevent buffer overflow with max length 20
    int res = sscanf(cmd_input, "%20s %u %u", cmd_string, &cmd_val1, &cmd_val2);

    // Now compare the first word of the string to the available commands
    if (strncmp(cmd_string, "ledblink", strlen("ledblink")) == 0) {
        if (res == 1) command = LED_BLINK;
    }
    else if (strncmp(cmd_string, "ledadc", strlen("ledadc")) == 0) {
        if (res == 1) command = LED_ADC;
    }
    else if (strncmp(cmd_string, "ledpowerfreq", strlen("ledpowerfreq")) == 0) {
        if (res == 3) command = LED_PWR;
    }
    else if (strncmp(cmd_string, "button", strlen("button")) == 0) {
        if (res == 1) command = BUTTON;
    }
    else if (strncmp(cmd_string, "ledramptime", strlen("ledramptime")) == 0) {
        if (res == 2) command = LED_RAMP;
    } else {
        command = NO_CMD;
    }
}