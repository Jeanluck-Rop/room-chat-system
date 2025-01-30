#include <iostream>
#include <stdexcept>
#include <csignal>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "client.hpp"
#include "message.hpp"

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
    throw std::runtime_error("Failed to create socket");

  // Setup server address structure
  sockaddr_in server_address{};
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);

  // Convert server IP address to binary format
  if (inet_pton(AF_INET, server_ip.c_str(), &server_address.sin_addr) <= 0)
    throw std::runtime_error("Invalid server IP address");

  if (connect(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
    throw std::runtime_error("Failed to connect to the server");

  is_connected = true;
  return true;
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
    disconnnect();
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

      // Parse the incoming message
      Message incoming_msg;
      if (incoming_msg.parse(raw_message)) {
        // Handle different types of messages
        switch (incoming_msg.get_type()) {
	case Message::Type::RESPONSE: {
	  std::string operation = incoming_message.get_operation();
	  std::string result = incoming_message.get_result();
	  
	  if (operation == "IDENTIFY") {
	    if (result == "SUCCESS") {
	      TerminalView::display_message("[INFO] Identification successful. Welcome to the chat!");
	    } else if (result == "USER_ALREADY_EXISTS") {
	      TerminalView::display_message("[ALERT] Username already exists. Disconnecting...");
	      disconnect();
	      return;
	    }
	  } else if (operation == "INVALID") {
	    if (result == "NOT_IDENTIFIED") {
	      TerminalView::display_message("[ALERT] Not identified. Disconnecting...");
	      disconnect();
	      return;
	    } else if (result == "INVALID") {
	      TerminalView::display_message("[ALERT] Invalid operation. Disconnecting...");
	      disconnect();
	      return;
	    }
	  } else if (operation == "TEXT")
	    TerminalView::display_message("[ALERT] User '" + incoming_msg.get_extra() + "' not found .");
	  break;
	}
	case Message::Type::NEW_USER:
	  TerminalView::display_message("[INFO] " + incoming_msg.get_username() + " joined the chat.");
	  break;
	case Message::Type::NEW_STATUS:
	  TerminalView::display_message("[INFO] " + incoming_msg.get_username() + " changed status to" + incoming_msg.get_status() + ".");
	  break;
	case Message::Type::TEXT_FROM:
	  TerminalView::display_message("[PRIVATE] " + incoming_msg.get_text());
	  break;
	case Message::Type::PUBLIC_TEXT_FROM:
	  TerminalView::display_message("[PUBLIC] " + incoming_msg.get_text());
	  break;
	case Message::Type::DISCONNECTED:
	  TerminalView::display_message("[INFO] " + incoming_message.get_username() + "disconnected from the chat.");
	  break;
	default:
	  TerminalView::display_message("[ALERT] Unknown message type received.");
        }
      }
    } else if (bytes_received == 0) {
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
 * Handles user input and actions, including identification and chat interactions.
 */
void Client::handle_user_actions() {
  bool identified = false;
  
  while (is_connected) {  
    std::string user_input = TerminalView::get_user_input();
    
    if (user_input == "/exit=") {
      Message disconnect_msg = Message::create_disconnect_message();
      send_message(disonnect_msg.to_json());
      disconnect();
      break;
    }
	
    if (!identified) {
      if (user_input.rfind("/id=", 0) == 0) {
	std::string username = user_input.substr(4);
	if (username.length() > 8 || username.length() < 1) {
	  TerminalView::display_message("[ALERT] Invalid username. Disconnecting...");
	  Message disconnect_msg = Message::create_disconnect_message();
	  send_message(disonnect_msg.to_json());
	  disconnect();
	  break;
	}
	
	identified = true;
	// Send IDENTIFY message using Message class
	Message identify_msg = Message::create_identify_message(username);
	send_message(identify_msg.to_json());
      } else {
	TerminalView::display_message("[ALERT] You must identify yourself before accessing the chat. Disconnecting...");
	Message disconnect_msg = Message::create_disconnect_message();
	send_message(disonnect_msg.to_json());
	disconnect();
	break;
      }
    } else if (user_input.rfind("/stts=", 0) == 0) {
      std::string status = user_input.substr(6);
      if (status == "AWAY" || status == "ACTIVE" || status == "BUSY") {
	Message status_msg = Message::create_status_message(status);
	send_message(status_msg.to_json());
      } else
	TerminalView::display_message("[ALERT] Invalid status. Options are: AWAY, ACTIVE, or BUSY.");
    } else if (user_input.rfind("/dm=", 0) == 0) {
      size_t message_detector = user_input.find(':');
      if (message_detector != std::string::npos) {
	std::string target_username = user_input.substr(4, message_detector - 4);
	std::string message_text = user_input.substr(message_detector + 1);
	    
	Message private_msg = Message::create_private_text_message(target_username, message_text);
	send_message(private_msg.to_json());
      } else {
	TerminalView::display_message("[ALERT] Incorrect usage of /dm=<username>:<message>");
      }
    } else {
      Message public_msg = Message::create_public_text_message(user_input);
      send_message(public_msg.to_json());
    }
  }
}

/**
 * Starts the client application by initializing threads for listening and user actions.
 * Ensures threads are properly joined before exiting.
 */
void Client::run_client() {
  TerminalView::display_message("[INFO] Successfully connected to the server.");
  TerminalView::display_message("[ALERT] You must identify yourself beore accessing the chat: /id=<username>");
  TerminalView::display_message("[INFO] Type '/exit=' to quit.");
  
  listener_thread = std::thread(&Client::receive_message, this);
  actions_thread = std::thread(&Client::handle_user_actions, this);
  
  if (listener_thread.joinable())
    listener_thread.join();
  if (actions_thread.joinable())
    actions_thread.join();
}
