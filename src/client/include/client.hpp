#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <nlohmann/json.hpp>

class Client {
public:
  Client();
  ~Client();
  
  bool connect_to_server(const std::string& server_ip, int port);
  void run_client();
  void disconnect();
  
  static void signal_handler(int signal);
  
private:
  int socket_fd;
  std::atomic<bool> is_connected;
  std::thread listener_thread; 
};

#endif // CLIENT_H
