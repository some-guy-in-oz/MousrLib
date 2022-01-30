#pragma once
#ifndef NATIVE_TESTS_H
#define NATIVE_TESTS_H

#include <unity.h>
#include "common.h"

// Mousr tests
void test_ParseMessage();
void test_toHexString();
void test_mousrAlloc();
void test_getRawData();
void test_convertToBytes();
void test_createFromRaw();
void test_connectionStatusMap();

// Settings tests
void test_writeUChar();

#endif // NATIVE_TESTS_H