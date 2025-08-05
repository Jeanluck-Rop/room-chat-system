#pragma once

#include <iostream>
#include <unordered_map>
#include <nlohmann/json.hpp>

/**
 * @class Message
 *
 * Represents a JSON-based chat protocol message with helper methods to create, parse, and access data.
 * The Message class encapsulates incoming and outgoing chat messages using JSON format.
 * It provides utilities for constructing the JSON messages protocol.
 **/
class Message
{
public:
  /** @enum Enum for identifying the type of JSON message **/
  enum class Type
    {
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
  
  /* Default constructor. Initializes an empty JSON message */
  Message();
  
  /**
   * Constructs a Message from a given JSON object.
   *
   * @param json_message The JSON object to wrap as a Message.
   **/
  explicit Message(const nlohmann::json& json_message);

  /**
   * Parses a raw JSON string into the internal data structure.
   *
   * @param raw_message The raw string from the server or input.
   * @return true if successfully parsed, false otherwise.
   **/
  bool parse(const std::string& raw_message);
  
  /**
   * Retrieves the type of the message.
   *
   * @return The corresponding Message::Type enum.
   **/
  Type get_type() const;

  /**
   * Retrieves the username field from the message.
   *
   * @return The username as a string or empty string if missing.
   **/
  std::string get_username() const;

  /**
   * Retrieves the text content from the message.
   *
   * @return The text content as a string or empty string if missing.
   **/
  std::string get_text() const;

  /**
   * Retrieves the status field from the message.
   *
   * @return The status as a string or empty string if missing.
   **/
  std::string get_status() const;

  /**
   * Retrieves the operation field from the message.
   *
   * @return The operation as a string or empty string if missing.
   **/
  std::string get_operation() const;

  /**
   * Retrieves the result field from the message.
   *
   * @return The result as a string or empty string if missing.
   **/
  std::string get_result() const;

  /**
   * Retrieves the extra field from the message.
   *
   * @return The extra as a string or empty string if missing.
   **/
  std::string get_extra() const;

  /**
   * Retrieves the count field from the message, if present.
   *
   * @return The integer value of 'count', or 0 if missing.
   **/
  int get_count() const;
  
  /**
   * Retrieves a map of users and their statuses from the message data.
   *
   * @return A map of usernames to their statuses. Returns an empty map if the data is missing or invalid.
   **/
  std::unordered_map<std::string, std::string> get_users() const;

  /**
   * Retrieves the room name field from the message.
   *
   * @return Room name string or empty string if missing.
   **/
  std::string get_roomname() const;

  /**
   * Serializes the message to a compact JSON string format.
   *
   * @return The serialized JSON as a string.
   **/
  std::string to_json() const;

  /**
   * Creates a message of the type IDENTIFY.
   *
   * @param username The username to identify with.
   * @return A Message object representing the IDENTIFY request.
   **/
  static Message create_identify_message(const std::string& username);

  /**
   * Creates a message of the type STATUS.
   *
   * @param status The new user status.
   * @return A Message object representing the STATUS request.
   **/
  static Message create_status_message(const std::string& status);

  /**
   * Creates a message of the type TEXT.
   *
   * @param target_username Recipient's username.
   * @param text The message content.
   * @return A Message object representing the TEXT request.
   **/
  static Message create_private_text_message(const std::string& target_username, const std::string& text);

  /**
   * Creates a message of the type PUBLIC_TEXT.
   *
   * @param text Message to broadcast.
   * @return A Message object representing the PUBLIC_TEXT request.
   **/
  static Message create_public_text_message(const std::string& text);

  /**
   * Creates a message of the type USER_LIST.
   *
   * @return A Message object representing the USER_LIST request.
   **/
  static Message create_users_list_message();

  /**
   * Creates a message of the type NEW_ROOM.
   *
   * @param roomname Name of the new room.
   * @return A Message object representing the NEW_ROOM request.
   **/
  static Message create_new_room_message(const std::string& roomname);

  /**
   * Creates a message of the type INVITE.
   *
   * @param roomname Room to invite users to.
   * @param usernames List of usernames to invite.
   * @return A Message object representing the INVITE request.
   **/
  static Message create_invite_message(const std::string& roomname, const std::vector<std::string>& usernames);

  /**
   * Creates a message of the type JOIN_ROOM.
   *
   * @param roomname Target room name.
   * @return A Message object representing the JOIN_ROOM request.
   **/
  static Message create_join_room_message(const std::string& roomname);

  /**
   * Creates a message of the type ROOM_USERS.
   *
   * @param roomname The room of interest.
   * @return A Message object representing the ROOM_USERS request.
   **/
  static Message create_room_users_message(const std::string& roomname);

  /**
   * Creates a message of the type ROOM_TEXT.
   *
   * @param roomname Target room.
   * @param text Message content.
   * @return A Message object representing the ROOM_TEXT request.
   **/
  static Message create_room_text_message(const std::string& roomname, const std::string& text);

  /**
   * Creates a message of the type LEAVE_ROOM.
   *
   * @param roomname Room to leave.
   * @return A Message object representing the LEAVE_ROOM request.
   **/
  static Message create_leave_room_message(const std::string& roomname);

  /**
   * Creates a message of the type DISCONNECT.
   *
   * @return A Message object representing the DISCONNECT request.
   **/
  static Message create_disconnect_message();
  
private:
  /**
   * Internal JSON representation of the message.
   * Stores the actual content of the message using the nlohmann::json structure.
   * Used for parsing, serialization, and field access.
   **/
  nlohmann::json json_data;
  
  /**
   * Parses a string into the corresponding Message::Type enum.
   *
   * @param type_str The string representing the type.
   * @return A valid Message::Type enum value, or UNKNOWN if not recognized.
   **/
  Type parse_type(const std::string& type_str) const;
};
