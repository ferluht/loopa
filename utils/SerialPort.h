//
// Created by ferluht on 08/01/2024.
//

#pragma once

#include <stdlib.h>
#include <unistd.h> //ssize_t
#include <cstdint>

int openAndConfigureSerialPort(const char* portPath, int baudRate);
bool serialPortIsOpen();
ssize_t flushSerialData();
ssize_t writeSerialData(uint8_t* bytes, size_t length);
ssize_t readSerialData(char* bytes, size_t length);
ssize_t closeSerialPort(void);
int getSerialFileDescriptor(void);