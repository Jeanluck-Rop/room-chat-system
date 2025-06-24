#include "client.hpp"

/* Returns the singleton instance of the Client class */
Client& Client::instance()
{
  static Client instance;
  return instance;
}

/* Constructor: Initializes the Client object with default values */
Client::Client() : socket_fd(-1), is_connected(false) {}

/* Destructor: Ensures proper cleanup by disconnecting if still connected */
Client::~Client()
{
  if (is_connected)
    disconnect();
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
  is_connected = true;
  return true;
}

/**
 * Starts the client application by initializing threads for listening and user actions.
 **/
void Client::run_client()
{
  TerminalView::print_info("Successfully connected to the server");
  TerminalView::print_info("You must identify yourself before accessing the chat with: /id=<username>");
  listener_thread = std::thread(&Client::receive_message, this);
  actions_thread = std::thread(&Client::user_actions, this);
  if (actions_thread.joinable())
    actions_thread.join();
  if (listener_thread.joinable())
    listener_thread.join();
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
  TerminalView::print_info("Disconnected from the server");
}

/**
 * Signal handler for handling interruptions like Ctrl+C.
 *
 * @param signal The signal number received.
 **/
void Client::signal_handler(int signal)
{
  if (signal == SIGINT) {
    TerminalView::print_info("\nInterruption received, disconnecting..");
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
    ssize_t received_bytes = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
    if (received_bytes > 0) {
      buffer[received_bytes] = '\0';
      std::string raw_message(buffer);
      handle_message(raw_message);
    } else if (received_bytes == 0) {
      TerminalView::print_server("Connection closed by the server.");
      disconnect();
      break;
    } else {
      if (errno != EINTR) {
        TerminalView::print_error("Error receiving data, disconnecting..");
        disconnect();
        break;
      }
    }
  }
}

/**
 * Processes a parsed Message object received from the server.
 *
 * @param incoming_msg a parsed Message object containing operation and result info.
 **/
void Client::handle_response(const Message& incoming_msg)
{
  std::string operation = incoming_msg.get_operation();
  std::string result = incoming_msg.get_result();
  std::string extra = incoming_msg.get_extra();

  if (operation == "IDENTIFY") {
    if (result == "SUCCESS") {
      TerminalView::print_server("* Welcome to the chat! *");
      TerminalView::print_info("Type --comms to see full chat commands");
    }
    else if (result == "USER_ALREADY_EXISTS") {
      TerminalView::print_invalid("Username [" + extra + "] already exists, disconnecting...");
      disconnect();
      return;
    }
  }

  if (operation == "TEXT")
    TerminalView::print_fail("User [" + extra + "] not found.");

  if (operation == "NEW_ROOM") {
    if (result == "SUCCESS")
      TerminalView::print_success("Room [" + extra + "] successfully created.");
    else if (result == "ROOM_ALREADY_EXISTS")
      TerminalView::print_fail("Room [" + extra + "] already exist");
  }

  if (operation == "INVITE") {
    if (result == "NO_SUCH_ROOM")
      TerminalView::print_fail("Room [" + extra + "] does not exist.");
    else if (result == "NO_SUCH_USER")
      TerminalView::print_fail("User [" + extra + "] not found.");
    else if (result == "NOT_JOINED")
      TerminalView::print_server("You have not been joined or invited to the room [" + extra + "].");
    else if (result == "SELF_INVITE")
      TerminalView::print_server("You can not invite yourself to a room.");
    else if (result == "ALREADY_MEMBER_OR_INVITED")
      TerminalView::print_server("User [" + extra + "] already invited or member.");
  }

  if (operation == "JOIN_ROOM") {
    if (result == "SUCCESS")
      TerminalView::print_success("Successfully joined to the room [" + extra + "].");
    else if (result == "NO_SUCH_ROOM")
      TerminalView::print_server("Room [" + extra + "] does not exist.");
    else if (result == "NOT_INVITED")
      TerminalView::print_server("You have not been invited to the room [" + extra + "].");
    else if (result == "ALREADY_MEMBER ")
      TerminalView::print_server("You have already joined the room [" + extra + "].");
  }

  if (operation == "ROOM_USERS" || operation == "ROOM_TEXT" || operation == "LEAVE_ROOM") {
    if (result == "NO_SUCH_ROOM")
      TerminalView::print_fail("Room [" + extra + "] does not exist.");
    else if (result == "NOT_JOINED")
      TerminalView::print_server("You have not been joined or invited to the room [" + extra + "].");
  }
  
  if (operation == "INVALID") {
    if (result == "NOT_IDENTIFIED") {
      TerminalView::print_invalid("Not identified, disconnecting...");
      disconnect();
      return;
    } else if (result == "INVALID") {
      TerminalView::print_invalid("Invalid operation, disconnecting...");
      disconnect_user();
      return;
    }
  }
}

/**
 * Parses and routes an incoming raw JSON-formatted message string received from the server.
 *
 * @param raw_message the raw JSON string received from the server.
 **/
void Client::handle_message(const std::string& raw_message)
{
  // Parse the incoming message
  Message incoming_msg;
  if (incoming_msg.parse(raw_message)) {
    // Handle different types of messages
    switch (incoming_msg.get_type()) {
    case Message::Type::RESPONSE:
      handle_response(incoming_msg);
      break;
    case Message::Type::NEW_USER:
      TerminalView::print_server(incoming_msg.get_username() + " joined the chat.");
      break;
    case Message::Type::NEW_STATUS:
      TerminalView::print_server(incoming_msg.get_username() + " changed status to " + incoming_msg.get_status() + ".");
      break;
    case Message::Type::TEXT_FROM:
      TerminalView::print_private(incoming_msg.get_username(), incoming_msg.get_text());
      break;
    case Message::Type::PUBLIC_TEXT_FROM:
      TerminalView::print_public(incoming_msg.get_username(), incoming_msg.get_text());
      break;
    case Message::Type::USER_LIST:
      TerminalView::print_use("\n[USERS LIST]:\n" + incoming_msg.get_users());
      break;
    case Message::Type::INVITATION:
      TerminalView::print_server(incoming_msg.get_username() + " invited you to the room [" + incoming_msg.get_roomname() + "].");
      break;
    case Message::Type::JOINED_ROOM:
      TerminalView::print_room("[" + incoming_msg.get_roomname() + "]: " + incoming_msg.get_username() + " joined the room.");
      break;
    case Message::Type::ROOM_USER_LIST:
      TerminalView::print_room("\n[" + incoming_msg.get_roomname()  + " USERS LIST]:\n" + incoming_msg.get_users());
      break;
    case Message::Type::ROOM_TEXT_FROM:
      TerminalView::print_room("[" + incoming_msg.get_roomname() + "] " + "(" + incoming_msg.get_username() + "): " + incoming_msg.get_text());
      break;
    case Message::Type::LEFT_ROOM:
      TerminalView::print_room("[" + incoming_msg.get_roomname()  + "]: " + incoming_msg.get_username() + " left the room.");
      break;
    case Message::Type::DISCONNECTED:
      TerminalView::print_server(incoming_msg.get_username() + " disconnected from the chat.");
      break;
    default:
      TerminalView::print_info("Unknown message type received");
    }
  }
}

/**
 * Handles user input and actions, including identification and chat interactions.
 **/
void Client::user_actions()
{
  bool identified = false;
  
  while (is_connected) {  
    std::string user_input = TerminalView::get_user_input();

    if (user_input.empty())
        continue;

    if (!identified) {
      if (!check_id(user_input)) {
	disconnect();
	return;
      }
      identified = true;
      continue;
    } 
    
    if (user_input.rfind("/status=", 0) == 0)
      change_status(user_input);
    else if (user_input.rfind("--users", 0) == 0) {
      Message users_list = Message::create_users_list_message();
      send_message(users_list.to_json());
    } else if (user_input.rfind("--dm", 0) == 0)
      direct_message(user_input);
    else if (user_input.rfind("/new_room=", 0) == 0)
      new_room(user_input);
    else if (user_input.rfind("/invite=", 0) == 0)
      invite_users(user_input);
    else if (user_input.rfind("/join_room=", 0) == 0)
      join_room(user_input);
    else if (user_input.rfind("/room_users=", 0) == 0)
      room_users(user_input);
    else if (user_input.rfind("/text_room=", 0) == 0)
      room_text(user_input);
    else if (user_input.rfind("/leave_room=", 0) == 0)
      leave_room(user_input);
    else if (user_input == "--exit") {
      disconnect_user();
      return;
    } else {    
      Message public_msg = Message::create_public_text_message(user_input);
      send_message(public_msg.to_json());
    }
  }
}

/**
 * Validates  the user-provided ID format and availability.
 *
 * @param user_input Command input from the user.
 * @return true if the ID is valid and sent, false otherwise.
 **/
bool Client::check_id(std::string& user_input)
{
  if (user_input.rfind("/id=", 0) != 0) {
    TerminalView::print_invalid("You must identify yourself before accessing the chat. Disconnecting...");
    return false;
  }
  std::string username = user_input.substr(4);
  if (username.length() > 8 || username.length() < 2) {
    TerminalView::print_invalid("Invalid username, disconnecting...");
    return false;
  }
  Message identify_msg = Message::create_identify_message(username);
  send_message(identify_msg.to_json());
  return true;
}

/**
 * Sends a request for change the client's status.
 *
 * @param user_input Command input from the user.
**/
void Client::change_status(std::string& user_input)
{
  std::string status = user_input.substr(8);
  if (status != "AWAY" || status != "ACTIVE" || status != "BUSY") {
    TerminalView::print_info("Invalid status. Options are: [AWAY], [ACTIVE], [BUSY]");
    return;
  }
  Message status_msg = Message::create_status_message(status);
  send_message(status_msg.to_json());
  TerminalView::print_success("Your status succesfully changed to [" + status + "].");
}

/**
 * Sends a private message to the target user.
 *
 * @param user_input Command input from the user.
 **/
void Client::direct_message(std::string& user_input)
{
  size_t message_detector = user_input.find(':');
  if (message_detector == std::string::npos) {
    TerminalView::print_info("Incorrect use of --dm");
    TerminalView::print_use("Use: --dm<username>:<message>");
    return;
  }
  std::string target_username = user_input.substr(4, message_detector - 4);
  std::string message_text = user_input.substr(message_detector + 1);
  Message private_msg = Message::create_private_text_message(target_username, message_text);
  send_message(private_msg.to_json());
}

/**
 * Sends a request for create a new chat room.
 *
 * @param user_input Command input from the user.
 **/
void Client::new_room(std::string& user_input)
{
  std::string roomname = user_input.substr(10);
  if (roomname.length() > 16 || roomname.length() < 3) {
    TerminalView::print_info("Invalid room name");
    return;
  }
  Message new_room_msg = Message::create_new_room_message(roomname);
  send_message(new_room_msg.to_json());
}

/**
 * Sends an invitation to multiple users to join a specific room.
 *
 * @param user_input Command input from the user.
 **/
void Client::invite_users(std::string& user_input)
{
  size_t roomname_detector = user_input.find(':');
  if (roomname_detector != std::string::npos) {
    std::string raw_usernames = user_input.substr(8, roomname_detector - 8);
    std::string target_roomname = user_input.substr(roomname_detector + 1);
    std::vector<std::string> usernames; //Vector to store the parsed usernames
    std::stringstream stringstream(raw_usernames); //stringstream with raw usernames string to enable splitting
    std::string username; //Temp string to hold each extracted username
    //Loop for extract usernames separated by ';'
    while (std::getline(stringstream, username, ';'))
      if (!username.empty()) usernames.push_back(username);
    Message invitation = Message::create_invite_message(target_roomname, usernames);
    send_message(invitation.to_json());
    TerminalView::print_success("Invitations sent.");
  } else {
    TerminalView::print_info("Incorrect use of /invite=");
    TerminalView::print_use("Use: /invite=<username_1>;<username_2>;<username_3>:<roomname>");
  }
}

/**
 * Sends a request to join a specific chat room.
 *
 * @param user_input Command input from user.
 **/
void Client::join_room(std::string& user_input)
{
  std::string roomname = user_input.substr(11);
  if (roomname.length() > 16 || roomname.length() < 3) {
    TerminalView::print_info("Invalid room name");
    return;
  }
  Message join_room_msg = Message::create_join_room_message(roomname);
  send_message(join_room_msg.to_json());
}

/**
 * Sends a request to lists currently users in a chat room.
 *
 * @param user_input Command input from the user.
 **/
void Client::room_users(std::string& user_input)
{
  std::string roomname = user_input.substr(12);
  if (roomname.length() > 16 || roomname.length() < 3) {
    TerminalView::print_info("Invalid room name");
    return;
  }
  Message room_users_msg = Message::create_room_users_message(roomname);
  send_message(room_users_msg.to_json());
}

/**
 * Sends a message to all users in the specific room.
 *
 * @param user_input Command input from the user.
 **/
void Client::room_text(std::string& user_input)
{
  size_t message_detector = user_input.find(':');
  if (message_detector == std::string::npos) {
    TerminalView::print_info("Incorrect use of /room_text=");
     TerminalView::print_use("Use: /room_text=<roomname>:<message>");
    return;
  }
  std::string target_roomname = user_input.substr(11, message_detector - 11);
  if (target_roomname.length() > 16 || target_roomname.length() < 3) {
    TerminalView::print_info("Invalid room name");
    return;
  }
  std::string message_text = user_input.substr(message_detector + 1);
  Message room_msg = Message::create_room_text_message(target_roomname, message_text);
  send_message(room_msg.to_json());
}

/**
 * Sends a request to leave a specific room.
 *
 * @param user_input Command input from the user.
 **/
void Client::leave_room(std::string& user_input)
{
  std::string roomname = user_input.substr(12);
  if (roomname.length() > 16 || roomname.length() < 3) {
    TerminalView::print_info("Invalid room name");
    return;
  }
  Message leave_room_msg = Message::create_leave_room_message(roomname);
  send_message(leave_room_msg.to_json());
}

/**
 * Sends a disconnect message to the server and closes the connection.
 **/
void Client::disconnect_user()
{
  if (!is_connected)
    return;
  Message disconnect_msg = Message::create_disconnect_message();
  send_message(disconnect_msg.to_json());
  disconnect();
}

/**
 * Sends a message to the server.
 *
 * @param message The message content to be sent.
 **/
void Client::send_message(const std::string& message)
{
  if (is_connected)
    if (send(socket_fd, message.c_str(), message.size(), 0) < 0)
      TerminalView::print_error("Failed to send message");
}
