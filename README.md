# atlast-wrapper
[Atlast Forth 1.2](https://www.fourmilab.ch/atlast/) "wrapper"** for ESP32-WROVER and FreeRTOS. Pulled out of [fryblik/esp32-atlast](https://github.com/fryblik/esp32-atlast) to allow integration into other projects.

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
