#pragma once

#include <mutex>
#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <vector>
#include <csignal>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <nlohmann/json.hpp>

#include "message.hpp"
#include "view.hpp"

/**
 * @class Client
 *
 * Singleton class that manages the client-side operations of a chat application.
 * Responsible for establishing connection with the server, sending/receiving messages,
 * handling user inputs, maintaining connection status, and orchestrating chat-related actions.
 **/
class Client
{
public:
  /**
   * Returns the singleton instance of the Client class.
   * Ensures that only one instance of the client exists during the lifetime of the application.
   **/
  static Client& instance();                 //Singleton getter
  
  Client(const Client&) = delete;            //Deleted copy constructor
  Client& operator=(const Client&) = delete; //Deleted copy assignment operator

  /**
   * Establishes a connection to the server at the specified IP and port.
   *
   * @param server_ip The IP address of the server to connect to.
   * @param port The port number of the server to connect to.
   * @return true if the connection is successful, otherwise throws an exception.
   * @throws std::runtime_error if socket creation, IP conversion, or connection fails.
   **/
  bool connect_to_server(const std::string& server_ip, int port);

  /**
   * Starts the client application by initializing threads for listening and user actions.
   * Ensures threads are properly joined before exiting.
   **/
  void run_client();

  /**
   * Disconnects the client gracefully from the server.
   **/
  void disconnect();

  /**
   * Signal handler for handling interruptions like Ctrl+C.
   *
   * @param signal The signal number received.
   **/
  static void signal_handler(int signal);
  
private:
  /* Constructor: Initializes the Client object with default values */
  Client();
  /* Destructor: Ensures proper cleanup by disconnecting if still connected */
  ~Client();
  
  int socket_fd;                    //File descriptor for the socket connection
  std::thread actions_thread;       //Thread for handling user input and actions
  std::thread listener_thread;      //Thread for receiving messages from server
  std::atomic<bool> is_connected;   //Flag indicating the connection status

  /**
   * Listens and parses incoming messages from the server in a loop.
   * Processes and parses received messages using the Message class that uses a json protocol.
   **/
  void receive_message();

  /**
   * Processes a parsed Message object received from the server.
   * Determines the operation type and reacts according it.
   *
   * @param incoming_msg a parsed Message object containing operation and result info.
   **/
  void handle_response(const Message& incoming_msg);

  /**
   * Parses and routes an incoming raw JSON-formatted message string received from the server.
   *
   * @param raw_message the raw JSON string received from the server.
   **/
  void handle_message(const std::string& raw_message);

  /**
   * Handles user input and actions, including identification and chat interactions.
   **/
  void user_actions();

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
  
  /**
   * Sends a message to the server.
   *
   * @param message The message content to be sent.
   **/
  void send_message(const std::string& message);
};
