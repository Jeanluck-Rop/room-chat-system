#pragma once

#include <string>
#include <poll.h>
#include <unistd.h>
#include <iostream>

/**
 * @class TerminalView
 *
 * Provides utility functions for formatted terminal I/O for a chat or messaging system.
 * This class contains only static methods and constants, and serves as a view component
 * in a terminal-based application.
 * It is responsible for printing categorized messages to the standard output with specific
 * color-coded formatting, as well as reading user input.
 **/
class TerminalView
{
public:
  /**
   * Waits for and returns user input from standard input.
   *
   * @return std::string The string entered by the user.
   **/
  static std::string get_user_input();

  /**
   * Prints the list of chat commands.
   **/
  static void print_commands();

  /**
   * Prints an informational message.
   *
   * @param text The message content to display.
   **/
  static void print_info(const std::string& text);

  /**
   * Prints a message indicating invalid input or usage.
   *
   * @param text The message content to display.
   **/
  static void print_invalid(const std::string& text);

  /**
   * Prints usage instructions or for users list.
   *
   * @param text The message content to display.
   **/
  static void print_use(const std::string& text);

  /**
   * Prints a system error message in bright red.
   *
   * @param text The message content to display.
   **/
  static void print_error(const std::string& text);

  /**
   * Prints a message from the server.
   *
   * @param text The server message to display.
   **/
  static void print_server(const std::string& text);

  /**
   * Prints a success message in green.
   *
   * @param text The success message to display.
   **/
  static void print_success(const std::string& text);

  /**
   * Prints a failure message.
   *
   * @param text The failure message to display.
   **/
  static void print_fail(const std::string& text);

  /**
   * Prints room events.
   *
   * @param text The room-related message to display.
   **/
  static void print_roomf(const std::string& text);
  
  /**
   * Prints room text.
   *
   * @param text The room-related message to display.
   **/
  static void print_room(const std::string& text);

  /**
   * Prints a public chat message with the sender's name.
   *
   * @param username The sender of the message.
   * @param text The chat message content.
   **/
  static void print_public(const std::string& username, const std::string& text);

  /**
   * Prints a private message with the sender's name.
   *
   * @param username The sender of the message.
   * @param text The message content.
   **/
  static void print_private(const std::string& username, const std::string& text);
  
private:
  /* ANSI escape sequence to reset terminal formatting */
  static constexpr const char* ORIGIN  = "\033[m";
  /* ANSI color code for informational messages */
  static constexpr const char* INFO    = "\033[38;5;117m";
  /* ANSI color code for invalid input messages */
  static constexpr const char* INVALID = "\033[38;5;196m";
  /* ANSI color code for error messages */
  static constexpr const char* ERROR   = "\033[38;5;196m";
  /* ANSI color code for server messages */
  static constexpr const char* SERVER  = "\033[3;38;5;220m";
  /* ANSI color code for success messages */
  static constexpr const char* SUCCESS = "\033[3;32m";
  /* ANSI color code for failure or warning messages */
  static constexpr const char* FAILED  = "\033[3;38;5;100m";
  /* ANSI color code for public messages */
  static constexpr const char* PUBLIC  = "\033[38;5;208m";
  /* ANSI color code for private messages */
  static constexpr const char* PRIVATE = "\033[38;5;27m";
  /* ANSI color code for room-related messages */
  static constexpr const char* ROOM    = "\033[38;5;127m";
  /* ANSI color code for room-related notifies */
  static constexpr const char* ROOMF   = "\033[3;38;5;127m";

  /* String to print chat commands */
  static constexpr const char* COMMANDS =
    "\033[38;5;105m   * Client commands for the chat *\n"
    "\033[38;5;105m\u2022 --comms\033[38;5;105m\n"
    "\t Show this list of available commands.\n"
    "\033[38;5;105m\u2022 --status <AWAY|ACTIVE|BUSY>\033[38;5;105m\n"
    "\t Change your current status.\n"
    "\033[38;5;105m\u2022 --users\033[38;5;105m\n"
    "\t Show the list of connected users.\n"
    "\033[38;5;105m\u2022 --dm <target_user> :<message>\033[38;5;105m\n"
    "\t Send a direct (private) message to a user.\n"
    "\033[38;5;105m\u2022 --new <roomname>\033[38;5;105m\n"
    "\t Create a new room.\n"
    "\033[38;5;105m\u2022 --invite <user1>;<user2> : <room>\033[38;5;105m\n"
    "\t Invite users to a room.\n"
    "\033[38;5;105m\u2022 --join <roomname>\033[38;5;105m\n"
    "\t Join an existing room.\n"
    "\033[38;5;105m\u2022 --roomies <roomname>\033[38;5;105m\n"
    "\t Show users in the specified room.\n"
    "\033[38;5;105m\u2022 --textr <roomname> :<message>\033[38;5;105m\n"
    "\t Send a message to a room.\n"
    "\033[38;5;105m\u2022 --leave <roomname>\033[38;5;105m\n"
    "\t Leave a room you are in.\n"
    "\033[38;5;105m\u2022 --exit\033[38;5;105m\n"
    "\t Disconnect from the chat server. \033[0m";
};
