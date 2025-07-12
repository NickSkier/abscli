#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include "../network/http_client.hpp"
#include "app_controller.hpp"

using json = nlohmann::json;

void AppController::login() {
  std::string serverUrl;
  std::string username;
  std::string password;
  json loginResponse;

  do {
    std::cout << "Enter abs server url: ";
    std::cin >> serverUrl;
  } while (!HttpClient::pingServer(serverUrl));
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

    loginResponse = HttpClient::postRequest(m_serverUrl, "/login", credentialsJsonPayload);
  } while (!loginResponse["user"].contains("accessToken"));

  m_accessToken = loginResponse["user"].value("accessToken", "");
  std::cout << "accessToken: " << m_accessToken << "\n";
}

