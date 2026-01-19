# Garage Door Controller

This project provides smart home features to un-smart garage doors.

## Features

### Sensors
- **Environmental Sensors**: 
  - **DHT22 Temperature/Humidity sensor** - Environmental monitoring
- **Depth sensor for door position** - Detects if garage door is open/closed
- **Depth sensor for vehicle presence** - Detects if a vehicle is parked in the garage
- **Additional sensors** - Support for future sensor integrations

### Garage Door Control
- **Modular architecture** - Designed to potentially support integrations for multiple door opener brands and models
- **Supported Models**:
  - **Overhead Door Model 696CD/B** - Uses their proprietary Series II protocol

### Home Assistant Integration via Zigbee
- **Door controls** - Open/Close garage door remotely
- **Light controls** - Control garage lighting
- **Sensor readouts** - Display all sensor data in Home Assistant

## Hardware Configuration

### Pin Assignments
- **GPIO0**: DHT22 Temperature/Humidity sensor data pin

## Getting Started

### Environment Setup

1. **Run the environment setup script**:
   ```powershell
   .\environment-setup.ps1
   ```
   This PowerShell script configures your environment with the necessary ESP-IDF tools and paths.

2. **Alternative manual setup** (if the script doesn't work):
   - Ensure ESP-IDF v5.5.1 is installed at `C:\code\esp\v5.5.1\esp-idf`
   - Add the tool paths to your PATH environment variable
   - Set `IDF_PATH` to `C:\code\esp\v5.5.1\esp-idf`
   - Set `IDF_TARGET` to `esp32c6`

### Building the Project

Once your environment is set up:

1. **Configure the project**:
   ```bash
   idf.py set-target esp32c6
   ```

2. **Build the project**:
   ```bash
   idf.py build
   ```

3. **Flash to device**:
   ```bash
   idf.py flash
   ```

4. **Monitor serial output**:
   ```bash
   idf.py monitor
   ```

**⚠️ IMPORTANT**: If `idf.py` fails or the environment setup script indicates ESP-IDF export failed, **STOP** and fix your ESP-IDF installation:

1. Install the missing ESP-IDF tools:
   ```bash
   python C:\code\esp\v5.5.1\esp-idf\tools\idf_tools.py install --targets=esp32c6
   ```

2. Re-run the environment setup:
   ```powershell
   .\environment-setup.ps1
   ```

3. Verify `idf.py` works before proceeding with build commands.

**Do not use manual cmake/ninja commands** - they are unreliable and should only be used as a last resort if ESP-IDF tools cannot be installed.

## Technology Stack
- ESP32-C6-WROOM-1 dev kit
- Zigbee protocol for smart home integration
- ESP-IDF framework
- C/C++ implementation