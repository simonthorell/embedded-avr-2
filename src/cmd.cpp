//==============================================================================
// CommandParser Class Implementation
//==============================================================================
#include "cmd.h"
#define CMD_LEN 16 // Max command length (Don't change without changing sscanf)
                  // Consequense could result in vulnrable buffer overflow!
//==============================================================================
// Public Method: parseCommand
//==============================================================================
void CMD::parse_cmd(const char* cmd_input) {
    // Temporary command strings  
    char cmd_1[CMD_LEN] = "ledblink";
    char cmd_2[CMD_LEN] = "ledadc";
    char cmd_3[CMD_LEN] = "ledpowerfreq";
    char cmd_4[CMD_LEN] = "button";
    char cmd_5[CMD_LEN] = "ledramptime";

    // Parse the command string and prevent buffer overflow with max length 16
    int res = sscanf(cmd_input, "%16s %u %u", cmd_string, &cmd_val1, &cmd_val2);

    // Now compare the first word of the string to the cmd's
    if (strncmp(cmd_string, cmd_1, strlen(cmd_1)) == 0) {
        if (res == 1) cmd = LED_BLINK;
    }
    else if (strncmp(cmd_string, cmd_2, strlen(cmd_2)) == 0) {
        if (res == 1) cmd = LED_ADC;
    }
    else if (strncmp(cmd_string, cmd_3, strlen(cmd_3)) == 0) {
        if (res == 3 && 
            cmd_val1 <= 255 && 
            cmd_val2 >= 200 && cmd_val2 <= 5000) { 
            cmd = LED_PWR;
        } else { cmd = CMD_NONE; }
    }
    else if (strncmp(cmd_string, cmd_4, strlen(cmd_4)) == 0) {
        if (res == 1) cmd = BTN;
    }
    else if (strncmp(cmd_string, cmd_5, strlen(cmd_5)) == 0) {
        if (res == 2 && cmd_val1 <= 5000) {
            cmd = LED_RAMP;
        } else { cmd = CMD_NONE; } 
    } else {
        cmd = CMD_NONE;
    }
}