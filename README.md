# atlast-wrapper
[Atlast Forth 1.2](https://www.fourmilab.ch/atlast/) "wrapper"** for ESP32-WROVER and FreeRTOS. Pulled out of [fryblik/esp32-atlast](https://github.com/fryblik/esp32-atlast) to allow integration into other projects.

Wraps Atlast in a FreeRTOS task, input and output are buffered. Specifically modified Atlast 1.2 files are required (src/atlast-1.2-esp32/).

## Usage
Submit line of code for evaluation:
```cpp
void atlastCommand(char* command)
```

Get contents of Atlast output buffer (clears the buffer):
```cpp
std::string atlastOutput()
```

Check if Atlast output buffer is populated:
```cpp
bool atlastOutputAvailable()
```

Send ATL_BREAK signal:
```cpp
void atlastKill(false)
```

Restart Atlast task and clear Atlast return stack:
```cpp
void atlastKill(true)
```
