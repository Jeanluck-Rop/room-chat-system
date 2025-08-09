#pragma once

#include <stdexcept>
#include <unordered_map>

/**
 * @class ChatCounter
 *
 * Utility class for tracking numeric values associated with chat names.
 * This class maintains a mapping between chat names and the count of the memebrs on the chat.
 * It supports adding, updating, and querying counters.
 **/
class ChatCounter
{
public:
  /**
   * Adds or sets the counter for a given chat name.
   * If the chat name already exists in the map, its value is replaced.
   *
   * @param chat_name The name of the chat.
   * @param i The initial value to set for the chat.
   **/
  void add(const std::string& chat_name, int i)
  {
    counters[chat_name] = i;
  }

  /**
   * Updates (increments or decrements) the counter for a chat.
   * If the chat name does not exist, a runtime error is thrown.
   *
   * @param chat_name The name of the chat.
   * @param n The value to add (positive or negative) to the current counter.
   * @throws std::runtime_error if the chat name does not exist.
   **/
  void update(const std::string& chat_name, int n)
  {
    if (counters.find(chat_name) == counters.end())
      throw std::runtime_error("Chat name does not exist: [" + chat_name + "]");
  
    counters[chat_name] += n;
  }
  
  /**
   * Retrieves the current counter value for a chat.
   * If the chat name is not found, returns 0 by default.
   *
   * @param chat_name The name of the chat.
   * @return The counter value associated with the chat, or 0 if not found.
   **/
  int count(const std::string& chat_name) const
  {
    auto k = counters.find(chat_name);
    return (k != counters.end()) ? k->second : 0;
  }

  /**
   * Removes the counter entry for a given chat name.
   * If the chat name does not exist, a runtime error is thrown.
   *
   * @param chat_name The name of the chat to remove from the map.
   * @throws std::runtime_error if the chat name does not exist.
   **/
  void remove(const std::string& chat_name)
  {
    if (counters.find(chat_name) == counters.end())
      throw std::runtime_error("Chat name does not exist: [" + chat_name + "]");
    
    counters.erase(chat_name);
  }
  
private:
  /**
   * Internal map storing chat names and their associated counters.
   * The key is the chat name (std::string), and the value is an integer counter.
   **/
  std::unordered_map<std::string, int> counters;
};
