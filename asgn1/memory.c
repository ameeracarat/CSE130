#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h>

void inval(void) {
    write(2, "Invalid Command\n", strlen("Invalid Command\n"));
    exit(1);
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
    

    do{

        bytesRead = read(STDIN_FILENO, buffer+total, buff_size-total);
        total += bytesRead;

    } while(bytesRead > 0 );


    //printf("%c\n", buffer[total-1]);
    

    char *command = strtok(buffer, "\n");
    //create new variable for filename

    if (strcmp("get", command) == 0) {

        if (buffer[total-1] != '\n'){
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


            bytesRead = read(fd, buffer, buff_size);


            if (bytesRead == -1) {
                inval();
            }

            //do{

                 bytesWritten = write(STDOUT_FILENO, buffer, bytesRead);
                if (bytesWritten == -1) {
                    inval();
                }

           // }while (bytesWritten < bytesRead);

        } while (bytesRead != 0);

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

        char *content = strtok(NULL, "");

        int bytesWritten = write(fd, content, con_len);

        do {

            bytesRead = read(STDIN_FILENO, buffer, buff_size);

            //while(totalBytesWritten < bytesRead){

            if (bytesRead > 0 && (con_len > totalBytesWritten)) {
                if ((bytesRead + totalBytesWritten) > con_len) {
                    bytesToRead = totalBytesWritten + bytesRead - con_len;
                } else {
                    bytesToRead = bytesRead;
                }

                bytesWritten = write(fd, buffer, bytesToRead);
                totalBytesWritten += bytesToRead;
            }

            //}

        } while (bytesRead != 0 || (totalBytesWritten == con_len));

        write(STDOUT_FILENO, "OK\n", strlen("OK\n"));
        exit(0);

    } else {
        inval();
    }

    // do{

    //  int bytesRead = read(STDOUT_FILENO, buffer, buff_size);
    //    //while(totalBytesWritten)
    //    while(totalBytesWritten < bytesRead){
    //     bytesWritten = write(STDOUT_FILENO, buffer, totalBytesWritten, bytesRead);
    //     totalBytesWritten += bytesWritten;
    //    }

    //  }
}



do{
    int bytesRead = read(srcfd, buffer, BUF_SIZE);

    do{
        bytesWritten = write(destfd, buffer+totalBytesWritten, bytesRead - totalBytesWritten);
        totalBytesWritten += bytesWritten;
    } while (bytesWritten > 0 && totalBytesWritten < numBytes);

    totalBytesRead += bytesRead;

} while (bytesRead > 0 && totalBytesRead < numBytes);
