#pragma once

#include <glib.h>
#include <csignal>
#include <iostream>
#include <arpa/inet.h>
#include <unordered_map>

#include "chat_counter.hpp"
#include "message.hpp"
#include "client.hpp"
#include "models.h"
#include "view.h"
#include "idle.h"

/**
 * @class Controller
 *
 * Manages the interaction between the graphical user interface and the client operations.
 * This class is responsible for sending user actions to the server, handling responses,
 * and processing messages to be shown to the user.
 * Implements the singleton pattern to ensure a single instance is used throughout the application.
 **/
class Controller
{
public:
  /**
   * Returns the singleton instance of the Controller class.
   * Ensures that only one instance of the controller exists during the lifetime of the application.
   **/
  static Controller& instance();                     //Singleton getter
  Controller(const Controller&) = delete;            //Deleted copy constructor
  Controller& operator=(const Controller&) = delete; //Deleted copy assignment operator

  /**
   * Attempts to establish a connection to the server and initialize the client.
   *
   * @param port Port number of the server.
   * @param server_ip IP address of the server to connect to.
   * @param user_name Client's username to be used for the connection.
   **/
  void try_connection(int port, const std::string& server_ip, const std::string& user_name);
    
  /**
   * Parses and routes an incoming raw JSON-formatted message string received from the server.
   *
   * @param raw_message the raw JSON string received from the server.
   **/
  void handle_message(const std::string& raw_message);
  
  /**
   * Validates the provided username format and availability.
   * Sends the identification request to the server.
   *
   * @param username Username to validate.
   * @return true if the username is valid, false otherwise.
   **/
  bool check_id(std::string& username);

  /**
   * Sends a message to the public chat.
   *
   * @param message_content The message content to be sent.
   **/
  void public_message(std::string& message_content);
  
  /**
   * Sends a private message to the target user, the recipient.
   *
   * @param message_content The message content to be sent.
   * @param recipient The recipient user.
   **/
  void direct_message(std::string& message_content, std::string& recipient);
  
  /**
   * Sends a request for change the client's status.
   * Sends only valid statuses ("AWAY", "ACTIVE", or "BUSY").
   *
   * @param status The new status.
   **/
  void change_status(std::string& status);

  /**
   * Sends a request to lists currently users in the chat.
   **/
  void chat_users();

  /**
   * Sends a request for create a new chat room.
   * Room name must be between 3 and 16 characters.
   *
   * @param roomname The new room name.
   **/
  void new_room(std::string& roomname);

  /**
   * Sends an invitation to multiple users to join a specific room.
   *
   * @param guest The users list to invite.
   * @param roomname The room to invite the users.
   **/
  void invite_users(char** guests, std::string& roomname);
  
  /**
   * Sends a request to join a specific chat room.
   *
   * @param roomname The room name to join.
   **/
  void join_room(std::string& roomname);

  /**
   * Sends a request to lists currently users in a chat room.
   *
   * @param roomname The room to request the users.
   **/
  void room_users(std::string& roomname);

  /**
   * Sends a message to all users in the specific room.
   *
   * @param message_content The message to send.
   * @param roomname The room to send the message.
   **/
  void room_message(std::string& message_content, std::string& roomname);

  /**
   * Sends a request to leave a specific room.
   *
   * @param roomname The room name to leave.
   **/
  void leave_room(std::string& roomname);

  /**
   * Retrieves the number of members connected on given chat.
   *
   * @param chat_name The name of the chat whose count is to be retrieved.
   * @return The current count associated with the given chat name.
   **/
  int get_chat_count(std::string& chat_name);
  
  /**
   * Sends a disconnect message to the server and closes the connection.
   **/
  void disconnect_user();

  /**
   * Schedules returning to the home page after a disconnection.
   **/
  void notify_disconnection();
  
private:
  Controller();  //private constructor
  ~Controller(); //private destructor

  /**
   * Processes a parsed Message object received from the server.
   * Determines the operation type and reacts according it.
   *
   * @param incoming_msg a parsed Message object containing operation and result info.
   **/
  void handle_response(const Message& incoming_msg);
  
  /**
   * Sends a dialog alert to the GUI thread.
   *
   * @param message The message to be displayed in the dialog.
   * @param type The type of dialog.
   **/
  void send_dialog(const std::string& message, DialogType type);

  /**
   * Triggers the creation of a new room in the GUI.
   *
   * @param room_name The name of the room to create.
   **/
  void create_room(const std::string& room_name);

  /**
   * Sends a new notification to the GUI.
   *
   * @param message The content of the notification.
   * @param roomname Optional name of the room related to the notification.
   * @param type The type of notification.
   **/
  void new_notify(const std::string& message, const std::string& roomname, NotifyType type);

  /**
   * Sends a chat message to the GUI.
   *
   * @param chat_name The name of the chat.
   * @param sender The name of the sender of the message.
   * @param content The content of the message.
   * @param chat_type The type of chat.
   * @param msg_type The type of message.
   **/
  void send_message(const std::string& chat_name, const std::string& sender, const std::string& content, ChatType chat_type, MessageType msg_type);

  /**
   * Updates the GUI with the current user list and their statuses.
   *
   * @param roomname The name of the room, or empty string for global context.
   * @param users_map A map of usernames to their statuses.
   **/
  void users_list(const std::string& roomname, const std::unordered_map<std::string, std::string> &users_map);

  /**
   * Updates the user count for a given chat in the GUI.
   *
   * @param chat_name The name of the chat (room).
   * @param count The number of active users.
   **/
  void update_count(const std::string& chat_name, int count);
  
  /**
   * Updates the online status of a specific user in the GUI.
   *
   * @param username The name of the user.
   * @param status The new status to be displayed.
   **/
  void update_status(const std::string& username, const std::string& status);

  /**
   * Schedules the removal of a user row from the chat UI.
   *
   * @param username The name of the user to remove from the chat list.
   **/
  void remove_user(const std::string& username);
  
  /**
   * Removes trailing whitespace (spaces and tabs) from a string.
   *
   * @param str The string to modify in-place.
   **/
  void trim(std::string& str);
};

