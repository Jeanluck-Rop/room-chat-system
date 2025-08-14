#include "client.hpp"

/* Returns the singleton instance of the Client class */
Client& Client::instance()
{
  static Client instance;
  return instance;
}

/* Constructor: Initializes the Client object with default values */
Client::Client() : socket_fd(-1), is_connected(false), ssl_ctx(nullptr), ssl(nullptr)
{
  OPENSSL_init_ssl(0, NULL);
  ssl_ctx = SSL_CTX_new(TLS_client_method());
  if (!ssl_ctx) {
    ERR_print_errors_fp(stderr);
    throw std::runtime_error("Failed to create SSL_CTX");
  }
}

/* Destructor: Ensures proper cleanup by disconnecting if still connected and freeing remaining ssl pointers */
Client::~Client()
{
  if (is_connected)
    disconnect();
  if (ssl)
    SSL_free(ssl);
  if (ssl_ctx)
    SSL_CTX_free(ssl_ctx);
}

/**
 * Establishes a connection to the server at the specified IP and port.
 *
 * @param server_ip The IP address of the server to connect to.
 * @param port The port number of the server to connect to.
 * @return true if the connection is successful, otherwise throws an exception.
 * @throws std::runtime_error if socket creation, IP conversion, or connection fails.
 **/
bool Client::connect_to_server(const std::string& server_ip, int port)
{
  //Create a TCP socket
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1)
    throw std::runtime_error("Failed to create socket.");
  //Setup server address structure
  sockaddr_in server_address{};
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  //Convert server IP address to binary format
  if (inet_pton(AF_INET, server_ip.c_str(), &server_address.sin_addr) <= 0)
    throw std::runtime_error("Invalid server IP address.");
  if (connect(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
    throw std::runtime_error("Failed to connect to the server.");

  //Creat SSL struct
  ssl = SSL_new(ssl_ctx);
  if (!ssl) {
    ERR_print_errors_fp(stderr);
    throw std::runtime_error("Failed to create SSL object.");
  }
  SSL_set_fd(ssl, socket_fd);
  //Handshake SSL
  if (SSL_connect(ssl) <= 0) {
    ERR_print_errors_fp(stderr);
    throw std::runtime_error("SSL handshake failed.");
  }

  is_connected = true;
  return true;
}

/**
 * Starts the client application by initializing threads for listening and user actions.
 **/
void Client::run_client()
{
  listener_thread = std::thread(&Client::receive_message, this);
  listener_thread.detach();
}

/**
 * Sends a message to the server.
 *
 * @param message The message content to be sent.
 **/
void Client::send_message(const std::string& message)
{
  if (!is_connected)
    return;

  size_t total_sent = 0;
  size_t msg_len = message.size();
  while (total_sent < msg_len) {
    int sent = SSL_write(ssl, message.c_str() + total_sent, msg_len - total_sent);
    if (sent <= 0) {
      int err = SSL_get_error(ssl, sent);
      std::cerr << "SSL_write failed, error " << err << std::endl;
      ERR_print_errors_fp(stderr);
      disconnect();
      return;
    }
    total_sent += sent;
  }
}

/**
 * Signal handler for handling interruptions like Ctrl+C.
 *
 * @param signal The signal number received.
 **/
void Client::signal_handler(int signal)
{
  if (signal == SIGINT) {
    std::cout << "\nInterruption received, disconnecting." << std::endl;
    instance().disconnect();
    exit(EXIT_SUCCESS);
  }
}

/**
 * Listens and parses incoming messages from the server in a loop.
 **/
void Client::receive_message()
{
  char buffer[1024];
  while (is_connected) {
    int received_bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    if (received_bytes > 0) {
      buffer[received_bytes] = '\0';
      std::string raw_message(buffer);
      Controller::instance().handle_message(raw_message);
    } else if (received_bytes == 0) {
      std::cout << "Connection closed by the server." << std::endl; ////
      disconnect();
      break;
    } else {
      int err = SSL_get_error(ssl, received_bytes);
      if (err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE) {
	std::cerr << "SSL_read failed, error: " << err << std::endl; ////
	ERR_print_errors_fp(stderr);
        disconnect();
        break;
      }
    }
  }
}

/**
 * Disconnects the client gracefully from the server.
 **/
void Client::disconnect()
{
  static std::mutex disconnect_mutex;
  std::lock_guard<std::mutex> lock(disconnect_mutex); //Ensure only one thread disconnects
  if (!is_connected)
    return;
  is_connected = false;
  shutdown(socket_fd, SHUT_RDWR); //Close both socket sides
  if (socket_fd != -1)
    close(socket_fd);
  socket_fd = -1;
  Controller::instance().notify_disconnection();
  std::cout << "Disconnected from the server." << std::endl;
}
