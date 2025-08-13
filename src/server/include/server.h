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
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "cJSON.h"
#include "room.h"
#include "message.h"

/* Client struct to represent a connected client */
typedef struct Client
{
  int socket_fd;          // Socket descriptor for the client's connection.
  char status[10];        // Client status, ACTIVE, AWAY, BUSY.
  char username[9];       // Client username (maximum 8 characters + null terminator).
  pthread_t thread;       // Thread associated with the client to handle its connection.
  int invited_count;      // Number of current invitations.
  char** invited_rooms;   // List of roomnames the client WASs invited to
  int invited_capacity;   // To allocate size memory for invitations list.
  bool is_disconnected;   // For stop handling a connected client.

  SSL *ssl;               // Handle encryptment of message received and sent.
  
  struct Client *next;    // Pointer to the next client in a linked list.
}
  Client;

/**
 * Sends a message to a specific client.
 * If the client is disconnected or NULL, the function returns immediately.
 * Logs an error and exits if sending the message fails.
 *
 * @param client Pointer to the target client.
 * @param message The message string to send.
 **/
void send_message(Client *client, const char* message);

/**
 * Function to initialize and start the server on the specified port.
 * This function creates the server socket, binds it to the port,
 * starts listening, and enters the server loop to accept clients.
 * Also sets up signal handling for clean termination.
 *
 * @param port Port number on which the server will listen for incoming connections.
 **/
void start_server(int port);

#endif // SERVER_H
