#pragma once

#include <mutex>
#include <thread>
#include <atomic>
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "controller.hpp"

/**
 * @class Client
 *
 * Class that manages the client-side operations of a chat application.
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
   * Sends a message to the server.
   *
   * @param message The message content to be sent.
   **/
  void send_message(const std::string& message);

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
  
  /* File descriptor for the socket connection */
  int socket_fd;
  /* Thread for receiving messages from server */
  std::thread listener_thread;
  /* Flag indicating the connection status */
  std::atomic<bool> is_connected;
  /* SSL context pointer */
  SSL_CTX* ssl_ctx;
  /* SSL pointer struct for encapsulate SSL/TLS session asociated to the socket_fd */
  SSL* ssl;

  /**
   * Listens and parses incoming messages from the server in a loop.
   * Processes and parses received messages using the Message class that uses a json protocol.
   **/
  void receive_message();
};
