#pragma once

#include <string>
#include "../db/db_manager.hpp"

class AppController {
public:
  AppController();

  auto login() -> bool;

private:
  abscli::db::DbManager m_db;
  std::string m_userId;
  std::string m_serverUrl;
  std::string m_accessToken;
};

