#ifndef VIEW_H
#define VIEW_H

#include <string>

class TerminalView {
public:
  static void display_message(const std::string& message);
  static void display_error(const std::string& message);
  static std::string get_user_input();
};

#endif // VIEW_H
