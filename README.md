# AVR Development Template

This is a template setup of a virtual development container for AVR microcontroller projects, using avr-gcc for compilation. It includes a suite of scripts designed to streamline the processes of flashing firmware to the microcontroller and monitoring its serial output.

## Prerequisites
Before using this template, ensure you have the following software installed on your system:

- **Docker**
- **Visual Studio Code** (VS Code)
- **Dev Containers** (VS Code extension)

## Flashing Firmware to the Microcontroller
To deploy firmware to your microcontroller, navigate to the `script` directory and execute the `flash-firmware.sh` script. This utility simplifies the process of transferring your compiled code onto the AVR device.

First install avrdude:
```bash
# macOS
brew install avrdude

# Linux 
sudo apt-get update
sudo apt-get install avrdude

# Windows or WSL
# Download from website: https://www.nongnu.org/avrdude/
```

Then run the flash firmware script:
```bash
cd script
chmod +x ./flash-firmware.sh
./flash-firmware.sh

# You can also specify your device port
./flash-firmware.sh -p <YOUR-PORT>

# Example
./flash-firmware.sh -p /dev/cu.usbserial-110
```

## Serial Port Monitoring
For real-time observation of the microcontroller's serial communication, the following utilities are provided within the `script` folder:

- For a basic monitoring setup, execute:

```bash
cd script
chmod +x ./monitor-mcu.sh
./monitor-mcu.sh
```

- Alternatively, for a more flexible monitoring experience, run the `monitor-mcu.py` Python script. Ensure Python and pySerial are installed on your system.

```bash
python monitor-mcu.py
```

## Identifying Your USB Device
If you are uncertain about your device's port, you can determine it using the following commands in your terminal or command prompt:

**macOS**
```bash
ls /dev/cu.*      # Option 1
ls /dev/tty.*     # Option 2
```

**Linux:**
```bash
dmesg | grep tty  # Option 1
ls /dev/ttyUSB*   # Option 2
ls /dev/ttyACM*   # Option 3
```

**Windows**
```bash
mode
```

## Contribution
Contributions are welcome. Please fork the repository, make your changes, and submit a pull request.

## License
Distributed under the MIT License. See `LICENSE` for more information.