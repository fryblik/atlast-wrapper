/* Atlast wrapper For ESP32.
 * Copyright (c) 2021 Vojtech Fryblik
 * This code is licensed under MIT license (see LICENSE.txt for details).
 */

#include "atlast-1.2-esp32/atlast.h"
#include "atlast-wrapper.h"


// Run Data
struct runData rd;

// Run Data mutex and task handle
SemaphoreHandle_t atlastRunMutex = xSemaphoreCreateMutex();
TaskHandle_t atlastTaskHandle = nullptr;


/**
 * ATLAST start run
 * 
 * Check start flag. If true, start execution. Otherwise wait a bit.
 * Returns true on program start, false otherwise.
 */
bool atlastStartRun() {
    // Take mutex
    xSemaphoreTake(atlastRunMutex, portMAX_DELAY);    

    if (rd.startFlag) {
        // Start execution and release mutex
        rd.startFlag = false;
        rd.isRunning = true;
        xSemaphoreGive(atlastRunMutex);
        return true;
    } else {
        // Release mutex and wait 0.1s
        xSemaphoreGive(atlastRunMutex);
        vTaskDelay(100 / portTICK_RATE_MS);
        return false;
    }
}

/**
 * ATLAST reset run
 * 
 * Reset Run Data and release mutex.
 */
void atlastResetRun() {
    // Empty the queue if nonempty
    if (!rd.commands.empty()) {
        std::queue<std::string> emptyQueue;
        std::swap(rd.commands, emptyQueue);
    }
    rd.startFlag = false;
    rd.killFlag = false;
    rd.isRunning = false;
    xSemaphoreGive(atlastRunMutex);
}

/**
 * ATLAST interpreter loop
 * 
 * Execute ATLAST commands from queue when available.
 * Blocks atlastRunMutex.
 * Run in a separate task.
 */
void atlastInterpreterLoop(void * pvParameter) {
    // Wait for execution
    while(true) {
        // If not ready to start, try again
        if (!atlastStartRun())
            continue;
        
        // Take mutex
        xSemaphoreTake(atlastRunMutex, portMAX_DELAY); 

        // Execute commands stored in Run Data
        while (!rd.commands.empty()) {
            // On KILL flag stop execution and reset Run Data
            if (rd.killFlag) {
                break;
            }

            // Get pointer to the string in front of the queue
            char *command = &rd.commands.front()[0];
            // Release mutex during execution to allow addition of commands to queue
            // Note: rd.commands.pop() must not be called in the meantime!
            xSemaphoreGive(atlastRunMutex);
            // Evaluate string in front of the queue
            atl_eval(command);

            // Take mutex again
            xSemaphoreTake(atlastRunMutex, portMAX_DELAY);
            // Remove front string from the queue
            rd.commands.pop();

            // Print acknowledgement of executed command
            rd.output += "\n< ok\n";
        }

        // Reset Run Data and release mutex
        atlastResetRun(); 
    }
}

/**
 * ATLAST command
 * 
 * Evaluate ATLAST command.
 */
void atlastCommand(char* command) {
    // Take mutex
    xSemaphoreTake(atlastRunMutex, portMAX_DELAY);

    // Print incoming command to output buffer
    rd.output += "> " + std::string(command) + "\n";
    // Append command to queue and start execution, if needed
    rd.commands.push(command);
    if (!rd.isRunning) {
        rd.startFlag = true;
    }

    // Release mutex
    xSemaphoreGive(atlastRunMutex);
}

/**
 * ATLAST output
 * 
 * Return contents of ATLAST output buffer and clear the buffer.
 */
std::string atlastOutput() {
    // Take mutex
    xSemaphoreTake(atlastRunMutex, portMAX_DELAY);

    // Declare empty string, swap content with output buffer
    std::string retStr;
    retStr.swap(rd.output);

    // Release mutex
    xSemaphoreGive(atlastRunMutex);

    return retStr;
}

/**
 * ATLAST output available
 * 
 * Return true iff ATLAST output buffer is populated.
 */
bool atlastOutputAvailable() {
    return !rd.output.empty();
}

/**
 * ATLAST create task
 * 
 * Create ATLAST machine task.
 */
void atlastCreateTask() {
    xTaskCreate(&atlastInterpreterLoop,
                ATL_TASK_NAME,
                ATL_TASK_STACK_SIZE,
                NULL,
                ATL_TASK_PRIORITY,
                &atlastTaskHandle); // Store task handle
}

/**
 * ATLAST init
 * 
 * Initiate ATLAST and create interpreter task.
 */
void atlastInit() {
    // Need to explicitly initialize ATLAST before extending dictionary
    atl_init();

    // TODO: Extend ATLAST dictionary with custom word definitions
    //atlastAddPrims();

    // Create ATLAST interpreter task
    atlastCreateTask();
}

/**
 * ATLAST kill
 * 
 * Sets KILL flag to reset Run Data.
 * Breaks running ATLAST program.
 * Restarts ATLAST in a new task if requested.
 * Does not call ATLAST ABORT (that would reset atlast.c state itself).
 */
void atlastKill(bool restartTask) {
    if (rd.isRunning) {
        // Set KILL flag for interpreter task
        rd.killFlag = true;

        // Set BREAK flag for atl_exec
        atl_break();
    }

    // Restart ATLAST task if requested
    if (restartTask) {
        // Prevent passing NULL below (would suspend and delete calling task)
        if (!atlastTaskHandle) {
            // TODO: How to notify about error?
            //printf("ERROR: Task handle is NULL, cannot restart task.\n");
            return;
        }

        // Delete task and recreate mutex
        vTaskDelete(atlastTaskHandle);
        vSemaphoreDelete(atlastRunMutex);
        atlastRunMutex = xSemaphoreCreateMutex();

        // Wait for the task to be deleted
        do {
            vTaskDelay(50 / portTICK_PERIOD_MS);
        } while (!strcmp(pcTaskGetTaskName(atlastTaskHandle), ATL_TASK_NAME));

        atlastResetRun();
        atlastCreateTask();

        // Command to clear return stack (keeps data stack)
        rd.commands.push("quit");
        rd.startFlag = true;
    }
}

/**
 * PRINTF wrapper
 * 
 * Append formatted data to ATLAST output buffer.
 * Used to substitute printf() in src/atlast-1.2-esp32/atlast.c.
 */
int printfWrap(char * format, ...){
    char buf[256];

    // Format string
    va_list args;
    va_start(args, format);
    int length = vsprintf(buf, format, args);
    va_end(args);

    // Add string to output buffer
    xSemaphoreTake(atlastRunMutex, portMAX_DELAY);
    rd.output.append(buf);
    xSemaphoreGive(atlastRunMutex);

    return length;
}