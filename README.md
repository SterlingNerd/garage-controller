# Garage Door Controller

This project provides smart home features to un-smart garage doors.

## Features

### Sensors
- **Temperature/Humidity sensor** - Environmental monitoring
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

## Technology Stack
- ESP32-C6 microcontroller
- Zigbee protocol for smart home integration
- ESP-IDF framework
- C/C++ implementation