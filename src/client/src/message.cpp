#include "message.hpp"

/* Default constructor. Initializes an empty JSON message */
Message::Message() : json_data({}) {}

/**
 * Constructs a Message from a given JSON object.
 *
 * @param json_message The JSON object to wrap as a Message.
 **/
Message::Message(const nlohmann::json& json_message) : json_data(json_message) {}

/**
 * Parses a raw JSON string into the internal data structure.
 *
 * @param raw_message The raw string from the server or input.
 * @return true if successfully parsed, false otherwise.
 **/
bool Message::parse(const std::string& raw_message)
{
  try
    {
      json_data = nlohmann::json::parse(raw_message);
    }
  catch (const std::exception& e)
    {
      std::cerr << "Error parsing message: " << e.what() << std::endl;
      return false;
    }
  return true;
}

/**
 * Retrieves the type of the message.
 *
 * @return The corresponding Message::Type enum.
 **/
Message::Type Message::get_type() const
{
  if (json_data.contains("type")) {
    std::string type_str = json_data["type"];
    return parse_type(type_str);
  }
  return Type::UNKNOWN;
}

/**
 * Retrieves the username field from the message.
 *
 * @return The username as a string or empty string if missing.
 **/
std::string Message::get_username() const
{
  if (json_data.contains("username"))
    return json_data["username"];
  return "";
}

/**
 * Retrieves the text content from the message.
 *
 * @return The text content as a string or empty string if missing.
 **/
std::string Message::get_text() const
{
  if (json_data.contains("text"))
    return json_data["text"];
  return "";
}

/**
 * Retrieves the status field from the message.
 *
 * @return The status as a string or empty string if missing.
 **/
std::string Message::get_status() const
{
  if (json_data.contains("status"))
    return json_data["status"];
  return "";
}

/**
 * Retrieves the operation field from the message.
 *
 * @return The operation as a string or empty string if missing.
 **/
std::string Message::get_operation() const
{
  if (json_data.contains("operation"))
    return json_data["operation"];
  return "";
}

/**
 * Retrieves the result field from the message.
 *
 * @return The result as a string or empty string if missing.
 **/
std::string Message::get_result() const
{
  if (json_data.contains("result"))
    return json_data["result"];
  return "";
}

/**
 * Retrieves the extra field from the message.
 *
 * @return The extra as a string or empty string if missing.
 **/
std::string Message::get_extra() const
{
  if (json_data.contains("extra"))
    return json_data["extra"];
  return "";
}

 /**
  * Retrieves the count field from the message, if present.
  *
  * @return The integer value of 'count', or 0 if missing.
  **/
int Message::get_count() const
{
  if (json_data.contains("count") && json_data["count"].is_number_integer())
    return json_data["count"].get<int>();
  return 0;
}

/**
 * Retrieves a map of users and their statuses from the message data.
 *
 * Parses the internal JSON object to extract a list of users, where each key is a username
 * and each value is the corresponding status.
 *
 * @return A map of usernames to their statuses. Returns an empty map if the data is missing or invalid.
 **/
std::unordered_map<std::string, std::string> Message::get_users() const
{
  std::unordered_map<std::string, std::string> list;
  if (json_data.contains("users") && json_data["users"].is_object())
    for (const auto& [user, status] : json_data["users"].items())
      list[user] = status.get<std::string>();
  return list;
}

/**
 * Retrieves the room name field from the message.
 *
 * @return Room name string or empty string if missing.
 **/
std::string Message::get_roomname() const
{
  if (json_data.contains("roomname"))
    return json_data["roomname"];
  return "";
}

/**
 * Serializes the message to a compact JSON string format.
 *
 * @return The serialized JSON as a string.
 **/
std::string Message::to_json() const
{
  return json_data.dump();
}

/**
 * Creates a message of the type IDENTIFY.
 *
 * @param username The username to identify with.
 * @return A Message object representing the IDENTIFY request.
 **/
Message Message::create_identify_message(const std::string& username)
{
  nlohmann::json msg;
  msg["type"] = "IDENTIFY";
  msg["username"] = username;
  return Message(msg);
}

/**
 * Creates a message of the type STATUS.
 *
 * @param status The new user status.
 * @return A Message object representing the STATUS request.
 **/
Message Message::create_status_message(const std::string& status)
{
  nlohmann::json msg;
  msg["type"] = "STATUS";
  msg["status"] = status;
  return Message(msg);
}

/**
 * Creates a message of the type TEXT.
 *
 * @param target_username Recipient's username.
 * @param text The message content.
 * @return A Message object representing the TEXT request.
 **/
Message Message::create_private_text_message(const std::string& target_username,
					     const std::string& text)
{
  nlohmann::json msg;
  msg["type"] = "TEXT";
  msg["username"] = target_username;
  msg["text"] = text;
  return Message(msg);
}

/**
 * Creates a message of the type PUBLIC_TEXT.
 *
 * @param text Message to broadcast.
 * @return A Message object representing the PUBLIC_TEXT request.
 **/
Message Message::create_public_text_message(const std::string& text)
{
  nlohmann::json msg;
  msg["type"] = "PUBLIC_TEXT";
  msg["text"] = text;
  return Message(msg);
}

/**
 * Creates a message of the type USER_LIST.
 *
 * @return A Message object representing the USER_LIST request.
 **/
Message Message::create_users_list_message()
{
  nlohmann::json msg;
  msg["type"] = "USERS";
  return Message(msg);
}

/**
 * Creates a message of the type NEW_ROOM.
 *
 * @param roomname Name of the new room.
 * @return A Message object representing the NEW_ROOM request.
 **/
Message Message::create_new_room_message(const std::string& roomname)
{
  nlohmann::json msg;
  msg["type"] = "NEW_ROOM";
  msg["roomname"] = roomname;
  return Message(msg);
}

/**
 * Creates a message of the type INVITE.
 *
 * @param roomname Room to invite users to.
 * @param usernames List of usernames to invite.
 * @return A Message object representing the INVITE request.
 **/
Message Message::create_invite_message(const std::string& roomname,
				       const std::vector<std::string>& usernames)
{
  nlohmann::json msg;
  msg["type"] = "INVITE";
  msg["roomname"] = roomname;
  msg["usernames"] = usernames;
  return Message(msg);
}

/**
 * Creates a message of the type JOIN_ROOM.
 *
 * @param roomname Target room name.
 * @return A Message object representing the JOIN_ROOM request.
 **/
Message Message::create_join_room_message(const std::string& roomname)
{
  nlohmann::json msg;
  msg["type"] = "JOIN_ROOM";
  msg["roomname"] = roomname;
  return Message(msg);
}

/**
 * Creates a message of the type ROOM_USERS.
 *
 * @param roomname The room of interest.
 * @return A Message object representing the ROOM_USERS request.
 **/
Message Message::create_room_users_message(const std::string& roomname)
{
  nlohmann::json msg;
  msg["type"] = "ROOM_USERS";
  msg["roomname"] = roomname;
  return Message(msg);
}

/**
 * Creates a message of the type ROOM_TEXT.
 *
 * @param roomname Target room.
 * @param text Message content.
 * @return A Message object representing the ROOM_TEXT request.
 **/
Message Message::create_room_text_message(const std::string& roomname,
					  const std::string& text)
{
  nlohmann::json msg;
  msg["type"] = "ROOM_TEXT";
  msg["roomname"] = roomname;
  msg["text"] = text;
  return Message(msg);
}

/**
 * Creates a message of the type LEAVE_ROOM.
 *
 * @param roomname Room to leave.
 * @return A Message object representing the LEAVE_ROOM request.
 **/
Message Message::create_leave_room_message(const std::string& roomname)
{
  nlohmann::json msg;
  msg["type"] = "LEAVE_ROOM";
  msg["roomname"] = roomname;
  return Message(msg);
}

/**
 * Creates a message of the type DISCONNECT.
 *
 * @return A Message object representing the DISCONNECT request.
 **/
Message Message::create_disconnect_message()
{
  nlohmann::json msg;
  msg["type"] = "DISCONNECT";
  return Message(msg);
}

/**
 * Parses a string into the corresponding Message::Type enum.
 *
 * @param type_str The string representing the type.
 * @return A valid Message::Type enum value, or UNKNOWN if not recognized.
 **/
Message::Type Message::parse_type(const std::string& type_str) const
{
  if (type_str == "RESPONSE")
    return Type::RESPONSE;
  if (type_str == "NEW_USER")
    return Type::NEW_USER;
  if (type_str == "NEW_STATUS")
    return Type::NEW_STATUS;
  if (type_str == "TEXT_FROM")
    return Type::TEXT_FROM;
  if (type_str == "PUBLIC_TEXT_FROM")
    return Type::PUBLIC_TEXT_FROM;
  if (type_str == "USER_LIST")
    return Type::USER_LIST;
  if (type_str == "INVITATION")
    return Type::INVITATION;
  if (type_str == "JOINED_ROOM")
    return Type::JOINED_ROOM;
  if (type_str == "ROOM_USER_LIST")
    return Type::ROOM_USER_LIST;
  if (type_str == "ROOM_TEXT_FROM")
    return Type::ROOM_TEXT_FROM;
  if (type_str == "LEFT_ROOM")
    return Type::LEFT_ROOM;
  if (type_str == "DISCONNECTED")
    return Type::DISCONNECTED;
  return Type::UNKNOWN;
}
