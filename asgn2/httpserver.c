
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include "asgn2_helper_funcs.h"
#include <regex.h>
#include <sys/stat.h>

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

int main(int argc, char *argv[]) {

    int fd;

    regex_t regex;
    regmatch_t pmatch[10];
    //char filename[];

    char *filename = NULL;

    char *re = "^([A-Z]{1,8}) /([a-zA-Z0-9.-]{1,63}) HTTP/([0-9]\\.[0-9])\r\n([a-zA-Z -]{1,63}: "
               "[1-9]\r\n)*\r\n";

    //char *re2 = "^([A-Z]{1,8}) /([a-zA-Z0-9.-]{1,63}) HTTP/([0-9]\\.[0-9])\r\n(.*)\r\n";

    static const char *s
        = "PUT /foo.txt HTTP/1.1\r\nContent-Length: 21\r\n\r\nHello foo, I am World";

    if (argc != 2) {
        fprintf(stderr, "Invalid Port\n");
        exit(1);
    }

    int port = atoi(argv[1]);
    if (port < 1 || port > 65535) {
        fprintf(stderr, "Invalid Port\n");
        exit(1);
    }

    Listener_Socket *socket = calloc(1, sizeof(Listener_Socket));
    int sock = listener_init(socket, port);

    if (sock == -1) {
        fprintf(stderr, "Invalid Port\n");
        exit(1);
    }

    while (1) {
        sock = listener_accept(socket);

        int buff_size = 2048;
        char buffer[buff_size];

        ssize_t bytes_read = 0;

        // bytes_read = read_n_bytes(sock, buffer, buff_size);
        //bytes_read =  read_until(fd, buffer, buff_size, "\r\n\r\n");

        bytes_read = read_until(sock, buffer, 2048, "\r\n\r\n");

        printf("bytess read: %zd\n", bytes_read);

        for (int i = 0; i < bytes_read; ++i) {
            printf("%c", buffer[i]);
        }
        printf("\n\n\n");

        if (regcomp(&regex, re, REG_NEWLINE | REG_EXTENDED)) {
            fprintf(stderr, "Failed to compile regex\n");
            exit(EXIT_FAILURE);
        }

        int get_put = 2;

        if (regexec(&regex, buffer, ARRAY_SIZE(pmatch), pmatch, 0) == 0) {
            for (int i = 0; i < ARRAY_SIZE(pmatch); ++i) {
                regoff_t start = pmatch[i].rm_so;
                regoff_t end = pmatch[i].rm_eo;

                if (start != -1 && end != -1) {
                    printf("%.*s\n", (int) (end - start), buffer + start);

                    if (i == 1) {

                        char matched_string[end - start + 1];
                        strncpy(matched_string, buffer + start, end - start);
                        matched_string[end - start] = '\0';

                        // Check if the extracted string is equal to "GET"
                        if (strcmp(matched_string, "GET") == 0) {
                            printf("Second element is \"GET\"\n");
                            get_put = 0;

                        } else if (strcmp(matched_string, "PUT") == 0) {
                            printf("Second element is \"PUT\"\n");
                            get_put = 1;
                        } else {
                            printf("Second element is not get or put\n");
                        }
                    }

                    if (i == 2) {

                        filename = malloc(end - start + 1);
                        if (filename == NULL) {
                            fprintf(stderr, "Memory allocation failed\n");
                            exit(EXIT_FAILURE);
                        }

                        strncpy(filename, buffer + start, end - start);
                        filename[end - start] = '\0';
                        printf("Filename: %s\n", filename);
                    }
                }
            }
        } else {
            fprintf(stderr, "No match found\n");
        }

        //IF GET
        if (get_put == 0) {

            fd = open(filename, O_RDONLY, 0);
            if (fd == -1) {
                fprintf(stderr, "Failed to open file\n");
                exit(1);
            }

            off_t end_position = lseek(fd, 0, SEEK_END);
            if (end_position == -1) {
                perror("Failed to seek end of file");
                close(fd);
                return 1;
            }

            // Get current file position, which is the size of the file
            off_t size = lseek(fd, 0, SEEK_CUR);
            if (size == -1) {
                perror("Failed to get file size");
                close(fd);
                return 1;
            }

            // Reset file pointer to beginning of file
            if (lseek(fd, 0, SEEK_SET) == -1) {
                perror("Failed to seek beginning of file");
                close(fd);
                return 1;
            }

            char message[] = "HTTP/1.1 200 OK\r\nContent-Length: ";

            ssize_t writ = write_n_bytes(sock, message, sizeof(message));

            struct stat fileStat;
            if (fstat(fd, &fileStat) == -1) {
                perror("Error getting file information");
                close(fd);
                return 1;
            }
            off_t fileSize = fileStat.st_size;

            char sizeString[1000];
            snprintf(sizeString, 1000, "%lld", (long long) fileSize);

            writ = write_n_bytes(sock, sizeString, strlen(sizeString));

            writ = write_n_bytes(sock, "\n", strlen("\n"));

            ssize_t passed_bytes = pass_n_bytes(fd, sock, 99);

        }

        //IF PUT
        else if (get_put == 1) {

            fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0666);
            if (fd == -1) {
                fprintf(stderr, "Failed to open file\n");
                exit(1);
            }

            char *body_start = strstr(buffer, "\r\n\r\n");
            if (body_start != NULL) {
                // Advance pointer to the beginning of the message body
                body_start += 4; // Move past "\r\n\r\n"

                // Calculate the length of the message body
                size_t body_length = bytes_read - (body_start - buffer);

                // Write out the message body
                ssize_t bytes_written = write(fd, body_start, body_length);
                if (bytes_written == -1) {
                    fprintf(stderr, "Failed to write message body\n");
                    exit(EXIT_FAILURE);
                }
            } else {
                fprintf(stderr, "Double CRLF not found\n");
                exit(EXIT_FAILURE);
            }
            //ssize_t passed_bytes = pass_n_bytes(fd, sock, con_len - bytes_written);
            // ssize_t bytes_written = write_n_bytes(fd, buf[], n)

        }

        else {
            printf("request not supported");
        }

        //ssize_t bytes_written = write_n_bytes(sock, buffer, bytes_read);
        // printf("bytes written: %zd", bytes_written);

        free(filename);

        close(sock);
    }

    return 0;
}
