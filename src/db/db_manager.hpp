#pragma once

#include <string>
#include <sqlite3.h>
#include <nlohmann/json.hpp>
#include "../models/models.hpp"

using json = nlohmann::json;

namespace abscli::db {
  class DbManager {
  public:
    DbManager(const std::string& dbDir, const std::string& dbFilename);
    ~DbManager();

    auto getColumnValuesFromTable(std::string columnName, std::string tableName) const -> std::vector<std::string>;

    auto getUserColumnValue(const std::string& username, const std::string& columnName) -> std::optional<std::string>;

    void updateUsersTableAfterLogin(const abscli::models::User& user);
    void updateUsersTable(const abscli::models::User& user);
    void updateLibrariesTable(const std::vector<abscli::models::Library>& libaries);
    void updateLibraryItemsTable(const std::vector<std::vector<abscli::models::LibraryItem>>& libaryItems);
    void updateBooksTable(const std::vector<abscli::models::Book>& books);

  private:
    auto openDB(const std::string& dbPath, const std::string& dbFilename) -> int;
    auto initDB() -> int;

    sqlite3* m_absclidb;
  };
}
