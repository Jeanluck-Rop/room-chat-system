#include <iostream>
#include <stdexcept>
#include <csignal>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "client.hpp"
#include "message.hpp"
#include "view.hpp"

/**
 * Returns the singleton instance of the Client class.
 * Ensures that only one instance of the client exists during the lifetime of the application.
 */
Client& Client::instance() {
  static Client instance;
  return instance;
}

/* Constructor: Initializes the Client object with default values */
Client::Client() : socket_fd(-1), is_connected(false) {}

/* Destructor: Ensures proper cleanup by disconnecting if still connected */
Client::~Client() {
  if (is_connected)
    disconnect();
}

/**
 * Establishes a connection to the specified server.
 * @param server_ip The IP address of the server to connect to.
 * @param port The port number of the server to connect to.
 * @return true if the connection is successful, otherwise throws an exception.
 * @throws std::runtime_error if socket creation, IP conversion, or connection fails.
 */
bool Client::connect_to_server(const std::string& server_ip, int port) {
  // Create a TCP socket
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1)
    throw std::runtime_error("Failed to create socket.");

  // Setup server address structure
  sockaddr_in server_address{};
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);

  // Convert server IP address to binary format
  if (inet_pton(AF_INET, server_ip.c_str(), &server_address.sin_addr) <= 0)
    throw std::runtime_error("Invalid server IP address.");

  if (connect(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
    throw std::runtime_error("Failed to connect to the server.");

  is_connected = true;
  return true;
}

/**
 * Starts the client application by initializing threads for listening and user actions.
 * Ensures threads are properly joined before exiting.
 */
void Client::run_client() {
  TerminalView::display_message("[INFO] Successfully connected to the server.");
  TerminalView::display_message("[ALERT] You must identify yourself before accessing the chat: /id=<username>");
  TerminalView::display_message("[INFO] Type '/exit=' to quit.");
  
  listener_thread = std::thread(&Client::receive_message, this);
  actions_thread = std::thread(&Client::handle_user_actions, this);
  
  if (listener_thread.joinable())
    listener_thread.join();
  if (actions_thread.joinable())
    actions_thread.join();
}

/**
 * Disconnects from the server, closing the socket and updating the state.
 */
void Client::disconnect() {
  if (is_connected) {
    is_connected = false;
    close(socket_fd);
    TerminalView::display_message("[INFO] Disconnected from the server.");
  }
}

/**
 * Signal handler for handling interruptions like Ctrl+C.
 * @param signal The signal number received.
 */
void Client::signal_handler(int signal) {
  if (signal == SIGINT) {
    TerminalView::display_message("\n[INFO] Interruption received. Disconnecting...");
    instance().disconnect();
    exit(EXIT_SUCCESS);
  }
}

/**
 * Listens for messages from the server in a loop.
 * Processes and parses received messages using the Message class that uses a json protocol.
 */
void Client::receive_message() {
  char buffer[1024];
  
  while (is_connected) {
    ssize_t received_bytes = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
    if (received_bytes > 0) {
      buffer[received_bytes] = '\0';
      std::string raw_message(buffer);
      handle_message_received(raw_message);
    } else if (received_bytes == 0) {
      TerminalView::display_message("[ALERT] connection closed by the served");
      disconnect();
    }
  }
}

/**
 * Sends a message to the server.
 * @param message The message content to be sent.
 */
void Client::send_message(const std::string& message) {
  if (is_connected)
    if (send(socket_fd, message.c_str(), message.size(), 0) < 0)
      TerminalView::display_error("[ERROR] Failed to send message.");
}

/**
 * Handles a response message received from the server.
 * Determines the operation type and reacts accordingcit.
 * @param incoming_msg a parsed Message object containing operation and result info.
 */
void Client::handle_response(const Message& incoming_msg) {
  std::string operation = incoming_msg.get_operation();
  std::string result = incoming_msg.get_result();
  if (operation == "IDENTIFY") {
    if (result == "SUCCESS")
      TerminalView::display_message("[INFO] Identification successful. Welcome to the chat!");
    else if (result == "USER_ALREADY_EXISTS") {
      TerminalView::display_message("[ALERT] Username already exists. Disconnecting...");
      disconnect();
      return;
    }
  }

  if (operation == "TEXT")
    TerminalView::display_message("[INFO] User '" + incoming_msg.get_extra() + "' not found.");

  if (operation == "NEW_ROOM") {
    if (result == "SUCCESS")
      TerminalView::display_message("[INFO] Room '" + incoming_msg.get_extra() + " successfully created.");
    else if (result == "ROOM_ALREADY_EXISTS")
      TerminalView::display_message("[INFO] Room '" + incoming_msg.get_extra() + "already exist.");
  }

  if (operation == "INVITE") {
    if (result == "NO_SUCH_ROOM")
      TerminalView::display_message("[INFO] Room '" + incoming_msg.get_extra() + " does not exist.");
    else if (result == "NO_SUCH_USER")
      TerminalView::display_message("[INFO] User '" + incoming_msg.get_extra() + "' not found.");
  }

  if (operation == "JOIN_ROOM") {
    if (result == "SUCCESS")
      TerminalView::display_message("[INFO] Successfully joined to the room: " + incoming_msg.get_extra() + ".");
    else if (result == "NO_SUCH_ROOM")
      TerminalView::display_message("[INFO] Room '" + incoming_msg.get_extra() + " does not exist.");
    else if (result == "NOT_INVITED")
      TerminalView::display_message("[INFO] You have not been invited to the room: " + incoming_msg.get_extra() + ".");
  }

  if (operation == "ROOM_USERS" ||
      operation == "ROOM_TEXT" ||
      operation == "LEAVE_ROOM") {
    if (result == "NO_SUCH_ROOM")
      TerminalView::display_message("[INFO] Room '" + incoming_msg.get_extra() + " does not exist.");
    else if (result == "NOT_JOINED")
      TerminalView::display_message("[INFO] You have not been joined or invited to the room: " + incoming_msg.get_extra() + ".");
  }
  
  if (operation == "INVALID") {
    if (result == "NOT_IDENTIFIED") {
      TerminalView::display_message("[ALERT] Not identified. Disconnecting...");
      disconnect();
      return;
    } else if (result == "INVALID") {
      TerminalView::display_message("[ALERT] Invalid operation. Disconnecting...");
      disconnect();
      return;
    }
  }
}

/**
 * Parses a raw JSON-formatted message string received from the server
 * and delegates handling based on its message type.
 * @param raw_message the raw JSON string received from the server.
 */
void Client::handle_message_received(const std::string& raw_message) {
  // Parse the incoming message
  Message incoming_msg;
  if (incoming_msg.parse(raw_message)) {
    // Handle different types of messages
    switch (incoming_msg.get_type()) {
    case Message::Type::RESPONSE:
      handle_response(incoming_msg);
      break;
    case Message::Type::NEW_USER:
      TerminalView::display_message("[INFO] " + incoming_msg.get_username() + " joined the chat.");
      break;
    case Message::Type::NEW_STATUS:
      TerminalView::display_message("[INFO] " + incoming_msg.get_username() + " changed status to" + incoming_msg.get_status() + ".");
      break;
    case Message::Type::TEXT_FROM:
      TerminalView::display_message("[PRIVATE (" + incoming_msg.get_username() + ")] " + incoming_msg.get_text());
      break;
    case Message::Type::PUBLIC_TEXT_FROM:
      TerminalView::display_message("[PUBLIC (" + incoming_msg.get_username() + ")] " + incoming_msg.get_text());
      break;
    case Message::Type::USER_LIST:
      TerminalView::display_message("[USERS LIST]" + incoming_msg.get_users());
      break;
    case Message::Type::INVITATION:
      TerminalView::display_message("[INFO] " + incoming_msg.get_username() + " invited you to the room: " + incoming_msg.get_roomname() + ".");
      break;
    case Message::Type::JOINED_ROOM:
      TerminalView::display_message("[" + incoming_msg.get_roomname()  + "] " + incoming_msg.get_username() + " joined the room.");
      break;
    case Message::Type::ROOM_USER_LIST:
      TerminalView::display_message("[" + incoming_msg.get_roomname()  + " USERS] " + incoming_msg.get_users());
      break;
    case Message::Type::ROOM_TEXT_FROM:
      TerminalView::display_message("[" + incoming_msg.get_roomname()  + " (" + incoming_msg.get_username() + ")] " + incoming_msg.get_text());
      break;
    case Message::Type::LEFT_ROOM:
      TerminalView::display_message("[" + incoming_msg.get_roomname()  + "] " + incoming_msg.get_username() + " left the room.");
      break;
    case Message::Type::DISCONNECTED:
      TerminalView::display_message("[INFO] " + incoming_msg.get_username() + " disconnected from the chat.");
      break;
    default:
      TerminalView::display_message("[ALERT] Unknown message type received.");
    }
  }
}

/**
 *
 */
bool Client::identify_user(std::string& user_input) {
  if (user_input.rfind("/id=", 0) == 0) {
    std::string username = user_input.substr(4);
    if (username.length() > 8 || username.length() < 1) {
      TerminalView::display_message("[ALERT] Invalid username. Disconnecting...");
      return false;
    }
    Message identify_msg = Message::create_identify_message(username);
    send_message(identify_msg.to_json());
    return true;
  }
  TerminalView::display_message("[ALERT] You must identify yourself before accessing the chat. Disconnecting...");
  return false;
}

/**
 *
 */
void Client::disconnect_user() {
  Message disconnect_msg = Message::create_disconnect_message();
  send_message(disconnect_msg.to_json());
  disconnect();
  return;
}

/**
 * Handles user input and actions, including identification and chat interactions.
 */
void Client::handle_user_actions() {
  bool identified = false;
  
  while (is_connected) {  
    std::string user_input = TerminalView::get_user_input();  
    if (!identified) {
      if (!(identify_user(user_input))) {
	disconnect_user();
	break;
      }
      identified = true;
    }
    
    if (user_input == "/exit=") {
      disconnect_user();
      break;
    }
    
    if (user_input.rfind("/stts=", 0) == 0) {
      std::string status = user_input.substr(6);
      if (status == "AWAY" || status == "ACTIVE" || status == "BUSY") {
	Message status_msg = Message::create_status_message(status);
	send_message(status_msg.to_json());
      } else
	TerminalView::display_message("[ALERT] Invalid status. Options are: AWAY, ACTIVE, or BUSY.");
    }

    if (user_input.rfind("/users=", 0) == 0) {
      Message users_list = Message::create_users_list_message();
      send_message(users_list.to_json());
    }
    
    if (user_input.rfind("/dm=", 0) == 0) {
      size_t message_detector = user_input.find(':');
      if (message_detector != std::string::npos) {
	std::string target_username = user_input.substr(4, message_detector - 4);
	std::string message_text = user_input.substr(message_detector + 1);
	Message private_msg = Message::create_private_text_message(target_username, message_text);
	send_message(private_msg.to_json());
      } else
	TerminalView::display_message("[ALERT] Incorrect usage of /dm=<username>:<message>");
    }

    Message public_msg = Message::create_public_text_message(user_input);
    send_message(public_msg.to_json());
  }
}
