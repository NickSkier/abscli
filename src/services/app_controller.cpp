#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <vector>
#include "../network/http_client.hpp"
#include "../models/models.hpp"
#include "../db/db_manager.hpp"
#include "../utils/json_utils.hpp"
#include "app_controller.hpp"

using json = nlohmann::json;

AppController::AppController() : m_db("abscli", "abscli.db"), m_refreshTokenStorage("com.nskier.abscli", "abscli") {
  auto usernames = m_db.getUserNames();
  if (usernames.empty()) { login(); }
  else {
    listItems(usernames);
    int usernameSelection;
    std::cout << "Select user profile: ";
    std::cin >> usernameSelection;
    std::string username = usernames[usernameSelection];

    std::optional<std::string> serverUrl, userId;
    do {
      serverUrl = m_db.getUserColumnValue(username, "absServer");
      userId = m_db.getUserColumnValue(username, "id");
      if (serverUrl && userId && abscli::http::pingServer(serverUrl.value())) {
        m_serverUrl = serverUrl.value();
        m_userId = userId.value();
        requestNewTokens();
      }
      else { login(); }
    } while (!(serverUrl && userId));
  }

  syncUserData();
}

auto AppController::requestNewTokens() -> void {
  try {
    const std::optional<std::string>& refreshToken = m_refreshTokenStorage.getToken(m_userId);
    if (!refreshToken) {
      std::cout << "You need to login again" << "\n";
      login();
      return;
    }

    json response = abscli::http::postRequest(m_serverUrl, "/auth/refresh", refreshToken.value(), true);
    if (!response["user"].contains("refreshToken")) {
      std::cout << "You need to login again" << "\n";
      login();
      return;
    }

    const std::string& newRefreshToken = abscli::utils::json::value(response["user"], "refreshToken", "");
    m_refreshTokenStorage.setToken(m_userId, newRefreshToken);

    abscli::models::User user;
    m_userId = abscli::utils::json::value(response["user"], "id", "");
    m_accessToken = abscli::utils::json::value(response["user"], "accessToken", "");

  } catch (const std::runtime_error& e) {
    std::cerr << "\033[1;31m[ERROR]\033[0m while requesting new tokens: " << e.what() << "\n";
  }
}

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

  abscli::models::User user;
  try {
    m_userId         = abscli::utils::json::value(loginResponse["user"], "id", "abscli_user");
    m_accessToken    = abscli::utils::json::value(loginResponse["user"], "accessToken",   "");
    user.createdAt   = abscli::utils::json::value(loginResponse["user"], "createdAt",      0);

    const std::string& refreshToken = abscli::utils::json::value(loginResponse["user"], "refreshToken", "");
    m_refreshTokenStorage.setToken(m_userId, refreshToken);

    std::cout << "refreshToken: " << refreshToken << "\n";
    std::cout << "accessToken: " << m_accessToken << "\n";

    user.id          = m_userId;
    user.username    = username;
    user.absServer   = m_serverUrl;
    user.accessToken = m_accessToken;
  } catch (const std::exception& e) {
    std::cerr << "Failed to parse user data from API response: " << e.what() << "\n";
    return false;
  }
  m_db.updateUsersTableAfterLogin(user);
  return true;
}

auto AppController::syncUserData() -> void {
  const std::optional<json>& response = requestData("/api/me", "id");
  if (response) {
    const json& responseData = response.value();
    abscli::models::User user;
    try {
      user.id                              = abscli::utils::json::value(responseData, "id",           "");
      user.username                        = abscli::utils::json::value(responseData, "username",     "");
      user.absServer                       = m_serverUrl;
      user.email                           = abscli::utils::json::value(responseData, "email",        "");
      user.type                            = abscli::utils::json::value(responseData, "type",         "");
      user.seriesHideFromContinueListening = responseData.at(                         "seriesHideFromContinueListening").dump();
      user.bookmarks                       = responseData.at(                         "bookmarks").dump();
      user.isActive                        = abscli::utils::json::value(responseData, "isActive",  false);
      user.isLocked                        = abscli::utils::json::value(responseData, "isLocked",  false);
      user.lastSeen                        = abscli::utils::json::value(responseData, "lastSeen",      0);
      user.createdAt                       = abscli::utils::json::value(responseData, "createdAt",     0);
      user.permissions                     = responseData.at(                         "permissions").dump();
      user.librariesAccessible             = responseData.at(                         "librariesAccessible").dump();
      user.itemTagsSelected                = responseData.at(                         "itemTagsSelected").dump();
    } catch (const std::exception& e) {
      std::cerr << "Failed to parse user data from API response: " << e.what() << "\n";
      return;
    }
    m_db.updateUsersTable(user);
  }
}

auto AppController::syncLibraries() -> void {
  const std::optional<json>& response = requestData("/api/libraries", "libraries");
  if (response) {
    const json& responseData = response.value()["libraries"];
    std::vector<abscli::models::Library> libraries;
    try {
      for (const auto& library : responseData) {
        libraries.emplace_back(abscli::models::Library{
          .id           = abscli::utils::json::value(library, "id",           ""),
          .name         = abscli::utils::json::value(library, "name",         ""),
          .displayOrder = abscli::utils::json::value(library, "displayOrder",  0),
          .icon         = abscli::utils::json::value(library, "icon",         ""),
          .mediaType    = abscli::utils::json::value(library, "mediaType",    ""),
          .settings     = library.at(                         "settings" ).dump(),
          .createdAt    = abscli::utils::json::value(library, "createdAt",     0),
          .lastUpdate   = abscli::utils::json::value(library, "lastUpdate",    0)
        });
      }
    } catch (const std::exception& e) {
      std::cerr << "Failed to parse library from API response: " << e.what() << "\n";
      return;
    }
    m_db.updateLibrariesTable(libraries);
  }
}

auto AppController::requestData(const std::string& endpoint, const std::string& responseContains) -> std::optional<json> {
  try {
    json response;
    response = abscli::http::getRequest(m_serverUrl, endpoint, m_accessToken);
    if (response.contains(responseContains)) {
      std::cout << "Successful API request for " << m_serverUrl + endpoint << "\n";
      return response;
    }
  } catch (const std::runtime_error& e) {
    std::cerr << "\033[1;31m[ERROR]\033[0m while requesting data: " << e.what() << "\n";
 }
  return std::nullopt;
}

auto AppController::listLibraries() -> void {
  for (auto& library : m_db.getLibrariesNames()) {
    std::cout << library << "\n";
  }
}

auto AppController::listItems(const std::vector<std::string>& vec) -> void {
  int index = 0;
  for (const auto& item : vec) {
    std::cout << "[" << index++ << "] " <<  item << "\n";
  }
}
