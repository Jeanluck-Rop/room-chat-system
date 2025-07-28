#pragma once

#include <glib.h>
#include <csignal>
#include <iostream>
#include <arpa/inet.h>

#include "message.hpp"
#include "client.hpp"
#include "models.h"
#include "view.h"

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
  void try_connection(const std::string& server_ip, int port, const std::string& user_name);
  
  /**
   * Parses and routes an incoming raw JSON-formatted message string received from the server.
   *
   * @param raw_message the raw JSON string received from the server.
   **/
  void handle_message(const std::string& raw_message);
  
  /**
   * Validates  the user-provided ID format and availability.
   * Sends the identification request to the server.
   *
   * @param user_input Command input from the user.
   * @return true if the ID is valid and sent, false otherwise.
   **/
  bool check_id(std::string& user_input);

  /**
   * Sends a request for change the client's status.
   * Sends only valid statuses ("AWAY", "ACTIVE", or "BUSY").
   *
   * @param user_input Command input from the user.
   **/
  void change_status(std::string& user_input);

  /**
   * Sends a private message to the target user.
   *
   * @param user_input Command input from the user.
   **/
  void direct_message(std::string& user_input);

  /**
   * Sends a request for create a new chat room.
   * Room name must be between 3 and 16 characters.
   *
   * @param user_input Command input from the user.
   **/
  void new_room(std::string& user_input);

  /**
   * Sends an invitation to multiple users to join a specific room.
   *
   * @param user_input Command input from the user.
   **/
  void invite_users(std::string& user_input);
  
  /**
   * Sends a request to join a specific chat room.
   *
   * @param user_input Command input from user.
   **/
  void join_room(std::string& user_input);

  /**
    * Sends a request to lists currently users in a chat room.
   *
   * @param user_input Command input from the user.
   **/
  void room_users(std::string& user_input);

  /**
   * Sends a message to all users in the specific room.
   *
   * @param user_input Command input from the user.
   **/
  void room_text(std::string& user_input);

  /**
   * Sends a request to leave a specific room.
   *
   * @param user_input Command input from the user.
   **/
  void leave_room(std::string& user_input);
  
  /**
   * Sends a disconnect message to the server and closes the connection.
   **/
  void disconnect_user();
  
private:
  Controller();  //private constructor
  ~Controller();
  
  /**
   * Processes a parsed Message object received from the server.
   * Determines the operation type and reacts according it.
   *
   * @param incoming_msg a parsed Message object containing operation and result info.
   **/
  void handle_response(const Message& incoming_msg);

  /**
   * Removes trailing whitespace (spaces and tabs) from a string.
   *
   * @param str The string to modify in-place.
   **/
  void trim(std::string& str);
};

