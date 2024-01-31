#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h>

void inval(void) {
    write(2, "Invalid Command\n", strlen("Invalid Command\n"));
    exit(1);
}

void str_write(int fd, char *buf, size_t num_chars) {
    ssize_t result;

    size_t num_written = 0;

    for (;;) {
        result = write(fd, buf + num_written, num_chars);

        if (result < 0) {
            //printf("there is an issue");
            break;
        }
        if (result == 0)
            break;

        num_written += result;
        num_chars -= result;
    }
}

int main(void) {

    int bytesToRead;

    int fd;
    char *filename;
    int totalBytesWritten = 0;

    int buff_size = 10000;

    int bytesRead;

    char buffer[buff_size];

    int total = 0;

    int totalBytesRead = 0;
    int bytesWritten = 0;

    do {

        bytesRead = read(STDIN_FILENO, buffer + total, buff_size - total);
        total += bytesRead;

    } while (bytesRead > 0);

    // if (total < buff_size){
    //     buffer[total] = '\0';
    // }

    char *command = strtok(buffer, "\n");

    if (strcmp("get", command) == 0) {

        if (buffer[total - 1] != '\n') {
            inval();
        }

        filename = strtok(NULL, "\n");

        fd = open(filename, O_RDONLY, 0);
        if (fd == -1) {
            inval();
        }

        totalBytesWritten = 0;

        do {
            int bytesWritten;
            totalBytesWritten = 0;

            bytesRead = read(fd, buffer, buff_size);

            if (bytesRead == -1) {
                close(fd);
                inval();
            }

            do {

                bytesWritten = write(
                    STDOUT_FILENO, buffer + totalBytesWritten, bytesRead - totalBytesWritten);
                totalBytesWritten += bytesWritten;
                if (bytesWritten == -1) {
                    close(fd);
                    inval();
                }

            } while (bytesWritten > 0 && totalBytesWritten < bytesRead);

        } while (bytesRead > 0);

        close(fd);

        exit(0);

    }

    else if (strcmp("set", command) == 0) {

        filename = strtok(NULL, "\n");

        fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0666);

        char *content_len = strtok(NULL, "\n");
        char *endptr;
        long con_len = strtol(content_len, &endptr, 10);
        if (con_len == 0) {
            write(STDOUT_FILENO, "OK\n", strlen("OK\n"));
            exit(0);
        }

        int command_len = total - 3 - 3 - strlen(filename) - strlen(content_len);
        char *content = &buffer[total - command_len];

        str_write(fd, content, command_len);

        do {

            bytesRead = read(STDIN_FILENO, buffer, buff_size);
            totalBytesWritten = 0;

            if (bytesRead == -1) {
                close(fd);
                inval();
            }

            //bytesWritten = write(fd, buffer, bytesRead);

            //  do {
            // bytesWritten
            //     = write(fd, buffer + totalBytesWritten, bytesRead - totalBytesWritten);
            // totalBytesWritten += bytesWritten;

            bytesWritten = write(fd, buffer, bytesRead);
            totalBytesWritten += bytesWritten;

            if (bytesWritten == -1) {
                close(fd);
                inval();
            }

            //   } while (bytesWritten > 0 && totalBytesWritten < bytesRead);

        } while (bytesRead != 0 && totalBytesRead < con_len);

        write(STDOUT_FILENO, "OK\n", strlen("OK\n"));
        close(fd);
        exit(0);

    } else {
        inval();
    }
}

//PASSBYTES()
// do{
//     int bytesRead = read(srcfd, buffer, BUF_SIZE);

//     do{
//         bytesWritten = write(destfd, buffer+totalBytesWritten, bytesRead - totalBytesWritten);
//         totalBytesWritten += bytesWritten;
//     } while (bytesWritten > 0 && totalBytesWritten < numBytes);

//     totalBytesRead += bytesRead;

// } while (bytesRead > 0 && totalBytesRead < numBytes);

//WRITEBYTES()
//USE JUST THIS FOR SET
// do{
//         bytesWritten = write(destfd, buffer+totalBytesWritten, bytesRead - totalBytesWritten);
//         totalBytesWritten += bytesWritten;
//     } while (bytesWritten > 0 && totalBytesWritten < numBytes);

// set() {
//     open file (wronly ocreat otrunc)
//     writeBytes(fd, buffer, len);
//     passBytes(stdin (srcfd), fd (destfd), remaining);
//     print "OK\n"
//     close file
// }

// get(){
//     open file (rdonly)
//     get file size (use fstat())
//     passBytes(fd, stdout, fileSize)

//     close file
// }
