#include "message.hpp"

Message::Message() : json_data({}) {}

Message::Message(const nlohmann::json& json_message) : json_data(json_message) {}

Message Message::create_identify_message(const std::string& username) {
  nlohmann::json msg;
  msg["type"] = "IDENTIFY";
  msg["username"] = username;
  return Message(msg);
}

Message Message::create_status_message(const std::string& status) {
  nlohmann::json msg;
  msg["type"] = "STATUS";
  msg["status"] = status;
  return Message(msg);
}

Message Message::create_private_text_message(const std::string& target_username, const std::string& text) {
  nlohmann::json msg;
  msg["type"] = "TEXT";
  msg["username"] = target_username;
  msg["text"] = text;
  return Message(msg);
}

Message Message::create_public_text_message(const std::string& text) {
  nlohmann::json msg;
  msg["type"] = "PUBLIC_TEXT";
  msg["text"] = text;
  return Message(msg);
}

Message Message::create_users_list_message() {
  nlohmann::json msg;
  msg["type"] = "USER_LIST";
  return Message(msg);
}

Message Message::create_new_room_message(const std::string& roomname) {
  nlohmann::json msg;
  msg["type"] = "NEW_ROOM";
  msg["roomname"] = roomname;
  return Message(msg);
}

Message Message::create_invite_message(const std::string& roomname, const std::vector<std::string>& usernames) {
  nlohmann::json msg;
  msg["type"] = "INVITE";
  msg["roomname"] = roomname;
  msg["usernames"] = usernames;
  return Message(msg);
}

Message Message::create_join_room_message(const std::string& roomname) {
  nlohmann::json msg;
  msg["type"] = "JOIN_ROOM";
  msg["roomname"] = roomname;
  return Message(msg);
}

Message Message::create_room_users_message(const std::string& roomname) {
  nlohmann::json msg;
  msg["type"] = "ROOM_USERS";
  msg["roomname"] = roomname;
  return Message(msg);
}

Message Message::create_room_text_message(const std::string& roomname, const std::string& text) {
  nlohmann::json msg;
  msg["type"] = "ROOM_TEXT";
  msg["roomname"] = roomname;
  msg["text"] = text;
  return Message(msg);
}

Message Message::create_left_room_message(const std::string& roomname) {
  nlohmann::json msg;
  msg["type"] = "LEFT_ROOM";
  msg["roomname"] = roomname;
  return Message(msg);
}

Message Message::create_disconnect_message() {
  nlohmann::json msg;
  msg["type"] = "DISCONNECT";
  return Message(msg);
}

Message Message::create_response_message(const std::string& operation, const std::string& result) {
  nlohmann::json msg;
  msg["type"] = "RESPONSE";
  msg["operation"] = operation;
  msg["result"] = result;
  return Message(msg);
}

bool Message::parse(const std::string& raw_message) {
  try {
    json_data = nlohmann::json::parse(raw_message);
  } catch (const std::exception& e) {
    std::cerr << "Error parsing message: " << e.what() << std::endl;
    return false;
  }
  return true;
}

Message::Type Message::get_type() const {
  if (json_data.contains("type")) {
    std::string type_str = json_data["type"];
    return parse_type(type_str);
  }
  return Type::UNKNOWN;
}

std::string Message::get_username() const {
  if (json_data.contains("username"))
    return json_data["username"];
  return "";
}

std::string Message::get_text() const {
  if (json_data.contains("text"))
    return json_data["text"];
  return "";
}

std::string Message::get_status() const {
  if (json_data.contains("status"))
    return json_data["status"];
  return "";
}

std::string Message::get_operation() const {
  if (json_data.contains("operation"))
    return json_data["operation"];
  return "";
}

std::string Message::get_result() const {
  if (json_data.contains("result"))
    return json_data["result"];
  return "";
}

std::string Message::get_extra() const {
  if (json_data.contains("extra"))
    return json_data["extra"];
  return "";
}

std::string Message::get_users() const {
  if (json_data.contains("users") && json_data["users"].is_object()) {
    std::string list = "";
    int n = 1;    
    for (const auto& [user, status] : json_data["users"].items()) {
      list += std::to_string(n) + ". " + user + " : " + status.get<std::string>() + "\n";
      n++;
    }
    return list;
  }
  return "";
}

std::string Message::get_roomname() const {
  if (json_data.contains("roomname"))
    return json_data["roomname"];
  return "";
}

std::string Message::to_json() const {
  return json_data.dump();
}

Message::Type Message::parse_type(const std::string& type_str) const {
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
