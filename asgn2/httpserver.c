
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include "asgn2_helper_funcs.h"
#include <regex.h>

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

int main(int argc, char *argv[]) {

    int fd;

    regex_t regex;
    regmatch_t pmatch[4];
    //char filename[];

    char *filename = NULL;

    char *re = "^([A-Z]{1,8}) /([a-zA-Z0-9._]{1,63}) (HTTP)$";

    static const char *s = "GET /foo.txt HTTP";

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

        printf("check\n");

        int buff_size = 2048;
        char buffer[buff_size];

        ssize_t bytes_read = 0;

        bytes_read = read_n_bytes(sock, buffer, buff_size);

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

        if (regexec(&regex, s, ARRAY_SIZE(pmatch), pmatch, 0) == 0) {
            for (int i = 0; i < ARRAY_SIZE(pmatch); ++i) {
                regoff_t start = pmatch[i].rm_so;
                regoff_t end = pmatch[i].rm_eo;

                if (start != -1 && end != -1) {
                    printf("%.*s\n", (int) (end - start), s + start);

                    if (i == 1) {

                        char matched_string[end - start + 1];
                        strncpy(matched_string, s + start, end - start);
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

                        strncpy(filename, s + start, end - start);
                        filename[end - start] = '\0';
                        printf("Filename: %s\n", filename);
                    }
                }
            }
        } else {
            fprintf(stderr, "No match found\n");
        }

        if (get_put == 0) {

            fd = open(filename, O_RDONLY, 0);
            if (fd == -1) {
                fprintf(stderr, "Failed to open file\n");
                exit(1);
            }

            ssize_t passed_bytes = pass_n_bytes(fd, sock, 99);

        }

        else if (get_put == 1) {

            fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0666);
            if (fd == -1) {
                fprintf(stderr, "Failed to open file\n");
                exit(1);
            }
            //ssize_t passed_bytes = pass_n_bytes(, fd, 99);

        }

        else {
            printf("request not supported");
        }

        ssize_t bytes_written = write_n_bytes(sock, buffer, bytes_read);
        printf("bytes written: %zd", bytes_written);

        free(filename);

        close(sock);
    }

    return 0;
}
