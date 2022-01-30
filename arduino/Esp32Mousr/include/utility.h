#pragma once
#ifndef MOUSR_UTILITY_H
#define MOUSR_UTILITY_H

#include <cstdint>

#ifdef ARDUINO_ARCH_ESP32

static void logMemory()
{
    uint32_t total = ESP.getHeapSize();
    uint32_t used = ESP.getFreeHeap();
    s_printf("Heap usage: %d/%d (%d%%)\n", total - used, total, (used / total) * 100);
}

/*
static void semTake(SemaphoreHandle_t waitHandle)
{
    if (waitHandle == nullptr)
    {
        return;
    }

    Serial.println("Waiting for semaphore...");
    xSemaphoreTake(waitHandle, portMAX_DELAY);
    Serial.println("Semaphore get!");
}

static void semGive(SemaphoreHandle_t waitHandle)
{
    if (waitHandle == nullptr)
    {
        return;
    }
    Serial.println("Releasing semaphore...");
    xSemaphoreGive(waitHandle);

}
*/

#else // ARDUINO_ARCH_ESP32
static void logMemory() {}
#endif
#endif // MOUSR_UTILITY_H