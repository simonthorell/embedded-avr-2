#!/bin/bash

# Define the serial device and baud rate
SERIAL_DEVICE="/dev/ttyUSB0" # Default Linux device path
BAUD_RATE=9600

# Configure serial device based on OS
configure_serial_device() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        SERIAL_DEVICE="/dev/ttyUSB0"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        SERIAL_DEVICE="/dev/cu.usbserial-110"
    elif [[ "$OSTYPE" == "msys"* ]] || [[ "$OSTYPE" == "win32"* ]]; then
        echo "For Windows or WSL, manual setup is required. Please adjust the script or use a compatible tool."
        exit 1
    else
        echo "Unknown OS. Exiting."
        exit 1
    fi
}

# Configure SERIAL_DEVICE based on the operating system
configure_serial_device

# Function to configure the serial device
configure_serial() {
    stty_command="stty"
    [[ "$OSTYPE" == "darwin"* ]] && stty_command="stty -f"
    
    # Common stty configuration for both Linux and macOS
    $stty_command $SERIAL_DEVICE $BAUD_RATE cs8 -cstopb -parenb -ixon -ixoff -crtscts
}

# Function to read from the serial device
read_serial() {
    echo "Reading from $SERIAL_DEVICE (Press CTRL+C to stop)..."
    cat $SERIAL_DEVICE
}

# Function to write to the serial device
write_serial() {
    echo "Type your message and press enter (CTRL+C to exit):"
    while IFS= read -r line; do
        echo "$line" > $SERIAL_DEVICE
    done
}

# Configure the serial device before starting two-way communication
configure_serial

# Start reading from the serial device in the background
read_serial &

# Start writing to the serial device in the foreground
write_serial