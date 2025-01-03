#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <nlohmann/json.hpp>

class Message {
public:
    enum class Type {      
      RESPONSE,
      NEW_USER,
      NEW_STATUS,
      TEXT_FROM,
      PUBLIC_TEXT_FROM,
      DISCONNECT,
      UNKNOWN
    };
  
  // Constructors
  Message();
  explicit Message(const nlohmann::json& json_message);
  
  // Methods for creating different types of messages
  static Message create_identify_message(const std::string& username);
  static Message create_text_message(const std::string& text);
  static Message create_status_message(const std::string& status);
  static Message create_private_text_message(const std::string& target_username, const std::string& text);
  static Message create_public_text_message(const std::string& text);
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
  
  // Serialization method
  std::string to_json() const;
  
private:
  nlohmann::json json_data;
  
  // Auxiliar method to determine message type
  Type parse_type(const std::string& type_str) const;
};

#endif // MESSAGE_HPP
