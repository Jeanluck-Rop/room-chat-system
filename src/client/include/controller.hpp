#pragma once

#include <glib.h>
#include <csignal>
#include <iostream>
#include <arpa/inet.h>
#include <unordered_map>

#include "message.hpp"
#include "client.hpp"
#include "models.h"
#include "view.h"
#include "idle.h"

/**
 * @class Controller
 **/
class Controller
{
public:
  static Controller& instance(); // Singleton getter
  Controller(const Controller&) = delete;
  Controller& operator=(const Controller&) = delete;

  /* */
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
   * Sends a disconnect message to the server and closes the connection.
   **/
  void disconnect_user();
  
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
  
  /* */
  void send_dialog(const std::string& message, DialogType type);

  /* */
  void create_room(const std::string& room_name);

  /* */ 
  void new_notify(const std::string& message, const std::string& roomname, NotifyType type);

  /* */
  void send_message(const std::string& chat_name, const std::string& sender, const std::string& content, ChatType chat_type, MessageType msg_type);

  /* */
  void users_list(const std::string& roomname, const std::unordered_map<std::string, std::string> &users_map);
  
  /**
   * Removes trailing whitespace (spaces and tabs) from a string.
   *
   * @param str The string to modify in-place.
   **/
  void trim(std::string& str);
};

