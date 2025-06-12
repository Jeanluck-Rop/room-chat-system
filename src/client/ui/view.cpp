#include "view.hpp"
#include <iostream>

void TerminalView::display_message(const std::string& message) {
    std::cout << message << std::endl;
}

void TerminalView::display_error(const std::string& message) {
    std::cerr << message << std::endl;
}

std::string TerminalView::get_user_input() {
    std::string input;
    std::getline(std::cin, input);
    return input;
}
