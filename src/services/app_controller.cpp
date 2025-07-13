#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include "../network/http_client.hpp"
#include "../models/models.hpp"
#include "../db/db_manager.hpp"
#include "app_controller.hpp"

using json = nlohmann::json;

AppController::AppController() : m_db("abscli", "abscli.db") { }

auto AppController::login() -> bool {
  std::string serverUrl;
  std::string username;
  std::string password;
  json loginResponse;

  do {
    std::cout << "Enter abs server url: ";
    std::cin >> serverUrl;
  } while (!abscli::http::pingServer(serverUrl));
  do {
    m_serverUrl = serverUrl;
    std::cout << "Enter username: ";
    std::cin >> username;
    std::cout << "Enter password: ";
    std::cin >> password;

    json credentialsJsonPayloadObj;
    credentialsJsonPayloadObj["username"] = username;
    credentialsJsonPayloadObj["password"] = password;
    std::string credentialsJsonPayload = credentialsJsonPayloadObj.dump();

    try {
      loginResponse = abscli::http::postRequest(m_serverUrl, "/login", credentialsJsonPayload);
    } catch (const std::runtime_error& e) {
      std::cout << e.what() << "\n";
    }
  } while (!loginResponse["user"].contains("refreshToken"));

  m_accessToken = getJsonValue(loginResponse["user"], "accessToken", "");
  m_userId = getJsonValue(loginResponse["user"], "id", "abscli_user");
  std::cout << "accessToken: " << m_accessToken << "\n";

  abscli::models::User user;
  try {
    user.id = m_userId;
    user.username = username;
    user.absServer = m_serverUrl;
    user.createdAt = getJsonValue(loginResponse["user"], "createdAt", 0);
    user.accessToken = m_accessToken;
  } catch (const std::exception& e) {
    std::cerr << "Failed to parse user data from API response: " << e.what() << "\n";
    return false;
  }
  m_db.updateUsersTableAfterLogin(user);
  return true;
}
