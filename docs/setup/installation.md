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

## Troubleshooting Build Issues

### Common Build Errors and Solutions

#### 1. FreeRTOS Function Errors
**Error**: `implicit declaration of function 'vTaskDelay'`
**Solution**: Include FreeRTOS headers:
```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
```

#### 2. LED Strip RMT Channel Conflicts
**Error**: `ESP_ERR_NOT_FOUND` when initializing LED strip
**Cause**: Zigbee uses RMT channels, conflicting with LED strip
**Solution**: Use default RMT channel assignment (don't specify channel explicitly)

#### 3. Undefined Zigbee Macros
**Error**: `ESP_MANUFACTURER_NAME` undeclared
**Solution**: Define manufacturer info directly in code:
```c
zcl_basic_manufacturer_info_t info = {
    .manufacturer_name = "\x09""ESPRESSIF",
    .model_identifier = "\x07""ESP32C6-GARAGE",
};
```

#### 4. GPIO Pin Configuration Issues
**Symptom**: LED doesn't respond despite correct logs
**Check**: Verify GPIO pin matches your hardware (check dev kit pinout)
**Solution**: Update `CONFIG_EXAMPLE_STRIP_LED_GPIO` in `sdkconfig.defaults`

#### 5. Component Dependency Issues
**Error**: Missing LED strip or Zigbee components
**Solution**: Ensure dependencies in `main/idf_component.yml`:
```yaml
dependencies:
  espressif/esp-zboss-lib: "~1.6.0"
  espressif/esp-zigbee-lib: "~1.6.0"
  espressif/led_strip: "~2.0.0"
```

### Hardware Debugging Steps

1. **Verify GPIO Pin**: Check your dev kit pinout matches the configured GPIO
2. **Test GPIO Functionality**: Add simple GPIO toggle to verify pin works
3. **Check Power**: Ensure LED strip has proper 5V power supply
4. **Verify LED Type**: WS2812 LEDs require specific timing protocols

### Zigbee Integration Issues

1. **LED Not Responding to Commands**: Check Zigbee network connectivity
2. **Duplicate Initialization**: Ensure light initialization only happens once
3. **Attribute Handler Conflicts**: Zigbee manager should route light commands properly

### Development Tips

- Always run `idf.py clean` after changing component dependencies
- Test GPIO pins individually before implementing complex protocols
- Use logging extensively for debugging hardware interactions
- Verify hardware pinouts match your specific ESP32 development board