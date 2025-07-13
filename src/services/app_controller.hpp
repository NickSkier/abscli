#pragma once

#include <string>
#include "../db/db_manager.hpp"

class AppController {
public:
  AppController();

  auto login() -> bool;

private:
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
  std::string m_userId;
  std::string m_serverUrl;
  std::string m_accessToken;
};

