#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <nlohmann/json.hpp>

class Message {
public:
  // Enum class for handle json protocol type messages
    enum class Type {
      NEW_USER,
      NEW_STATUS,
      TEXT_FROM,
      PUBLIC_TEXT_FROM,
      USER_LIST,
      INVITATION,
      JOINED_ROOM,
      ROOM_USER_LIST,
      ROOM_TEXT_FROM,
      LEFT_ROOM,
      DISCONNECTED,
      RESPONSE,
      UNKNOWN //Default type message
    };
  
  Message();
  explicit Message(const nlohmann::json& json_message);
  
  // Methods for creating different types of messages according to the json protocol
  static Message create_identify_message(const std::string& username);
  static Message create_status_message(const std::string& status);
  static Message create_private_text_message(const std::string& target_username, const std::string& text);
  static Message create_public_text_message(const std::string& text);
  static Message create_users_list_message();
  static Message create_new_room_message(const std::string& roomname);
  static Message create_invite_message(const std::string& roomname, const std::string& usernames);
  static Message create_join_room_message(const std::string& roomname);
  static Message create_room_users_message(const std::string& roomname);
  static Message create_room_text_message(const std::string& roomname, const std::string& text);
  static Message create_left_room_message(const std::string& roomname);
  static Message create_disconnect_message();
  static Message create_response_message(const std::string& operation, const std::string& result);
  
  // Method for parsing incoming messages
  bool parse(const std::string& raw_message);
  
  // Getters for received message details
  Type get_type() const;
  std::string get_username() const;
  std::string get_text() const;
  std::string get_status() const;
  std::string get_operation() const;
  std::string get_result() const;
  std::string get_extra() const;
  std::string get_users() const;
  std::string get_roomname() const;
  // Serialization method, convert the message in json type
  std::string to_json() const;
  
private:
  nlohmann::json json_data;
  
  // Auxiliar method to determine message type
  Type parse_type(const std::string& type_str) const;
};

#endif // MESSAGE_HPP
