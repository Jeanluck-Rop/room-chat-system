#ifndef TERMINAL_VIEW_H
#define TERMINAL_VIEW_H

#include <string>

class TerminalView {
public:
  static void display_message(const std::string& message);
  static void display_error(const std::string& message);
  static std::string get_user_input();
};

#endif // TERMINAL_VIEW_H
