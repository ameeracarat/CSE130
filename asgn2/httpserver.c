
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include "asgn2_helper_funcs.h"
#include <regex.h>
#include <sys/stat.h>
#include <errno.h>

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

int main(int argc, char *argv[]) {

    int fd;

    regex_t regex;
    regmatch_t pmatch[10];
    //char filename[];

    char *filename = NULL;

    char *re
        = "^([A-Z]{1,8}) /([a-zA-Z0-9.-/_]{1,63}) HTTP/([0-9]\\.[0-9])\r\n([a-zA-Z0-9.-]{1,128}: "
          "[ -~]{1,128}\r\n)*\r\n";

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

        bytes_read = read_until(sock, buffer, 2048, "\r\n\r\n");

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

                            get_put = 0;

                        } else if (strcmp(matched_string, "PUT") == 0) {

                            get_put = 1;
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

                    if (i == 3) {

                        char matched_string[end - start + 1];
                        strncpy(matched_string, buffer + start, end - start);
                        matched_string[end - start] = '\0';

                        if (strcmp(matched_string, "1.1") != 0) {

                            char message505[] = "HTTP/1.1 505 Version Not Supported\r\nContent-Length: "
                                                "22\r\n\r\nVersion Not Supported\n";
                            ssize_t writ5 = write_n_bytes(sock, message505, strlen(message505));
                            get_put = 9;
                        }
                    }
                }
            }
        } else {

            char message400[]
                = "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n";
            ssize_t writ3 = write_n_bytes(sock, message400, strlen(message400));
        }

        //IF GET
        if (get_put == 0) {

            fd = open(filename, O_RDONLY, 0);
            if (fd == -1) {

                //file does not exist
                if (errno == ENOENT) {
                    char message404[]
                        = "HTTP/1.1 404 Not Found\r\nContent-Length: 10\r\n\r\nNot Found\n";
                    ssize_t writ2 = write_n_bytes(sock, message404, strlen(message404));
                }
                // else {
                //     char message403[]
                //         = "HTTP/1.1 403 Forbidden\r\nContent-Length: 10\r\n\r\nForbidden\n";
                //     ssize_t writ3 = write_n_bytes(sock, message403, strlen(message403));
                //     //fprintf(stderr, "Failed to open file for get\n");
                // }
            } else {

                struct stat statbuf;
                if (stat(filename, &statbuf) == -1) {
                    perror("stat");
                    exit(EXIT_FAILURE);
                }

                if (S_ISDIR(statbuf.st_mode)) {
                    char message403[]
                        = "HTTP/1.1 403 Forbidden\r\nContent-Length: 10\r\n\r\nForbidden\n";
                    ssize_t writ3 = write_n_bytes(sock, message403, strlen(message403));
                    //fprintf(stderr, "Failed to open file for get\n");
                }

                char message[] = "HTTP/1.1 200 OK\r\nContent-Length: ";

                ssize_t writ = write_n_bytes(sock, message, strlen(message));

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

                writ = write_n_bytes(sock, "\r\n\r\n", strlen("\r\n\r\n"));

                ssize_t passed_bytes = pass_n_bytes(fd, sock, fileSize);
            }

            close(fd);

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
                //ssize_t bytes_written = write(fd, body_start, body_length);
                ssize_t bytes_written = write_n_bytes(fd, body_start, body_length);
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

            close(fd);

        }

        else if (get_put==2){
            char message501[]
                = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n";
            ssize_t writ4 = write_n_bytes(sock, message501, strlen(message501));
        }

        free(filename);
        close(sock);
    }

    return 0;
}
