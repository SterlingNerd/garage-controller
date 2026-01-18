# Agent Configuration Notes

## ESP-IDF Tool Paths and Environment Setup

This ESP-IDF project uses ESP-IDF v5.5.1 with custom tool installations. All tools are located in the `.espressif` directory structure.

### Python Environment Setup

This ESP-IDF project uses Python 3.11.2. There are multiple Python installations available:

**Working Python executable (confirmed for builds):**
```
C:\code\esp\tools\.espressif\tools\idf-python\3.11.2\python.exe
```

**Alternative Python path (may exist but not working for builds):**
```
C:\code\esp\tools\.espressif\python_env\idf5.5_py3.11_env\Scripts\python.exe
```

**Important for Agents/AI Assistants:**
- The `python` command may not be available directly due to Windows app execution aliases being disabled (security best practice)
- Always use the full path when testing Python: `& "C:\code\esp\tools\.espressif\tools\idf-python\3.11.2\python.exe" --version`
- The Python environment is properly configured in the user's PATH but requires full path invocation for direct command line usage
- Version: Python 3.11.2 (ESP-IDF v5.5.1 environment)

### ESP-IDF Build Tools

**ESP-IDF Installation Path:**
```
C:\code\esp\v5.5.1\esp-idf
```

**CMake:**
```
C:\code\esp\tools\.espressif\tools\cmake\3.30.2\bin\cmake.exe
```

**Ninja:**
```
C:\code\esp\tools\.espressif\tools\ninja\1.12.1\ninja.exe
```

**CCache (for faster recompilation):**
```
C:\code\esp\tools\.espressif\tools\ccache\4.11.2\ccache-4.11.2-windows-x86_64\ccache.exe
```

### Manual Build Environment Setup

When the standard `idf.py build` command fails due to environment issues, use this manual setup:

**PowerShell Environment Variables:**
```powershell
$env:PATH = "C:\code\esp\tools\.espressif\tools\cmake\3.30.2\bin;C:\code\esp\tools\.espressif\tools\ninja\1.12.1;C:\code\esp\tools\.espressif\tools\idf-python\3.11.2;C:\code\esp\tools\.espressif\tools\ccache\4.11.2\ccache-4.11.2-windows-x86_64;" + $env:PATH
$env:IDF_PATH = "C:\code\esp\v5.5.1\esp-idf"
$env:IDF_TARGET = "esp32c6"
```

**Configure Build:**
```powershell
& "C:\code\esp\tools\.espressif\tools\cmake\3.30.2\bin\cmake.exe" -S . -B build -G Ninja
```

**Build Project:**
```powershell
& "C:\code\esp\tools\.espressif\tools\ninja\1.12.1\ninja.exe" -C build
```

**Clean Build:**
```powershell
Remove-Item -Recurse -Force build
```

### Zigbee Component Dependencies

This project requires specific Zigbee components that must be included in CMakeLists.txt:

**Required Components in main/CMakeLists.txt PRIV_REQUIRES:**
- `esp-zigbee-lib`
- `esp-zboss-lib`
- `zcl_utility` (custom component created locally)

**Component Dependencies in main/idf_component.yml:**
- `espressif/esp-zboss-lib: "~1.6.0"`
- `espressif/esp-zigbee-lib: "~1.6.0"`

### Troubleshooting Notes

- When `idf.py` fails with "No module named 'click'", use the manual cmake/ninja build process above
- Missing `zcl_utility.h` indicates the custom `zcl_utility` component needs to be created (see components/zcl_utility/)
- ESP-IDF export scripts may fail due to missing ESP_ROM_ELF_DIR environment variable, but manual builds work fine
- Always verify component dependencies are properly included in both idf_component.yml and CMakeLists.txt PRIV_REQUIRES