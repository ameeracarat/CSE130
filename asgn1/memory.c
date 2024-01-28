#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


#include <fcntl.h>



int main(void) {

  int fd;
  char *filename;
  int totalBytesWritten = 0;

  int buff_size = 100;

  int bytesRead;
  
  char buffer[buff_size];
  read(STDIN_FILENO, buffer, buff_size);


  char *command = strtok(buffer, "\n");
  //create new variable for filename
  

  if (strcmp("get", command)==0){

    filename = strtok(NULL, "\n");
     
    fd = open(filename, O_RDONLY, 0);

    totalBytesWritten = 0;

    do{
       bytesRead = read(fd, buffer, buff_size);
      int bytesWritten = write(STDOUT_FILENO, buffer, bytesRead);

    } while (bytesRead != 0);

    

  }

  else if (strcmp("set", command)==0){

    filename = strtok(NULL, "\n");
   
    fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    char *content_len = strtok(NULL, "\n");
    char *endptr;
    long convertedValue = strtol(content_len, &endptr, 10);

    char *content = strtok(NULL, "");


    int bytesWritten = write(fd, content, convertedValue);


    do{

      bytesRead = read(STDIN_FILENO, buffer, buff_size);

    }while (bytesRead != 0);


    


  }

  do{

   int bytesRead = read(STDOUT_FILENO, buffer, buff_size);
     //while(totalBytesWritten)
     while(totalBytesWritten < bytesRead){
      bytesWritten = write(STDOUT_FILENO, buffer, totalBytesWritten, bytesRead);
      totalBytesWritten += bytesWritten;
     }
    
   }
  
  
  






}
