#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "token_storage.hpp"
#include "../db/db_manager.hpp"

using json = nlohmann::json;

class AppController {
public:
  AppController();

  auto login() -> bool;
  void syncUserData();
  void syncLibraries();

  void listLibraries();

private:
  auto requestData(const std::string& endpoint, const std::string& responseContains) -> std::optional<json>;

  template<typename T>
  auto getJsonValue(const json& jsonObj, const std::string& key, const T& defaultReturn) -> T {
    if (jsonObj.contains(key) && !jsonObj[key].is_null()) {
      return jsonObj.at(key).get<T>();
    }
    return defaultReturn;
  }

  auto getJsonValue(const json& jsonObj, const std::string& key, const char* defaultReturn) -> std::string {
    return getJsonValue<std::string>(jsonObj, key, std::string(defaultReturn));
  }

  abscli::db::DbManager m_db;
  abscli::TokenStorage m_refreshTokenStorage;
  std::string m_userId;
  std::string m_serverUrl;
  std::string m_accessToken;
};
