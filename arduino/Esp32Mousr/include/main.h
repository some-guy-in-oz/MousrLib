#pragma once
#ifndef MOUSR_MAIN_H
#define MOUSR_MAIN_H

#include "common.h"

#ifdef ARDUINO
#ifdef ARDUINO_ARCH_ESP32
#include "esp32-hal-log.h"
#endif // ARDUINO_ARCH_ESP32

#if _DO_BLE
#include "Mousr.h"
#include "MousrBluetooth.h"

MousrBluetooth mb;
void waitForStatus(MousrConnectionStatus);
static void onBluetoothStatusChange(MousrConnectionStatus oldStatus, MousrConnectionStatus newStatus);
static void onBluetoothNotify(BLERemoteCharacteristic* characteristic, MousrData& data);

#endif // _DO_BLE

#if _DO_SS
#include "Controller.h"
#define JOYSTICK_INT_PIN 32 // Interrupt pin
#define DRIFT_U 20
#define DRIFT_D 20
#define DRIFT_L 55
#define DRIFT_R 50

void onButtonPressStateChange(ButtonState press);
void onAnalogStickMovement(AnalogStickMovement move);
Controller c(JOYSTICK_INT_PIN);
#endif // _DO_SS

#ifdef _DO_OLED
#include "Oled.h"
#endif // _DO_OLED

#include "utility.h"

#endif // ARDUINO
#endif // MOUSR_MAIN_H