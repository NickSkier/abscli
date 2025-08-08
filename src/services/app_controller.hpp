#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "token_storage.hpp"
#include "../db/db_manager.hpp"
#include "src/models/models.hpp"

using json = nlohmann::json;

class AppController {
public:
  AppController();

  void requestNewTokens();
  auto login() -> bool;
  void syncUserData();
  void syncLibraries();
  void syncLibrariesItems();
  auto parseBook(const json& itemMedia) -> abscli::models::Book;

  void listLibraries();
  void listLibraryItems();
  void listBooks();

  static void listItems(const std::vector<std::string>& vec);
  static void listItems(const std::vector<std::vector<std::string>>& vec);

private:
  auto requestData(const std::string& endpoint, const std::string& responseContains) -> std::optional<json>;

  abscli::db::DbManager m_db;
  abscli::TokenStorage m_refreshTokenStorage;
  std::string m_userId;
  std::string m_serverUrl;
  std::string m_accessToken;
};
