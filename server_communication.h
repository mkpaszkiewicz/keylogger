//
// Created by konrad on 25.01.16.
//

#ifndef KEYLOGGER_SERVER_COMMUNICATION_H
#define KEYLOGGER_SERVER_COMMUNICATION_H

#include <stdint.h>

#define PROTOCOL_VERSION 1

int sendDataToServer(int fd, char* buffer, int size, uint32_t * machineId);

#endif //KEYLOGGER_SERVER_COMMUNICATION_H
