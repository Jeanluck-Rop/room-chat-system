#include "message.hpp"
#include <iostream>

Message::Message() : json_data({}) {}

Message::Message(const nlohmann::json& json_message) : json_data(json_message) {}

Message Message::create_identify_message(const std::string& username) {
  nlohmann::json msg;
  msg["type"] = "IDENTIFY";
  msg["username"] = username;
  return Message(msg);
}

Message Message::create_text_message(const std::string& text) {
  nlohmann::json msg;
  msg["type"] = "TEXT";
  msg["text"] = text;
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
    return Type::PRIVATE_TEXT;
  if (type_str == "PUBLIC_TEXT_FROM")
    return Type::PUBLIC_TEXT;
  if (type_str == "DISCONNECT")
    return Type::DISCONNECT;
  return Type::UNKNOWN;
}
