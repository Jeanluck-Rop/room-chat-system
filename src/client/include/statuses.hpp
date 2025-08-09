#pragma once

//#include <optional>
#include <stdexcept>
#include <unordered_map>

/**
 * @class StatusesList
 *
 * Utility class for tracking user statuses.
 * Maintains a mapping between usernames and their associated status strings.
 * Supports adding, updating, querying, and removing statuses.
 */
class StatusesList
{
public:
  /**
   * Adds or sets the status for a given username.
   * If the username already exists in the map, its value is replaced.
   *
   * @param username The username to associate with the status.
   * @param status The status string to store.
   **/
  void add(const std::string& username, const std::string& status)
  {
    statuses_list[username] = status;
  }

  /**
   * Updates the status for an existing username.
   * If the username does not exist, a runtime error is thrown.
   *
   * @param username The username whose status should be updated.
   * @param status The new status string to assign.
   * @throws std::runtime_error if the username does not exist.
   **/
  void update(const std::string& username, const std::string& status)
  {
    if (statuses_list.find(username) == statuses_list.end())
      throw std::runtime_error("Username does not exist: [" + username + "]");

    statuses_list[username] = status;
  }

  /**
   * Retrieves the current status for a given username.
   * If the username is not found, returns an empty string.
   *
   * @param username The username to query.
   * @return The status string associated with the username, or empty string if not found.
   **/
  std::string status(const std::string& username) const
  {
    auto it = statuses_list.find(username);
    return (it != statuses_list.end()) ? it->second : "";
  }

  /**
   * Removes the status entry for a given username.
   * If the username does not exist, a runtime error is thrown.
   *
   * @param username The username whose status should be removed.
   * @throws std::runtime_error if the username does not exist.
   **/
  void remove(const std::string& username)
  {
    if (statuses_list.find(username) == statuses_list.end())
      throw std::runtime_error("Username does not exist: [" + username + "]");

    statuses_list.erase(username);
  }

private:
  /**
   * Internal map storing usernames and their associated statuses.
   **/
  std::unordered_map<std::string, std::string> statuses_list;
};
