/* Atlast wrapper For ESP32.
 * Copyright (c) 2021 Vojtech Fryblik
 * This code is licensed under MIT license (see LICENSE.txt for details).
 */

#ifdef __cplusplus  // Certain function(s) included by C files

#include <Arduino.h>
#include <queue>

#define ATL_TASK_NAME       "atl"
#define ATL_TASK_PRIORITY   5
#define ATL_TASK_STACK_SIZE 4096


// Run Data
struct runData {
    std::queue<std::string> commands;
    std::string output;
    bool startFlag;
    volatile bool killFlag;
    bool isRunning;
};
extern struct runData rd;

// Run Data mutex
extern SemaphoreHandle_t atlastRunMutex;
extern TaskHandle_t atlastTaskHandle;


/**
 * ATLAST start run
 * 
 * Check start flag. If true, start execution.
 * Returns true on program start, false otherwise.
 */
bool atlastStartRun();

/**
 * ATLAST reset run
 * 
 * Reset Run Data and release mutex.
 */
void atlastResetRun();

/**
 * ATLAST interpreter loop
 * 
 * Execute ATLAST commands when available.
 * Run in a separate task.
 */
void atlastInterpreterLoop(void * pvParameter);

/**
 * ATLAST command
 * 
 * Evaluate ATLAST command.
 */
void atlastCommand(char* command);

/**
 * ATLAST output
 * 
 * Return contents of ATLAST output buffer and clear the buffer.
 */
std::string atlastOutput();

/**
 * ATLAST output available
 * 
 * Return true iff ATLAST output buffer is populated.
 */
bool atlastOutputAvailable();

/**
 * ATLAST create task
 * 
 * Create ATLAST machine task.
 */
void atlastCreateTask();

/**
 * ATLAST init
 * 
 * Initiate ATLAST and create interpreter task.
 */
void atlastInit();

/**
 * ATLAST kill
 * 
 * Sets KILL flag to clear interpreter command queue.
 * Breaks running ATLAST program.
 * Restarts ATLAST in new task if requested.
 * Does not call ATLAST ABORT (that would reset ATLAST interpreter).
 */
void atlastKill(bool restartTask);

extern "C" {
#endif // __cplusplus

/**
 * PRINTF wrapper
 * 
 * Append formatted data to ATLAST output buffer.
 * Used to substitute printf() in src/atlast-1.2-esp32/atlast.c.
 */
int printfWrap(char * format, ...);

#ifdef __cplusplus
}
#endif // __cplusplus