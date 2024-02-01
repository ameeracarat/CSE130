#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
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
            exit(1);
        }
        if (result == 0)
            break;

        num_written += result;
        num_chars -= result;
    }
}

int isNum(const char *str) {
    while (*str) {
        if (!isdigit(*str)) {
            return 0; // Not a digit
        }
        str++;
    }
    return 1; // All characters are digits
}

int main(void) {

    int fd;
    char *filename;
    int totalBytesWritten = 0;

    int buff_size = 10000;

    int bytesRead;

    char buffer[buff_size];

    int total = 0;

    int totalBytesRead = 0;
    int bytesWritten = 0;

    //fill up buffer to get command
    //do it in a loop to ensure the buffer is filled as much as possible
    do {
        bytesRead = read(STDIN_FILENO, buffer + total, buff_size - total);
        total += bytesRead;
    } while (bytesRead > 0);

    //use strtok to get command
    char *command = strtok(buffer, "\n");

    //GET
    if (strcmp("get", command) == 0) {

        //check if command ends in newline
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

        if (isNum(content_len) == 0) {
            inval();
        }

        char *endptr;
        long con_len = strtol(content_len, &endptr, 10);

        if (con_len == 0) {
            write(STDOUT_FILENO, "OK\n", strlen("OK\n"));
            exit(0);
        }

        int len_of_command = total - 3 - 3 - strlen(filename) - strlen(content_len);
        char *content = &buffer[total - len_of_command];

        str_write(fd, content, len_of_command);

        do {

            bytesRead = read(STDIN_FILENO, buffer, buff_size);
            totalBytesWritten = 0;

            if (bytesRead == -1) {
                close(fd);
                inval();
            }

            str_write(fd, buffer, bytesRead);

            if (bytesWritten == -1) {
                close(fd);
                inval();
            }

        } while (bytesRead != 0 && totalBytesRead < con_len);

        write(STDOUT_FILENO, "OK\n", strlen("OK\n"));
        close(fd);
        exit(0);

    } else {
        inval();
    }
}
