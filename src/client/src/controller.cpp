#include "controller.hpp"

Controller& Controller::instance()
{
  static Controller instance;
  return instance;
}

Controller::Controller() {}
Controller::~Controller() {}

/**
 * Processes a parsed Message object received from the server.
 *
 * @param incoming_msg a parsed Message object containing operation and result info.
 **/
void Controller::handle_response(const Message& incoming_msg)
{
  std::string operation = incoming_msg.get_operation();
  std::string result = incoming_msg.get_result();
  std::string extra = incoming_msg.get_extra();
  std::string msg;
  
  if (operation == "IDENTIFY") {
    if (result == "SUCCESS")
      g_idle_add(enter_chat_idle, NULL);
    else if (result == "USER_ALREADY_EXISTS") {
      msg = "Username [" + extra + "] already exists.";
      init_alert_dialog(msg.c_str(), WARNING_DIALOG);
      Client::instance().disconnect();
      return;
    }
  }
  ///
///if (operation == "STATUS")
/// alert_dialog("Invalid new status.");
///
///if (operation == "PUBLIC_TEXT")
/// alert_dialog("Invalid public text content.");
///
///if (operation == "TEXT") {
/// if (result == "INVALID")
///  alert_dialog("Invalid username or private text content.");
/// else if (result == "NO_SUCH_USER")
///   alert_dialog("User [" + extra + "] not found.");
///}
///
///if (operation == "NEW_ROOM") {
/// if (result == "SUCCESS")
///  new_room(extra); /////
/// else if (result == "INVALID")
///  alert_dialog("Invalid room name.");
/// else if (result == "ROOM_ALREADY_EXISTS")
///  alert_dialog("Room [" + extra + "] already exist");
/// else if (result == "ERROR_JOINING")
///  alert_dialog("There was an error while joining the room [" + extra + "].");
/// else if (result == "ERROR_MARKING")
///  alert_dialog("There was an error marking the user [" + extra + "] as invited.");
///}
///
///if (operation == "INVITE") {
/// if (result == "INVALID")
///  alert_dialog("Invalid room name.");
/// else if (result == "NO_SUCH_ROOM")
///  alert_dialog("Room [" + extra + "] does not exist.");
/// else if (result == "NO_SUCH_USER")
///  alert_dialog("User [" + extra + "] not found.");
/// else if (result == "NOT_JOINED")
///  alert_dialog("You have not been joined or invited to the room [" + extra + "].");
/// else if (result == "SELF_INVITE")
///  alert_dialog("You can not invite yourself to a room.");
/// else if (result == "ALREADY_MEMBER_OR_INVITED")
///  alert_dialog("User [" + extra + "] already invited or member.");
/// else if (result == "ERROR_MARKING")
///  alert_dialog("There was an error marking the user [" + extra + "] as invited.");
///}
///
///if (operation == "JOIN_ROOM") {
/// if (result == "SUCCESS")
///  new_room(extra); /////
/// else if (result == "INVALID")
///  alert_dialog("Invalid room name.");
/// else if (result == "NO_SUCH_ROOM")
///  alert_dialog("Room [" + extra + "] does not exist.");
/// else if (result == "NOT_INVITED")
///  alert_dialog("You have not been invited to the room [" + extra + "].");
/// else if (result == "ALREADY_MEMBER")
///  alert_dialog("You have already joined the room [" + extra + "].");
/// else if (result == "ERROR_JOINING")
///  alert_dialog("There was an error while joining the room [" + extra + "].");
///}
///
///if (operation == "ROOM_USERS" || operation == "ROOM_TEXT" || operation == "LEAVE_ROOM") {
/// if (result == "INVALID")
///  alert_dialog("Invalid room name or text content.");
/// else if (result == "NO_SUCH_ROOM")
///  alert_dialog("Room [" + extra + "] does not exist.");
/// else if (result == "NOT_JOINED")
///  alert_dialog("You have not been joined or invited to the room [" + extra + "].");
///}
///
///if (operation == "INVALID") {
/// if (result == "NOT_IDENTIFIED") {
///  alert_dialog("Not identified");
///  disconnect();
///  return;
///} else if (result == "INVALID") {
///  alert_dialog("Invalid operation, disconnecting...");
///  disconnect_user();
///  return;
/// }
///}
///1
}


/**
 * Parses and routes an incoming raw JSON-formatted message string received from the server.
 *
 * @param raw_message the raw JSON string received from the server.
 **/
void Controller::handle_message(const std::string& raw_message)
{
  Message incoming_msg;
  if (incoming_msg.parse(raw_message)) {
    switch (incoming_msg.get_type()) {
      //std::string info = "Info";
      //std::string username = incoming_msg.get_username();
      //std::string roomname = incoming_msg.get_roomname();
      //std::string text = incoming_msg.get_text();
      //std::string status = incoming_msg.get_status();
      
    case Message::Type::RESPONSE:
      handle_response(incoming_msg);
      break;
///
///case Message::Type::NEW_USER:
///  std::string joined = username + " joined the chat";
///  std::string notify = username + " joined the chat + ".";
///
///  add_new_notify(notify, NULL, NORMAL_NOTIF);
///  message_received("Public Chat", info, joined, PUBLIC_CHAT, INFO_MESSAGE);
///  PROBABLY ADD MORE ACTIONS LIKE UPDATE CHAT USERS COUNT
///  break;
///
///case Message::Type::NEW_STATUS:
///  std::string changed = username + " changed status to " + status;
///
///  add_new_notify(changed, NULL, NORMAL_NOTIF);
///  message_received("Public Chat", info, changed, PUBLIC_CHAT, INFO_MESSAGE);
///  break;
///
///case Message::Type::TEXT_FROM:
///  message_received(username, username, text, USER_CHAT, NORMAL_MESSAGE);
///  break;
///
///case Message::Type::PUBLIC_TEXT_FROM:
///  message_received("Public Chat", username, text, PUBLIC_CHAT, NORMAL_MESSAGE);
///  break;
///
///case Message::Type::USER_LIST:
///  PROBABLY CHANGE TO RESPONSE INSTEAD OF TYPE, BUT WE COULD FIND A WAY TO IMPLEMENT THIS 
///  break;
///
///case Message::Type::INVITATION:
///  std::string invitation = username + " invited you to the room [" + roomname + "].";
///
///  add_new_notify(invitation, roomname, INVITE_NOTIF);
///  break;
///
///case Message::Type::JOINED_ROOM:
///  std::string joined = username + " joined the room"
///  std::string notify = username + " joined the room [" + roomname + "]."
///
///  add_new_notify(notify, roomname, NORMAL_NOTIF);
///  message_received(roomname, username, joined, ROOM_CHAT, INFO_MESSAGE);
///  break;
//
///case Message::Type::ROOM_USER_LIST:
///  PROBABLY CHANGE TO RESPONSE INSTEAD OF TYPE, BUT WE COULD FIND A WAY TO IMPLEMENT THIS 
///  break;
///
///case Message::Type::ROOM_TEXT_FROM:
///
///  message_received(roomname, username, text, ROOM_CHAT, NORMAL_MESSAGE);
///  break;
///
///case Message::Type::LEFT_ROOM:
///  std::string left = username + " left the room"
///  std::string notify = username + " left the room [" + roomname + "]."
///
///  add_new_notify(notify, roomname, NORMAL_NOTIF);
///  message_received(roomname, username, left, ROOM_CHAT, INFO_MESSAGE);
///  break;

///case Message::Type::DISCONNECTED:
///  std::string left = username + " disconnected from the chat";
///  std::string notify = username + "disconnected from the chat + ".";
///
///  add_new_notify(notify, NULL, NORMAL_NOTIF);
///  message_received("Public Chat", info, left, PUBLIC_CHAT, INFO_MESSAGE);
///  PROBABLY ADD MORE ACTIONS LIKE UPDATE CHAT USERS COUNT
///  break;
    default:
      break; ///HERE GOES THE FUTURE ALERT DIALOGS
    }
  }
}

/**
 * Validates  the user-provided ID format and availability.
 *
 * @param user_input Command input from the user.
 * @return true if the ID is valid and sent, false otherwise.
 **/
bool Controller::check_id(std::string& username)
{
  trim(username);
  if (username.length() > 8 || username.length() < 2)     return false;
  Message identify_msg = Message::create_identify_message(username);
  Client::instance().send_message(identify_msg.to_json());
  return true;
}

/**
 * Sends a request for change the client's status.
 *
 * @param user_input Command input from the user.
**/
void Controller::change_status(std::string& user_input)
{
  std::string status = user_input.substr(8);
  trim(status);
  if (status != "AWAY" || status != "ACTIVE" || status != "BUSY") {
    //TerminalView::print_info("Invalid status. Options are: [AWAY], [ACTIVE], [BUSY]");
    return;
  }
  Message status_msg = Message::create_status_message(status);
  Client::instance().send_message(status_msg.to_json());
  //TerminalView::print_success("Your status succesfully changed to [" + status + "].");
}

/**
 * Sends a private message to the target user.
 *
 * @param user_input Command input from the user.
 **/
void Controller::direct_message(std::string& user_input)
{
  size_t message_detector = user_input.find(':');
  if (message_detector == std::string::npos) {
    //TerminalView::print_info("Incorrect use of --dm");
    //TerminalView::print_use("Use: --dm <username> : <message>");
    return;
  }
  std::string target_username = user_input.substr(4, message_detector - 4);
  std::string message_text = user_input.substr(message_detector + 1);
  trim(target_username);
  Message private_msg = Message::create_private_text_message(target_username, message_text);
  Client::instance().send_message(private_msg.to_json());
}

/**
 * Sends a request for create a new chat room.
 *
 * @param user_input Command input from the user.
 **/
void Controller::new_room(std::string& user_input)
{
  std::string roomname = user_input.substr(5);
  trim(roomname);
  if (roomname.length() > 16 || roomname.length() < 3) {
    //TerminalView::print_info("Invalid room name");
    return;
  }
  Message new_room_msg = Message::create_new_room_message(roomname);
  Client::instance().send_message(new_room_msg.to_json());
}

/**
 * Sends an invitation to multiple users to join a specific room.
 *
 * @param user_input Command input from the user.
 **/
void Controller::invite_users(std::string& user_input)
{
  size_t roomname_detector = user_input.find(':');
  if (roomname_detector == std::string::npos) {
    //TerminalView::print_info("Incorrect use of --iv");
    //TerminalView::print_use("Use: --invite <username_1>;<username_2>;<username_3> : <roomname>");
    return;
  }
  std::string raw_usernames = user_input.substr(8, roomname_detector - 8);
  std::string target_roomname = user_input.substr(roomname_detector + 1);
  trim(target_roomname);
  std::vector<std::string> usernames;            //Vector to store the parsed usernames
  std::stringstream stringstream(raw_usernames); //stringstream with raw usernames string to enable splitting
  std::string username;                          //Temp string to hold each extracted username
  //Loop for extract usernames separated by ';'
  while (std::getline(stringstream, username, ';')) {
    trim(username);
    if (!username.empty())
      usernames.push_back(username);
  }
  Message invitation = Message::create_invite_message(target_roomname, usernames);
  Client::instance().send_message(invitation.to_json());
  //TerminalView::print_success("Invitation(s) sent.");
}

/**
 * Sends a request to join a specific chat room.
 *
 * @param user_input Command input from user.
 **/
void Controller::join_room(std::string& user_input)
{
  std::string roomname = user_input.substr(6);
  trim(roomname);
  if (roomname.length() > 16 || roomname.length() < 3) {
    //TerminalView::print_info("Invalid room name");
    return;
  }
  Message join_room_msg = Message::create_join_room_message(roomname);
  Client::instance().send_message(join_room_msg.to_json());
}

/**
 * Sends a request to lists currently users in a chat room.
 *
 * @param user_input Command input from the user.
 **/
void Controller::room_users(std::string& user_input)
{
  std::string roomname = user_input.substr(9);
  trim(roomname);
  if (roomname.length() > 16 || roomname.length() < 3) {
    //TerminalView::print_info("Invalid room name");
    return;
  }
  Message room_users_msg = Message::create_room_users_message(roomname);
  Client::instance().send_message(room_users_msg.to_json());
}

/**
 * Sends a message to all users in the specific room.
 *
 * @param user_input Command input from the user.
 **/
void Controller::room_text(std::string& user_input)
{
  size_t message_detector = user_input.find(':');
  if (message_detector == std::string::npos) {
    //TerminalView::print_info("Incorrect use of --room");
    //TerminalView::print_use("Use: --textr <roomname> : <message>");
    return;
  }
  std::string target_roomname = user_input.substr(7, message_detector - 7);
  trim(target_roomname);
  if (target_roomname.length() > 16 || target_roomname.length() < 3) {
    //TerminalView::print_info("Invalid room name");
    return;
  }
  std::string message_text = user_input.substr(message_detector + 1);
  Message room_msg = Message::create_room_text_message(target_roomname, message_text);
  Client::instance().send_message(room_msg.to_json());
}

/**
 * Sends a request to leave a specific room.
 *
 * @param user_input Command input from the user.
 **/
void Controller::leave_room(std::string& user_input)
{
  std::string roomname = user_input.substr(7);
  trim(roomname);
  if (roomname.length() > 16 || roomname.length() < 3) {
    //TerminalView::print_info("Invalid room name");
    return;
  }
  Message leave_room_msg = Message::create_leave_room_message(roomname);
  Client::instance().send_message(leave_room_msg.to_json());
}

/**
 * Removes leading and trailing whitespace (spaces and tabs) from a string.
 * 
 * @param str The string to be trimmed.
 **/
void Controller::trim(std::string& str)
{
  // Trim the beginning
  str.erase(str.begin(),
	    std::find_if(str.begin(),
			 str.end(),
			 [](unsigned char ch)
			 {
			   return !std::isspace(ch);
			 }));
  // Trim the end
  str.erase(std::find_if(
			 str.rbegin(),
			 str.rend(),
			 [](unsigned char ch)
			 {
			   return !std::isspace(ch);
			 }).base(), str.end());
}

/**
 * Sends a disconnect message to the server and closes the connection.
 **/
void Controller::disconnect_user()
{
  std::cout << "Disconnecting..." << std::endl;
  Message disconnect_msg = Message::create_disconnect_message();
  Client::instance().send_message(disconnect_msg.to_json());
  Client::instance().disconnect();
}

/**
 *
 **/
void Controller::try_connection(const std::string& server_ip, int port, const std::string& user_name)
{
  std::signal(SIGINT, Client::signal_handler);  
  try
    {
      auto& client = Client::instance();
      client.connect_to_server(server_ip, port);
      client.run_client();
      std::cout << "Todo salio bien." << std::endl;
    }
  catch (const std::runtime_error &e)
    {
      std::string err = e.what();
      std::cerr << "[ERROR]: " << err << std::endl;
      init_alert_dialog(err.c_str(), ERROR_DIALOG);
    }

  std::string msg = "Invalid user name.";
  std::string username = user_name;
  if (!check_id(username))
    init_alert_dialog(msg.c_str(), ERROR_DIALOG);
}
