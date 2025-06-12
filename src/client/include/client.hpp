#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <vector>
#include <csignal>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <nlohmann/json.hpp>
#include "message.hpp"

class Client {
public:
  static Client& instance();  // Singleton getter

  Client(const Client&) = delete;
  Client& operator=(const Client&) = delete;
  
  bool connect_to_server(const std::string& server_ip, int port);
  void run_client();
  void disconnect();
  
  static void signal_handler(int signal);
  
private:
  Client();
  ~Client();
  
  int socket_fd;
  std::atomic<bool> is_connected;
  std::thread listener_thread;
  std::thread actions_thread;

  void receive_message();
  void send_message(const std::string& message);
  void handle_response(const Message& incoming_msg);
  void handle_message_received(const std::string& raw_message);
  bool check_id(std::string& user_input);
  void change_status(std::string& user_input);
  void direct_message(std::string& user_input);
  void new_room(std::string& user_input);
  void invite_users(std::string& user_input);
  void join_room(std::string& user_input);
  void room_users(std::string& user_input);
  void room_text(std::string& user_input);
  void left_room(std::string& user_input);
  void disconnect_user();
  void handle_user_actions();
};

#endif // CLIENT_H
