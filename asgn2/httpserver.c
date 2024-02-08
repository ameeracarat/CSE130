
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include "asgn2_helper_funcs.h"
#include <regex.h>

int main(int argc, char *argv[]) {

    regex_t regex;

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

        //do{
        bytes_read = read_n_bytes(sock, buffer, buff_size);

        printf("bytes read: %zd\n", bytes_read);

        ssize_t bytes_written = write_n_bytes(sock, buffer, bytes_read);
        printf("bytes written: %zd", bytes_written);

        //  } while (bytes_read > 0);

        // char *error_code = "101";

        //  write_n_bytes(STDOUT_FILENO, error_code, 3);

        close(sock);
    }

    return 0;
}
