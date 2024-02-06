
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include "asgn2_helper_funcs.h"

int main(int argc, char* argv[]){

    if (argc != 2){
        fprintf(stderr, "Invalid Port\n");
        exit(1);
    }
   
    int port = atoi(argv[1]);
    if (port < 1 || port > 65535){
        fprintf(stderr, "Invalid Port\n");
        exit(1);
    }
   
    Listener_Socket *socket = calloc(1, sizeof(Listener_Socket));
    int sock = listener_init(socket, port);

    
    if (sock == -1){
        fprintf(stderr, "Invalid Port\n");
        exit(1);
    }

    //while (1){
        sock = listener_accept(socket);

        int buff_size = 10000;

         char buffer[buff_size];

        ssize_t bytes_read = read_n_bytes(sock, buffer, buff_size);

        for (int i = 0; i < bytes_read; i++){
            printf("%c", buffer[i]);
        }



   // }
    

    return 0;



    
   

    






}
