#include "controller.hpp"

Controller& Controller::instance()
{
  static Controller instance;
  return instance;
}

Controller::Controller() {}
Controller::~Controller() {}

/* */
void Controller::try_connection(int port,
				const std::string& server_ip,
				const std::string& user_name)
{
  std::signal(SIGINT, Client::signal_handler);  
  try
    {
      auto& client = Client::instance();
      client.connect_to_server(server_ip, port);
      client.run_client();
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

/**
 * Parses and routes an incoming raw JSON-formatted message string received from the server.
 *
 * @param raw_message the raw JSON string received from the server.
 **/
void Controller::handle_message(const std::string& raw_message)
{
  Message incoming_msg;
  if (!incoming_msg.parse(raw_message))
    return;

  std::string info = "Info";
  std::string username = incoming_msg.get_username();
  std::string roomname = incoming_msg.get_roomname();
  std::string text = incoming_msg.get_text();
   
  switch (incoming_msg.get_type()) {
  case Message::Type::RESPONSE:
    handle_response(incoming_msg);
    break;
  case Message::Type::NEW_USER:
    new_notify("[" + username + "] joined the chat" + ".", "", NORMAL_NOTIF);
    send_message("Public Chat", info, "[" + username + "] joined the chat", PUBLIC_CHAT, INFO_MESSAGE);
    ///
    break;
  case Message::Type::NEW_STATUS:
    new_notify("[" + username + "] changed status to " + incoming_msg.get_status() + ".", "", NORMAL_NOTIF);
    send_message("Public Chat", info, "[" + username + "] changed status to " + incoming_msg.get_status() + ".", PUBLIC_CHAT, INFO_MESSAGE);
    ///
    break;
  case Message::Type::TEXT_FROM:
    send_message(username, username, text, USER_CHAT, NORMAL_MESSAGE);
    break;
  case Message::Type::PUBLIC_TEXT_FROM:
    send_message("Public Chat", username, text, PUBLIC_CHAT, NORMAL_MESSAGE);
    break;
  case Message::Type::USER_LIST:
    users_list("", incoming_msg.get_users());
    break;
  case Message::Type::INVITATION:
    new_notify("[" + username + "] invited you to the room [" + roomname + "].", roomname, INVITE_NOTIF);
    break;
  case Message::Type::JOINED_ROOM:
    new_notify("[" + username + "] joined the room [" + roomname + "].", roomname, NORMAL_NOTIF);
    send_message(roomname, username, "[" + username + "] joined the room", ROOM_CHAT, INFO_MESSAGE);
    break;
  case Message::Type::ROOM_USER_LIST:
    users_list(roomname, incoming_msg.get_users());
    break;
  case Message::Type::ROOM_TEXT_FROM:
    send_message(roomname, username, text, ROOM_CHAT, NORMAL_MESSAGE);
    break;
  case Message::Type::LEFT_ROOM:
    new_notify("[" + username + "] left the room [" + roomname + "].", "", NORMAL_NOTIF);
    send_message(roomname, username, "[" + username + "] left the room", ROOM_CHAT, INFO_MESSAGE);
    ///
    break;
  case Message::Type::DISCONNECTED:
    new_notify("[" + username + "] disconnected from the chat.", "", NORMAL_NOTIF);
    send_message("Public Chat", info, "[" + username + "] disconnected from the chat", PUBLIC_CHAT, INFO_MESSAGE);
    ///
    break;
  default:
    send_dialog("Unknown message type received", WARNING_DIALOG);
    break;
  }
}

/**
 * Validates the provided username format and availability.
 *
 * @param username Username to validate.
 * @return true if the username is valid, false otherwise.
 **/
bool Controller::check_id(std::string& username)
{
  trim(username);
  if (username.length() > 8 || username.length() < 2)
    return false;
  Message identify_msg = Message::create_identify_message(username);
  Client::instance().send_message(identify_msg.to_json());
  return true;
}

/**
 * Sends a message to the public chat.
 *
 * @param message_content The message content to be sent.
 **/
void Controller::public_message(std::string& message_content)
{
  trim(message_content);
  Message public_msg = Message::create_public_text_message(message_content);
  Client::instance().send_message(public_msg.to_json());
}

/**
 * Sends a private message to the target user, the recipient.
 *
 * @param message_content The message content to be sent.
 * @param recipient The recipient user.
 **/
void Controller::direct_message(std::string& message_content,
				std::string& recipient)
{
  trim(message_content);
  trim(recipient);
  Message private_msg = Message::create_private_text_message(recipient, message_content);
  Client::instance().send_message(private_msg.to_json());
}

/**
 * Sends a request for change the client's status.
 *
 * @param status The new status.
 **/
void Controller::change_status(std::string& status)
{
  trim(status);
  if (status != "AWAY" && status != "ACTIVE" && status != "BUSY") {
    send_dialog("Invalid status selected.", WARNING_DIALOG);
    return;
  }
  Message status_msg = Message::create_status_message(status);
  Client::instance().send_message(status_msg.to_json());
  send_dialog("Your status succesfully changed to [" + status + "].", SUCCESS_DIALOG);
}

/**
 * Sends a request to lists currently users in the chat.
 **/
void Controller::chat_users()
{
  Message users_list = Message::create_users_list_message();
  Client::instance().send_message(users_list.to_json());
}

/**
 * Sends a request for create a new chat room.
 *
 * @param roomname The new room name.
 **/
void Controller::new_room(std::string& roomname)
{
  trim(roomname);
  if (roomname.length() > 16 || roomname.length() < 3) {
    send_dialog("Invalid room name.", WARNING_DIALOG);
    return;
  }
  Message new_room_msg = Message::create_new_room_message(roomname);
  Client::instance().send_message(new_room_msg.to_json());
}

/**
 * Sends an invitation to multiple users to join a specific room.
 *
 * @param guest The users list to invite.
 * @param roomname The room to invite the users.
 **/
void Controller::invite_users(char** guests, std::string& roomname)
{
  trim(roomname);
  std::vector<std::string> guests_list;
  for (int i = 0; guests[i] != NULL; i++)
    guests_list.emplace_back(guests[i]);
  Message invitation = Message::create_invite_message(roomname, guests_list);
  Client::instance().send_message(invitation.to_json());
  send_dialog("Invitations sent.", SUCCESS_DIALOG);
}

/**
 * Sends a request to join a specific chat room.
 *
 * @param roomname The room name to join.
 **/
void Controller::join_room(std::string& roomname)
{
  trim(roomname);
  if (roomname.length() > 16 || roomname.length() < 3) {
    send_dialog("Invalid room name.", WARNING_DIALOG);
    return;
  }
  Message join_room_msg = Message::create_join_room_message(roomname);
  Client::instance().send_message(join_room_msg.to_json());
}

/**
 * Sends a request to lists currently users in a chat room.
 *
 * @param roomname The room to request the users.
 **/
void Controller::room_users(std::string& roomname)
{
  trim(roomname);
  if (roomname.length() > 16 || roomname.length() < 3) {
    send_dialog("Invalid room name.", WARNING_DIALOG);
    return;
  }
  Message room_users_msg = Message::create_room_users_message(roomname);
  Client::instance().send_message(room_users_msg.to_json());
}

/**
 * Sends a message to all users in the specific room.
 *
 * @param message_content The message to send.
 * @param roomname The room to send the message.
 **/
void Controller::room_message(std::string& message_content,
			      std::string& roomname)
{
  trim(roomname);
  if (roomname.length() > 16 || roomname.length() < 3) {
    send_dialog("Invalid room name.", WARNING_DIALOG);
    return;
  }
  trim(message_content);
  Message room_msg = Message::create_room_text_message(roomname, message_content);
  Client::instance().send_message(room_msg.to_json());
}

/**
 * Sends a request to leave a specific room.
 *
 * @param roomname The room name to leave.
 **/
void Controller::leave_room(std::string& roomname)
{
  trim(roomname);
  if (roomname.length() > 16 || roomname.length() < 3) {
    send_dialog("Invalid room name.", WARNING_DIALOG);
    return;
  }
  Message leave_room_msg = Message::create_leave_room_message(roomname);
  Client::instance().send_message(leave_room_msg.to_json());
}

/**
 * Sends a disconnect message to the server and closes the connection.
 **/
void Controller::disconnect_user()
{
  Message disconnect_msg = Message::create_disconnect_message();
  Client::instance().send_message(disconnect_msg.to_json());
  Client::instance().disconnect();
}

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
  
  if (operation == "IDENTIFY") {
    if (result == "SUCCESS")
      g_idle_add(enter_chat_idle, NULL);
    else if (result == "USER_ALREADY_EXISTS") {
      std::string msg = "Username [" + extra + "] already exists.";
      DialogIdle *data = g_new(DialogIdle, 1);
      data->detail = g_strdup(msg.c_str());
      data->type = WARNING_DIALOG;
      g_idle_add(init_alert_dialog_idle, data);
      Client::instance().disconnect();
      return;
    }
  }
  
  if (operation == "STATUS")
    send_dialog("Invalid new status.", WARNING_DIALOG);
  if (operation == "PUBLIC_TEXT")
    send_dialog("Invalid public text content.", WARNING_DIALOG);
  
  if (operation == "TEXT") {
    if (result == "INVALID")
      send_dialog("Invalid username or private text content.", WARNING_DIALOG);
    else if (result == "NO_SUCH_USER")
      send_dialog("User [" + extra + "] not found.", WARNING_DIALOG);
  }

  if (operation == "NEW_ROOM") {
    if (result == "SUCCESS")
      create_room(extra);
    else if (result == "INVALID")
      send_dialog("Invalid room name.", WARNING_DIALOG);
    else if (result == "ROOM_ALREADY_EXISTS")
      send_dialog("Room [" + extra + "] already exist", WARNING_DIALOG);
    else if (result == "ERROR_JOINING")
      send_dialog("There was an error while joining the room [" + extra + "].", ERROR_DIALOG);
    else if (result == "ERROR_MARKING")
      send_dialog("There was an error marking the user [" + extra + "] as invited.", ERROR_DIALOG);
  }

  if (operation == "INVITE") {
    if (result == "INVALID")
      send_dialog("Invalid room name.", WARNING_DIALOG);
    else if (result == "NO_SUCH_ROOM")
      send_dialog("Room [" + extra + "] does not exist.", WARNING_DIALOG);
    else if (result == "NO_SUCH_USER")
      send_dialog("User [" + extra + "] not found.", WARNING_DIALOG);
    else if (result == "NOT_JOINED")
      send_dialog("You have not been joined or invited to the room [" + extra + "].", WARNING_DIALOG);
    else if (result == "SELF_INVITE")
      send_dialog("You can not invite yourself to a room.", WARNING_DIALOG);
    else if (result == "ALREADY_MEMBER_OR_INVITED")
      send_dialog("User [" + extra + "] already invited or member.", WARNING_DIALOG);
    else if (result == "ERROR_MARKING")
      send_dialog("There was an error marking the user [" + extra + "] as invited.", ERROR_DIALOG);
  }

  if (operation == "JOIN_ROOM") {
    if (result == "SUCCESS")
      create_room(extra);
    else if (result == "INVALID")
      send_dialog("Invalid room name.", WARNING_DIALOG);
    else if (result == "NO_SUCH_ROOM")
      send_dialog("Room [" + extra + "] does not exist.", WARNING_DIALOG);
    else if (result == "NOT_INVITED")
      send_dialog("You have not been invited to the room [" + extra + "].", WARNING_DIALOG);
    else if (result == "ALREADY_MEMBER")
      send_dialog("You have already joined the room [" + extra + "].", WARNING_DIALOG);
    else if (result == "ERROR_JOINING")
      send_dialog("There was an error while joining the room [" + extra + "].", ERROR_DIALOG);
  }

  if (operation == "ROOM_USERS" || operation == "ROOM_TEXT" || operation == "LEAVE_ROOM") {
    if (result == "INVALID")
      send_dialog("Invalid room name or text content.", WARNING_DIALOG);
    else if (result == "NO_SUCH_ROOM")
      send_dialog("Room [" + extra + "] does not exist.", WARNING_DIALOG);
    else if (result == "NOT_JOINED")
      send_dialog("You have not been joined or invited to the room [" + extra + "].", WARNING_DIALOG);
  }
  
  if (operation == "INVALID") {
    if (result == "NOT_IDENTIFIED") {
      send_dialog("Not identified.", WARNING_DIALOG);
      Client::instance().disconnect();
      return;
    } else if (result == "INVALID") {
      send_dialog("Invalid operation, disconnecting from the server", WARNING_DIALOG);
      disconnect_user();
      return;
    }
  }
}

/* */
void Controller::send_dialog(const std::string& message,
			     DialogType type)
{
  DialogIdle *data = g_new(DialogIdle, 1);
  data->detail = g_strdup(message.c_str());
  data->type = type;
  g_idle_add(alert_dialog_idle, data);
}

/* */
void
Controller::create_room(const std::string& room_name)
{
  MessageIdle *data = g_new0(MessageIdle, 1);
  data->chat_name = g_strdup(room_name.c_str());
  data->chat_type = ROOM_CHAT;
  data->msg_type = NORMAL_MESSAGE;
  g_idle_add(message_received_idle, data);
}

/* */
void
Controller::new_notify(const std::string& message,
		       const std::string& roomname,
		       NotifyType type)
{
  NotifyIdle *data = g_new0(NotifyIdle, 1);
  data->msg = g_strdup(message.c_str());
  if (roomname != "")
    data->room_name = strdup(roomname.c_str());
  data->type = type;
  g_idle_add(new_notify_idle, data);
}

/* */
void Controller::send_message(const std::string& chat_name,
			      const std::string& sender,
			      const std::string& content,
			      ChatType chat_type,
			      MessageType msg_type)
{
  MessageIdle *data = g_new0(MessageIdle, 1);
  data->chat_name = g_strdup(chat_name.c_str());
  data->sender = g_strdup(sender.c_str());
  data->content = g_strdup(content.c_str());
  data->chat_type = chat_type;
  data->msg_type = msg_type;
  g_idle_add(message_received_idle, data);
}

/* */
void Controller::users_list(const std::string& roomname,
			    const std::unordered_map<std::string, std::string>& users_map)
{
  size_t count = users_map.size();            //for allocate the neccessary memory
  char **users = g_new0(char*, count + 1);
  char **statuses = g_new0(char*, count + 1); 
  size_t i = 0;
  for (const auto &p : users_map) {
    users[i] = g_strdup(p.first.c_str());     //copy username
    statuses[i] = g_strdup(p.second.c_str()); //copy status
    i++;
  }
  
  if (roomname != "") {
    RoomUsersListIdle *data = g_new0(RoomUsersListIdle, 1);
    data->room_name =  g_strdup(roomname.c_str());
    data->users = users;
    data->statuses = statuses;
    g_idle_add(room_users_idle, data); 
  } else {
    ChatUsersListIdle *data = g_new0(ChatUsersListIdle, 1);
    data->users = users;
    data->statuses = statuses;
    g_idle_add(show_users_idle, data);
  }
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
