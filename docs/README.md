# Garage Controller Documentation

This directory contains all documentation for the Garage Door Controller project.

## Directory Structure

- `api/` - API documentation and specifications
- `hardware/` - Hardware schematics, pinouts, and specifications
- `protocols/` - Communication protocols and specifications
- `setup/` - Installation and setup guides
- `user/` - User manuals and operation guides

## Key Documents

- [Zigbee Protocol Implementation](protocols/zigbee.md)
- [Hardware Pinout](hardware/dev-kit-pinout.jpeg)
- [Installation Guide](setup/installation.md)

## Development Insights

### Architecture Lessons Learned

**Component Initialization Order**: Light hardware must be initialized before Zigbee stack to avoid RMT channel conflicts.

**Zigbee Attribute Routing**: Single attribute handler in zigbee_manager routes commands to appropriate component handlers (light, sensor, etc.).

**Hardware Debugging Strategy**:
1. Verify GPIO pins with simple tests before implementing complex protocols
2. Use extensive logging for hardware interactions
3. Test individual components before integration
4. Always verify hardware pinouts match your specific development board

**ESP-IDF Specific Issues**:
- RMT channels are shared resources - Zigbee uses them extensively
- FreeRTOS functions require explicit includes
- Component dependencies must be declared in both CMakeLists.txt and idf_component.yml
- LED strip libraries may conflict with wireless protocols

## Contributing

When adding new documentation:
1. Place hardware-related docs in `hardware/`
2. Protocol specifications go in `protocols/`
3. API docs go in `api/`
4. Setup instructions go in `setup/`
5. User-facing documentation goes in `user/`

When encountering build issues:
1. Document the error and solution in the troubleshooting section
2. Update component dependencies if new libraries are added
3. Verify hardware configurations match your setup