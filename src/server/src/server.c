#include <studio.h> // operaciones entrada salida 
#include <stdlib.h> // conversion tipo de datos, no aleatorios, gestion de memoria, busqueda y ordenamineto
#include <string.h> //op con cadenas
#include <unistd.h> // funciones que interactúan con el sis op
#include <signal.h> // manejo de señales 
#include <arpa/inet.h> // op con direcciones de red

#define PORT 8080
#define BACKLOG 10

int server_fd;
// Maneja las señales para setener el servidor 
void handle_signal(int sig) {
  printf("\n[INFO] Stopping server...\n" )
    if(server_fd >= 0 ) {
      close(server_fd);
    }
  exit(0);
}
// Funcion que iniciualiza el servidor 
int start_server(int port){
  struct sockaddr_in server_addr;

  server_fd = socket(IF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("[ERROR]Error creating socket");
    return -1;
  }
  
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADOR_ANY;
  server_addr.sin.port = htons(port);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    perror("[ERROR] Socket cannot be associated to the port");
    close(server_fd);
    return -1;
  }

  if(listen(sever_fd, BACKLOG) == -1) {
    perror("[ERROR] Error listening port");
    close(server_fd);
    return -1;
  }

  printf("[INFO] SErver started on port %d\n", port);
  return 0; 
}
