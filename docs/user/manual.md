# User Manual

## Overview

The Garage Door Controller provides smart home integration for traditional garage doors, adding remote control and monitoring capabilities.

## Features

### Door Control
- Remote open/close via Home Assistant
- Door position monitoring
- Safety interlocks

### Lighting
- Garage lighting control
- Integration with door operations

### Sensors
- Temperature and humidity monitoring
- Door position detection
- Vehicle presence detection

### Smart Home Integration
- Full Zigbee compatibility
- Home Assistant integration
- Real-time status updates

## Operation

### Basic Door Control

1. Use Home Assistant dashboard to control door
2. Door will automatically stop if obstructed
3. Safety sensors prevent accidents

### Monitoring

- View current door status
- Check environmental conditions
- Monitor vehicle presence

## Safety Features

- Obstacle detection
- Emergency stop capability
- Safety interlocks prevent simultaneous conflicting operations

## Troubleshooting

### LED Not Working
1. Check GPIO pin configuration matches your hardware
2. Verify LED strip has proper 5V power supply
3. Ensure LED data line is connected to correct GPIO pin
4. Check Zigbee network connectivity for remote control

### Door Not Responding
1. Check power connection
2. Verify Zigbee network connectivity
3. Check for mechanical obstructions

### Sensor Issues
1. Verify sensor connections
2. Check calibration
3. Review error logs
4. DHT22 sensors may show checksum errors in humid environments

### Build Issues
1. Ensure ESP-IDF environment is properly configured
2. Check for missing component dependencies
3. Verify GPIO pin assignments match your hardware
4. Run `idf.py clean` after changing configurations

## Maintenance

- Regularly test safety features
- Clean sensors as needed
- Update firmware when available