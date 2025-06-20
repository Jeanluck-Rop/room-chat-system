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
#include "room.h"
#include "message.h"

/* Define a client struct to represents a connected client */
typedef struct Client {
  int socket_fd;       //Socket descriptor for the client's connection.
  char username[9];    //Client username (maximum 8 characters + null terminator).
  char status[10];     //Client status
  struct Client *next; //Pointer to the next client in a linked list.
  pthread_t thread;    //Thread associated with the client to handle its connection.
  char **invited_rooms;
  int invited_count;
  int invited_capacity;
  bool is_disconnected;
} Client;

void print_message(const char *text, char type);
void disconnect_client(Client *client);
void send_message(Client *client, const char *message);
void broadcast_message(const char *message,  int sender_socket);
Client *find_client_by_username(const char *username);
bool was_invited(Client *client, const char *roomname);
bool mark_as_invited(Client *client, const char *roomname);
void room_response(Client *client, const char *operation, const char *result, const char *roomname);
void invalid_response(Client *client, const char *result);
void leave_room(Client *client, Message *incoming_message);
void room_text(Client *client, Message *incoming_message);
void get_room_users(Client *client, Message *incoming_message);
void join_room(Client *client, Message *incoming_message);
void invite_guests(Client *client, Message *incoming_message);
void new_room(Client *client, Message *incoming_message);
void public_text(Client *client, Message *incoming_message);
void private_text(Client *client, Message *incoming_message);
void users_list(Client *client, Message *incoming_message);
void change_status(Client *client, Message *incoming_message);
bool handle_identify(Client *client, Message *incoming_message);
bool client_actions(Client *client, Message *incoming_message);
void *handle_client(void *arg);
void server_cycle(int server_fd);
void start_server(int port);

#endif // SERVER_H
