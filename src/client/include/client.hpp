#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <thread>
#include <atomic>
#include <nlohmann/json.hpp>
#include <memory>
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

  void handle_user_actions();
  void send_message(const std::string& msg);
  void receive_message();
};

#endif // CLIENT_H
