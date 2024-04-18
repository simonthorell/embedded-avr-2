import serial
import threading

# Install pyserial with pip: 'pip install pyserial'

# Configure your serial device here
SERIAL_PORT = '/dev/cu.usbserial-110'
BAUD_RATE = 9600

# Initialize serial port
ser = serial.Serial(SERIAL_PORT, BAUD_RATE)

# Function to handle incoming serial data
def read_serial():
    while True:
        if ser.in_waiting > 0:
            incoming_data = ser.read(ser.in_waiting).decode('utf-8')
            print(f"{incoming_data}", end='', flush=True)

# Function to send data to the serial device
def write_serial():
    while True:
        outgoing_data = input("") + '\n'  # Add null terminator to the string
        ser.write(outgoing_data.encode('utf-8'))

# Start reading from serial in a background thread
read_thread = threading.Thread(target=read_serial)
read_thread.daemon = True
read_thread.start()

# Main thread handles writing to serial
write_serial()
