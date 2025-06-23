#pragma once

#include <string>
#include <poll.h>
#include <unistd.h>
#include <iostream>

class TerminalView
{
public:
  static void display_message(const std::string& message);
  
  static void display_error(const std::string& message);
  
  static std::string get_user_input();
};
