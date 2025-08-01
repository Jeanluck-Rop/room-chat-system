#pragma once

#include <stdexcept>
#include <unordered_map>

class ChatCounter
{
public:
  /* */
  void add(const std::string& chat_name, int i)
  {
    counters[chat_name] = i;
  }

  /* */
  void update(const std::string& chat_name, int n)
  {
    if (counters.find(chat_name) == counters.end())
      throw std::runtime_error("Chat name does not exist: [" + chat_name + "]");
  
    counters[chat_name] += n;
  }
  
  /* */
  int count(const std::string& chat_name) const
  {
    auto k = counters.find(chat_name);
    return (k != counters.end()) ? k->second : 0;
  }
  
private:

  /* */
  std::unordered_map<std::string, int> counters;
};
