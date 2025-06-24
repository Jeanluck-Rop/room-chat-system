#include "view.hpp"

using namespace std;

/**
 * Waits for and returns user input from standard input.
 *
 * @return std::string The string entered by the user.
 **/
string TerminalView::get_user_input()
{
  //This is for ensure a disconnection of a client that identifies with an existing username
  struct pollfd fds[1];
  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;
  int ret = poll(fds, 1, 200); //Wait 200ms for input
  string input;
  if (ret > 0 && (fds[0].revents & POLLIN))
    getline(cin, input);
  return input;
}

/**
 * Prints an informational message.
 *
 * @param text The message content to display.
 **/
void TerminalView::print_info(const string& text)
{
  cout << INFO << "[INFO]: " << text << "." << ORIGIN << endl;
}

/**
 * Prints a message indicating invalid input or usage.
 *
 * @param text The message content to display.
 **/
void TerminalView::print_invalid(const string& text)
{
  cout << INVALID << "[ALERT]: " << text << ORIGIN << endl;
}

/**
 * Prints usage instructions or for users list.
 *
 * @param text The message content to display.
 **/
void TerminalView::print_use(const string& text)
{
  cout << INFO << text << ORIGIN << endl;
}

/**
 * Prints a system error message in bright red.
 *
 * @param text The message content to display.
 **/
void TerminalView::print_error(const string& text)
{
  cerr << ERROR << "[ERROR]: " << text << "." << ORIGIN << endl;
}

/**
 * Prints a message from the server.
 *
 * @param text The server message to display.
 **/
void TerminalView::print_server(const string& text)
{
  cout << SERVER << "   " << u8"\u2022 " << text << ORIGIN << endl;
}

/**
 * Prints a success message in green.
 *
 * @param text The success message to display.
 **/
void TerminalView::print_success(const string& text)
{
  cout << SUCCESS << "   " << u8"\u2022 " << text << ORIGIN << endl;
}

/**
 * Prints a failure message.
 *
 * @param text The failure message to display.
 **/
void TerminalView::print_fail(const string& text)
{
  cout << FAILED << "   " << u8"\u2022 " << text << ORIGIN << endl;
}

/**
 * Prints room-specific information or events.
 *
 * @param text The room-related message to display.
 **/
void TerminalView::print_room(const string& text)
{
  cout << ROOM << text << ORIGIN << endl;
}

/**
 * Prints a public chat message with the sender's name.
 *
 * @param username The sender of the message.
 * @param text The chat message content.
 **/
void TerminalView::print_public(const string& username, const string& text)
{
  cout << PUBLIC << "[PUBLIC] (" << username << "): " << text << ORIGIN << endl;
}

/**
 * Prints a private message with the sender's name.
 *
 * @param username The sender of the message.
 * @param text The message content.
 **/
void TerminalView::print_private(const string& username, const string& text)
{
  cout << PRIVATE << "[PRIVATE] (" << username << "): " << text << ORIGIN << endl;
}
