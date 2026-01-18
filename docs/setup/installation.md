# Installation Guide

## Prerequisites

- ESP-IDF v5.5.1
- Python 3.11.2 (included with ESP-IDF)
- Git

## Building the Project

1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd garage-controller
   ```

2. Set up ESP-IDF environment:
   ```bash
   # Follow ESP-IDF installation guide for your platform
   # Make sure idf.py is in your PATH
   ```

3. Configure the project:
   ```bash
   idf.py set-target esp32c6
   idf.py menuconfig
   ```

4. Build the project:
   ```bash
   idf.py build
   ```

5. Flash to device:
   ```bash
   idf.py -p <PORT> flash monitor
   ```

## Hardware Setup

See [Hardware Documentation](../hardware/) for pinout and connection details.

## Configuration

Default configuration is in `config/sdkconfig.defaults`. Modify as needed for your setup.