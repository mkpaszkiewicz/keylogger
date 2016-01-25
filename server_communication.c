#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include "server_communication.h"
#include "protocol_messages.h"

int sendDataToServer(int fd, char* buffer, int size, uint32_t *machineId)
{
    char helperBuffer[HELPER_BUFFER_SIZE];
    // say hello
    struct hello_msg helloMsg = buildHelloMsg(PROTOCOL_VERSION, *machineId);

    printf("hello: %d %d %d\n", helloMsg.msgType, ntohl(helloMsg.protocolVersion), ntohl(helloMsg.machineId));
    printf("sizeof: %d\n", (int) sizeof(helloMsg));
    send(fd, &helloMsg, sizeof(helloMsg), 0);

    // get response (with id, if not present)
    recv(fd, helperBuffer, 1, 0);
    if (helperBuffer[0] == 0)
    {
        printf("-> OK\n");
    }
    else if (helperBuffer[0] == 1)
    {
        recv(fd, helperBuffer, sizeof(struct ok_id_data_header), 0);
        struct ok_id_data_header hdr = deserializeOkIdDataHeader(helperBuffer);
        // update id
        *machineId = hdr.machineId;
        // save id to file, if received TODO
        printf("-> OKIdData: %d %d\n", hdr.machineId, hdr.dataSize);
    }
    else
    {
        printf("-> ??? (%d)\n", helperBuffer[0]);
        return 0;
    }

    // send some data
    struct send_msg_header sendMsgHeader = buildSendMsgHeader(size);
    printf("Sending...\n");
    send(fd, &sendMsgHeader, sizeof(sendMsgHeader), 0);
    send(fd, buffer, (size_t) size, 0);

    // get ok
    recv(fd, helperBuffer, 1, 0);
    if (helperBuffer[0] == 0)
    {
        printf("-> OK\n");
    }

    // say bye
    struct bye_msg byeMsg = buildByeMsg();
    send(fd, &byeMsg, sizeof(byeMsg), 0);
    return 1;
}
