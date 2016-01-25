//
// Created by konrad on 25.01.16.
//

#include <netinet/in.h>
#include "protocol_messages.h"

struct hello_msg buildHelloMsg(const uint32_t protocolVersion, const uint32_t machineId) {
    struct hello_msg result;
    result.msgType = (uint8_t) 0;
    result.machineId = htonl(machineId);
    result.protocolVersion = htonl(protocolVersion);
    return result;
}

struct send_msg_header buildSendMsgHeader(const uint32_t dataSize) {
    struct send_msg_header result;
    result.msgType = (uint8_t) 1;
    result.dataSize = htonl(dataSize);
    return result;
}

struct bye_msg buildByeMsg() {
    struct bye_msg result;
    result.msgType = (uint8_t) 2;
    return result;
}

struct ok_id_data_header deserializeOkIdDataHeader(const char *data) {
    struct ok_id_data_header result;
    const struct ok_id_data_header* serialized = (const struct ok_id_data_header *) data;
    result.machineId = ntohl(serialized->machineId);
    result.dataSize = ntohl(serialized->dataSize);
    return result;
}
