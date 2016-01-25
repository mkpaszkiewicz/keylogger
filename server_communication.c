//
// Created by konrad on 25.01.16.
//

#include <sys/socket.h>
#include "server_communication.h"
#include "protocol_messages.h"

int sendDataToServer(int fd, char* buffer, int size, uint32_t *machineId) {
    // say hello
    struct hello_msg helloMsg = buildHelloMsg(PROTOCOL_VERSION, *machineId);
    send(fd, &helloMsg, sizeof(helloMsg), 0);

    // get response (with id, if not present)
    // save id to file, if received
    // send some data
    // get ok
    // say bye
    struct bye_msg byeMsg = buildByeMsg();
    send(fd, &byeMsg, sizeof(byeMsg), 0);
    return 0; //not ok
}
