
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
#include <arpa/inet.h>
#include <err.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>

#include "List.h"

#include "queue.h"
#include "rwlock.h"

#define MAX_URI_LENGTH 200

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define DEFAULT_THREADS 4 // Default number of worker threads

int num_threads; // Number of worker threads
queue_t *request_queue; // Queue for holding incoming requests

List L;

//reader mutex (get)
//writer mutex (put)

//pthread mutex_t;

pthread_mutex_t reader_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writer_mutex = PTHREAD_MUTEX_INITIALIZER;

struct KeyValue {
    char key[128];
    char value[128];
};

struct FileLock {
    rwlock_t *lock;
    char URI[MAX_URI_LENGTH];
};

void get(int sock, const char *filename, const char *buffer, regex_t regex) {

    pthread_mutex_lock(&reader_mutex);

    rwlock_t *fileLockLock = NULL;
    bool found = false;
    moveFront(L);
    while ((cur_index(L) < length(L)) && (found == false) && (length(L) > 0)) {
        struct FileLock *filelock = get_cursor(L);
        moveNext(L);
        if (strcmp(filename, filelock->URI) == 0) { // Compare filenames
            printf("Found\n"); // Print "found" message
            found = true;
            fileLockLock = filelock->lock;
        }
    }

    if (!found) {
        struct FileLock *fileLock = malloc(sizeof(struct FileLock));
        if (fileLock == NULL) {
            perror("Error allocating memory for file lock");
            exit(EXIT_FAILURE);
        }
        fileLock->lock = rwlock_new(N_WAY, 2);
        if (fileLock->lock == NULL) {
            perror("Error creating file lock");
            exit(EXIT_FAILURE);
        }
        strncpy(fileLock->URI, filename, MAX_URI_LENGTH);
        fileLock->URI[MAX_URI_LENGTH - 1] = '\0'; // Ensure null-terminated
        fileLockLock = fileLock->lock;
    }

    reader_lock(fileLockLock);


    regmatch_t pmatch2[10];
    char *re2 = "([a-zA-Z0-9.-]{1,128}): ([ -~]{1,128})\r\n"; 

    if (regcomp(&regex, re2, REG_NEWLINE | REG_EXTENDED)) {
        exit(EXIT_FAILURE);
    }

    int offset = 0;
    struct KeyValue keyValues[100];
    int numKeyValuePairs = 0;

    while (regexec(&regex, buffer + offset, ARRAY_SIZE(pmatch2), pmatch2, 0) == 0) {

        regoff_t start0 = pmatch2[0].rm_so;
        regoff_t end0 = pmatch2[0].rm_eo;

        regoff_t start1 = pmatch2[1].rm_so;
        regoff_t end1 = pmatch2[1].rm_eo;

        regoff_t start2 = pmatch2[2].rm_so;
        regoff_t end2 = pmatch2[2].rm_eo;

        if (start0 != -1 && end0 != -1) {

            strncpy(keyValues[numKeyValuePairs].key, buffer + start1 + offset, end1 - start1);
            keyValues[numKeyValuePairs].key[end1 - start1] = '\0'; // Null-terminate the string
            printf("Value: %s\n", keyValues[numKeyValuePairs].key);
            strncpy(keyValues[numKeyValuePairs].value, buffer + start2 + offset, end2 - start2);
            keyValues[numKeyValuePairs].value[end2 - start2] = '\0'; // Null-terminate the string

            numKeyValuePairs++; // Increment the counter
        }

        offset += end0;
    }
    printf("key val pairs: %d\n", numKeyValuePairs);

    int content_LENGTH = 0;
    int request_ID = 0;

    for (int i = 0; i < numKeyValuePairs; i++) {
        //printf("Key: %s, Value: %s\n", keyValues[i].key, keyValues[i].value);

        // Check if the key is "Content-Length:"
        if (strcmp(keyValues[i].key, "Content-Length") == 0) {
            // Convert the value to an integer and store it in content_LENGTH
            content_LENGTH = atoi(keyValues[i].value);
            printf("Content-Length found: %d\n", content_LENGTH);
        }

        // Check if the key is "request_ID:"
        if (strcmp(keyValues[i].key, "Request-Id") == 0) {
            // Convert the value to an integer and store it in content_LENGTH
            request_ID = atoi(keyValues[i].value);
            printf("Request id found: %d\n", content_LENGTH);
        }
    }

    int fd = open(filename, O_RDONLY, 0);
    if (fd == -1) {

        //file does not exist
        if (errno == ENOENT) {
            char message404[] = "HTTP/1.1 404 Not Found\r\nContent-Length: 10\r\n\r\nNot Found\n";
            write_n_bytes(sock, message404, strlen(message404));
        }

    } else {

        struct stat statbuf;
        if (stat(filename, &statbuf) == -1) {
            perror("stat");
            exit(EXIT_FAILURE);
        }

        if (S_ISDIR(statbuf.st_mode)) {
            char message403[] = "HTTP/1.1 403 Forbidden\r\nContent-Length: 10\r\n\r\nForbidden\n";
            write_n_bytes(sock, message403, strlen(message403));
        }

        char message[] = "HTTP/1.1 200 OK\r\nContent-Length: ";

        write_n_bytes(sock, message, strlen(message));

        struct stat fileStat;
        if (fstat(fd, &fileStat) == -1) {
            perror("Error getting file information");
            close(fd);
            exit(1);
        }
        off_t fileSize = fileStat.st_size;

        char sizeString[1000];
        snprintf(sizeString, 1000, "%lld", (long long) fileSize);
        write_n_bytes(sock, sizeString, strlen(sizeString));

        write_n_bytes(sock, "\r\n\r\n", strlen("\r\n\r\n"));

        pass_n_bytes(fd, sock, fileSize);
    }

   

    

    

    fprintf(stderr, "GET,/%s,200,%d\n", filename, request_ID);

    reader_unlock(fileLockLock);

    pthread_mutex_unlock(&reader_mutex);

    close(fd);
}

void put(int sock, const char *filename, const char *buffer, ssize_t bytes_read, regex_t regex) {

    //mutex lock
    pthread_mutex_lock(&writer_mutex);

    //todo: declare variable for lock
    rwlock_t *fileLockLock = NULL;
    bool found = false;
    moveFront(L);
    while ((cur_index(L) < length(L)) && (found == false) && (length(L) > 0)) {
        struct FileLock *filelock = get_cursor(L);
        moveNext(L);
        if (strcmp(filename, filelock->URI) == 0) { // Compare filenames
            printf("Found\n"); // Print "found" message
            found = true;
            fileLockLock = filelock->lock;
        }
    }

    if (!found) {
        struct FileLock *fileLock = malloc(sizeof(struct FileLock));
        if (fileLock == NULL) {
            perror("Error allocating memory for file lock");
            exit(EXIT_FAILURE);
        }
        fileLock->lock = rwlock_new(N_WAY, 2);
        if (fileLock->lock == NULL) {
            perror("Error creating file lock");
            exit(EXIT_FAILURE);
        }
        strncpy(fileLock->URI, filename, MAX_URI_LENGTH);
        fileLock->URI[MAX_URI_LENGTH - 1] = '\0'; // Ensure null-terminated
        fileLockLock = fileLock->lock;
    }

    writer_lock(fileLockLock);

    

    int exists = 0;

    if (access(filename, F_OK) != -1) {
        exists = 1;
    }

    int fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0666);
    if (fd == -1) {

        exit(1);
    }

    regmatch_t pmatch2[10];
    char *re2 = "([a-zA-Z0-9.-]{1,128}): ([ -~]{1,128})\r\n"; 

    if (regcomp(&regex, re2, REG_NEWLINE | REG_EXTENDED)) {
        exit(EXIT_FAILURE);
    }

    int offset = 0;
    struct KeyValue keyValues[100];
    int numKeyValuePairs = 0;

    while (regexec(&regex, buffer + offset, ARRAY_SIZE(pmatch2), pmatch2, 0) == 0) {

        regoff_t start0 = pmatch2[0].rm_so;
        regoff_t end0 = pmatch2[0].rm_eo;

        regoff_t start1 = pmatch2[1].rm_so;
        regoff_t end1 = pmatch2[1].rm_eo;

        regoff_t start2 = pmatch2[2].rm_so;
        regoff_t end2 = pmatch2[2].rm_eo;

        if (start0 != -1 && end0 != -1) {

            strncpy(keyValues[numKeyValuePairs].key, buffer + start1 + offset, end1 - start1);
            keyValues[numKeyValuePairs].key[end1 - start1] = '\0'; // Null-terminate the string
            printf("Value: %s\n", keyValues[numKeyValuePairs].key);
            strncpy(keyValues[numKeyValuePairs].value, buffer + start2 + offset, end2 - start2);
            keyValues[numKeyValuePairs].value[end2 - start2] = '\0'; // Null-terminate the string

            numKeyValuePairs++; // Increment the counter
        }

        offset += end0;
    }
    printf("key val pairs: %d\n", numKeyValuePairs);

    int content_LENGTH = 0;
    int request_ID = 0;

    for (int i = 0; i < numKeyValuePairs; i++) {
        //printf("Key: %s, Value: %s\n", keyValues[i].key, keyValues[i].value);

        // Check if the key is "Content-Length:"
        if (strcmp(keyValues[i].key, "Content-Length") == 0) {
            // Convert the value to an integer and store it in content_LENGTH
            content_LENGTH = atoi(keyValues[i].value);
            printf("Content-Length found: %d\n", content_LENGTH);
        }

        // Check if the key is "request_ID:"
        if (strcmp(keyValues[i].key, "Request-Id") == 0) {
            // Convert the value to an integer and store it in content_LENGTH
            request_ID = atoi(keyValues[i].value);
            printf("Request id found: %d\n", content_LENGTH);
        }
    }

    char *body_start = strstr(buffer, "\r\n\r\n");
    if (body_start != NULL) {
        // Advance pointer to the beginning of the message body
        body_start += 4; // Move past "\r\n\r\n"

        // Calculate the length of the message body
        size_t body_length = bytes_read - (body_start - buffer);

        ssize_t bytes_written = write_n_bytes(fd, body_start, body_length);

        if (bytes_written == -1) {

            exit(EXIT_FAILURE);
        }

        pass_n_bytes(sock, fd, content_LENGTH - bytes_written);
    } else {

        exit(EXIT_FAILURE);
    }

    if (exists == 1) {
        char message200[] = "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nOK\n";
        write_n_bytes(sock, message200, strlen(message200));
    } else {
        char message201[] = "HTTP/1.1 201 Created\r\nContent-Length: 8\r\n\r\nCreated\n";
        write_n_bytes(sock, message201, strlen(message201));
    }

    if (exists == 1) {
        fprintf(stderr, "PUT,/%s,200,%d\n", filename, request_ID);

    } else {

        fprintf(stderr, "PUT,/%s,201,%d\n", filename, request_ID);
    }

    //mutex unlock
    writer_unlock(fileLockLock);
    pthread_mutex_unlock(&writer_mutex);

    close(fd);
}

void *worker_thread(void *args) {
    (void) args;

    while (1) {

        int *sock_ptr = NULL;

        queue_pop(request_queue, (void **) &sock_ptr);
        int sock = (intptr_t) sock_ptr; // Cast back to integer type
        printf("Socket value: %d\n", sock);

        regex_t regex;
        regmatch_t pmatch[10];

        char *filename = NULL;

        char *re = "^([A-Z]{1,8}) /([a-zA-Z0-9.-/_]{1,63}) "
                   "HTTP/([0-9]\\.[0-9])\r\n([a-zA-Z0-9.-]{1,128}: "
                   "[ -~]{1,128}\r\n)*\r\n";

        int buff_size = 2048;
        char buffer[buff_size];
        ssize_t bytes_read = 0;

        bytes_read = read_until(sock, buffer, 2048, "\r\n\r\n");

        if (regcomp(&regex, re, REG_NEWLINE | REG_EXTENDED)) {
            exit(EXIT_FAILURE);
        }

        buffer[bytes_read] = '\0';

        int get_put = 2;

        if (regexec(&regex, buffer, ARRAY_SIZE(pmatch), pmatch, 0) == 0) {
            for (int i = 0; i < (int) ARRAY_SIZE(pmatch); ++i) {
                regoff_t start = pmatch[i].rm_so;
                regoff_t end = pmatch[i].rm_eo;

                if (start != -1 && end != -1) {
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

                            exit(EXIT_FAILURE);
                        }

                        strncpy(filename, buffer + start, end - start);
                        filename[end - start] = '\0';
                        //printf("Filename: %s\n", filename);
                    }

                    if (i == 3) {

                        char matched_string[end - start + 1];
                        strncpy(matched_string, buffer + start, end - start);
                        matched_string[end - start] = '\0';

                        if (strcmp(matched_string, "1.1") != 0) {

                            char message505[]
                                = "HTTP/1.1 505 Version Not Supported\r\nContent-Length: "
                                  "22\r\n\r\nVersion Not Supported\n";
                            write_n_bytes(sock, message505, strlen(message505));
                            get_put = 9;
                        }
                    }
                }
            }
        } else {

            char message400[]
                = "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n";
            write_n_bytes(sock, message400, strlen(message400));
            get_put = 9;
        }

        //IF GET
        if (get_put == 0) {

            //reader lock
            get(sock, filename, buffer, regex);
            //reader unlock

        }

        //IF PUT

        else if (get_put == 1) {

            //writer lock

            put(sock, filename, buffer, bytes_read, regex);
            //writer unlock

        }

        else if (get_put == 2) {
            char message501[]
                = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n";
            write_n_bytes(sock, message501, strlen(message501));
        }

        //free(filename);
        close(sock);
    }

    return NULL;
}

int main(int argc, char *argv[]) {

    if (argc < 2 || argc > 4) {
        fprintf(stdout, "Usage: %s [-t threads] <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    num_threads = DEFAULT_THREADS; // Default number of threads
    int opt;
    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
        case 't':
            num_threads = atoi(optarg);
            if (num_threads < 1) {
                fprintf(stdout, "Invalid number of threads\n");
                exit(EXIT_FAILURE);
            }
            break;
        default: fprintf(stdout, "Usage: %s [-t threads] <port>\n", argv[0]); exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stdout, "Port number is missing\n");
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[optind]);
    if (port < 1 || port > 65535) {

        printf("port:%d \n", port);

        fprintf(stdout, "1: Invalid Port\n");
        exit(1);
    }

    Listener_Socket socket;
    int sock = listener_init(&socket, port);
    if (sock == -1) {
        fprintf(stdout, "2: Invalid Port\n");
        exit(1);
    }

    printf("num threads: %d, and port is: %d\n", num_threads, port);

    L = newList();

    request_queue = queue_new(num_threads); // Queue size equal to number of threads

    // Initialize worker threads
    pthread_t threads[num_threads];
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, &worker_thread, NULL);
    }

    while (1) {

        sock = listener_accept(&socket);
        if (sock != -1) {
            queue_push(request_queue, (void *) (intptr_t) sock);
        }
    }

    return 0;
}
