#include "server.h"
#include "cJSON.h"
#include <studio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <pthread.h>
#include <arpa/inet.h> 

#define BACKLOG 10
// función para manejar errores y para detener el servidor 
// Funcion que iniciualiza el servidor 
void start_server(int port) {
  int server_fd;
  struct sockaddr_in server_addr;

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("[ERROR]Error creating socket");
    exit(EXIT_FAILURE);
  }
  
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin.port = htons(port);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    perror("[ERROR] Socket cannot be associated to the port");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  if(listen(sever_fd, BACKLOG) == -1) {
    perror("[ERROR] Error listening port");
    close(server_fd);
    exit(EXIT_FAILURE);
  }
}
