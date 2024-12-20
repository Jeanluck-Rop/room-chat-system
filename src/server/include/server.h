#ifndef SERVER_H
#define SERVER_H

#include "cJSON.h"
#include <pthread.h>

/* Define a client struct to represents a connected client */
typedef struct Client {
  int socket_fd;       // Socket descriptor for the client's connection.
  char username[9];    // Username of the client (maximum 8 characters + null terminator).
  char status[10];     // Client status (e.g., "online", "offline").
  struct Client *next; // Pointer to the next client in a linked list.
  pthread_t thread;    // Thread associated with the client to handle its connection.
} Client;


void start_server(int port);

#endif // SERVER_H
