#ifndef KEYLOGGER_SERVER_COMMUNICATION_H
#define KEYLOGGER_SERVER_COMMUNICATION_H

#include <stdint.h>

#define PROTOCOL_VERSION 1
#define HELPER_BUFFER_SIZE 1024

int sendDataToServer(int fd, char* buffer, int size, uint32_t * machineId);

#endif //KEYLOGGER_SERVER_COMMUNICATION_H
