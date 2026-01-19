# Agent Configuration Notes

## ⚠️ CRITICAL: IDF.py Command Execution

**When executing any `idf.py` commands (set-target, build, flash, monitor, etc.), the AI assistant must PAUSE and instruct the user to run the commands manually. Do NOT run `idf.py` commands directly.**

### Zigbee Component Dependencies
This project requires specific Zigbee components that must be included in CMakeLists.txt:

**Required Components in main/CMakeLists.txt PRIV_REQUIRES:**
- `esp-zigbee-lib`
- `esp-zboss-lib`
- `zcl_utility` (custom component created locally)

**Component Dependencies in main/idf_component.yml:**
- `espressif/esp-zboss-lib: "~1.6.0"`
- `espressif/esp-zigbee-lib: "~1.6.0"`

### Environment Setup


#### Build Commands

**Standard ESP-IDF workflow:**
```bash
idf.py set-target esp32c6
idf.py clean
idf.py build
idf.py flash
idf.py monitor
```

### Troubleshooting Notes

- Always verify component dependencies are properly included in both idf_component.yml and CMakeLists.txt PRIV_REQUIRES
- If you get compilation errors about missing headers (like esp_timer.h), add the required component to PRIV_REQUIRES in CMakeLists.txt

## ⚠️ CRITICAL: Component Refactoring Checklist

**When combining, renaming, or removing ESP-IDF components, update ALL CMakeLists.txt files that reference them:**

### Required Updates:
1. **Component's own CMakeLists.txt**: Update SRCS, INCLUDE_DIRS, and PRIV_REQUIRES to reflect the new structure
2. **Main CMakeLists.txt**: Update INCLUDE_DIRS and PRIV_REQUIRES to reference the new component name
3. **ALL dependent component CMakeLists.txt**: Update any components that depend on the renamed/removed component
4. **Test build immediately**: Run `idf.py build` after changes to catch missing dependencies

### Common Failure Points:
- Zigbee component often depends on sensor components
- Light components may depend on device drivers
- Main application component references most other components

**Example**: When combining `sensor_interface` into `sensor_manager`, update:
- `components/sensor_manager/CMakeLists.txt` (combine SRCS and dependencies)
- `main/CMakeLists.txt` (change sensor_interface → sensor_manager)
- `components/zigbee/CMakeLists.txt` (remove sensor_interface dependency)

## ⚠️ CRITICAL: ESP-IDF Terminal Usage

**AI assistants MUST NOT attempt manual builds with cmake/ninja. All ESP-IDF builds must use `idf.py` commands within a proper ESP-IDF terminal.**

### Why Manual CMake/Ninja Builds Fail

ESP-IDF projects cannot be built using direct `cmake` and `ninja` commands because:

1. **Environment Variables Missing**: ESP-IDF requires specific environment variables (IDF_PATH, IDF_TARGET, etc.) that are only set in ESP-IDF terminals
2. **Python Path Issues**: The ESP-IDF Python environment is not accessible via standard `python` commands
3. **Toolchain Configuration**: ESP-IDF tools require specific PATH configurations that are only available in ESP-IDF terminals
4. **Component Registration**: ESP-IDF's component system requires proper initialization that only happens in ESP-IDF terminals

### ESP-IDF Terminal Requirements

**ALL `idf.py` commands must be executed in a proper ESP-IDF terminal:**

- Use ESP-IDF Command Prompt (Windows)
- Use `idf.py` terminal command (Linux/Mac)
- Do NOT use regular PowerShell/Command Prompt terminals
- Do NOT use manual cmake/ninja invocations

### Correct Build Process

1. **Open ESP-IDF Terminal**: Launch proper ESP-IDF command environment
2. **Navigate to Project**: `cd C:\code\garage-controller`
3. **Configure Target**: `idf.py set-target esp32c6`
4. **Build**: `idf.py build`
5. **Flash**: `idf.py flash`
6. **Monitor**: `idf.py monitor`

### What Happens When You Try Manual Builds

When AI assistants attempt manual cmake/ninja builds, you will encounter:
- "CMake Error: Could not find CMAKE_CXX_COMPILER"
- "No module named 'click'" errors
- Missing ESP-IDF component dependencies
- Undefined symbols and linker errors
- Build failures due to missing environment variables

### AI Assistant Instructions

- **NEVER** run `idf.py` commands directly in tools
- **ALWAYS** instruct user to run `idf.py` commands manually in ESP-IDF terminal
- **NEVER** use manual cmake/ninja commands
- **ALWAYS** remind user that ESP-IDF builds require proper terminal environment