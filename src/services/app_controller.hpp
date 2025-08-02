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

  abscli::db::DbManager m_db;
  abscli::TokenStorage m_refreshTokenStorage;
  std::string m_userId;
  std::string m_serverUrl;
  std::string m_accessToken;
};
