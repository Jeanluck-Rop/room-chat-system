#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h> 
#include <pthread.h>
#include <arpa/inet.h> 
#include "cJSON.h"

/* Define a client struct to represents a connected client */
typedef struct Client {
  int socket_fd;       //Socket descriptor for the client's connection.
  char username[9];    //Client username (maximum 8 characters + null terminator).
  char status[10];     //Client status
  struct Client *next; //Pointer to the next client in a linked list.
  pthread_t thread;    //Thread associated with the client to handle its connection.
} Client;

void print_message(const char *text, char type);
void disconnect_client(Client *client);
void handle_client(Client *client);

void handle_received_message(char *raw_message);

void server_cycle(int server_fd);
void start_server(int port);

#endif // SERVER_H
