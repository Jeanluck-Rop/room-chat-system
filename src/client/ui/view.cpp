#include "view.hpp"

void TerminalView::display_message(const std::string& message) {
    std::cout << message << std::endl;
}

void TerminalView::display_error(const std::string& message) {
    std::cerr << message << std::endl;
}

std::string TerminalView::get_user_input() {
    std::string input;

    //This is for ensure a disconnection of a client that identifies with an existing username
    struct pollfd fds[1];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    // Wait 200ms for input
    int ret = poll(fds, 1, 200);
    if (ret > 0 && (fds[0].revents & POLLIN))
        std::getline(std::cin, input);
    
    return input;
}
