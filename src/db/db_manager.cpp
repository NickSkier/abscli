#include <filesystem>
#include <iostream>
#include <string>
#include <sqlite3.h>
#include "statement.hpp"
#include "db_manager.hpp"

abscli::db::DbManager::DbManager(const std::string& dbDir, const std::string& dbFilename) {
  std::string dbPath = std::getenv("HOME") + std::string("/") + dbDir;
  openDB(dbPath, dbFilename);
  initDB();
}
abscli::db::DbManager::~DbManager() { sqlite3_close(m_absclidb); }

auto abscli::db::DbManager::openDB(const std::string& dbPath, const std::string& dbFilename) -> int {
  const std::filesystem::path dbDir = dbPath;
  const std::string dbFilePath = dbPath + "/" + dbFilename;

  if (!std::filesystem::exists(dbFilePath)) {
    std::cerr << "Database file not found: " << dbFilePath << "\n";
    std::filesystem::create_directories(dbDir);
  }

  int responseCode = sqlite3_open(dbFilePath.c_str(), &m_absclidb);

  if (responseCode != SQLITE_OK) {
    std::cerr << "Failed to open database: " << sqlite3_errmsg(m_absclidb) << "\n";
    return responseCode;
  }
  std::cout << "Database opened\n";
  
  return responseCode;
}

auto abscli::db::DbManager::initDB() -> int {
  const char* sqlCreateUsersTable = "CREATE TABLE IF NOT EXISTS users("
                                    "id UUID PRIMARY                 KEY,"
                                    "username                        VARCHAR(255),"
                                    "absServer                       VARCHAR(255),"
                                    "email                           VARCHAR(255),"
                                    "type                            VARCHAR(255),"
                                    "seriesHideFromContinueListening TINYINT(1),"
                                    "bookmarks                       JSON,"
                                    "isActive                        TINYINT(1),"
                                    "isLocked                        TINYINT(1),"
                                    "lastSeen                        DATETIME,"
                                    "createdAt                       DATETIME NOT NULL,"
                                    "permissions                     JSON,"
                                    "librariesAccessible             JSON,"
                                    "itemTagsSelected                JSON,"
                                    "accessToken                     VARCHAR(255));";

  int responseCode;
  responseCode = sqlite3_exec(m_absclidb, sqlCreateUsersTable, nullptr, nullptr, nullptr);
  return responseCode;
}

void abscli::db::DbManager::updateUsersTableAfterLogin(const abscli::models::User& user) {
  try {
    abscli::db::Statement stmt(m_absclidb, "INSERT INTO users ("
                               "id, username, absServer, createdAt, accessToken) "
                               "VALUES (?, ?, ?, ?, ?) "
                               "ON CONFLICT(id) DO UPDATE SET "
                               "username = excluded.username, "
                               "absServer = excluded.absServer, "
                               "createdAt = excluded.createdAt, "
                               "accessToken = excluded.accessToken;"
    );
    stmt.bind(1, user.id);
    stmt.bind(2, user.username);
    stmt.bind(3, user.absServer);
    stmt.bind(4, user.createdAt);
    stmt.bind(5, user.accessToken);
    stmt.step(m_absclidb);
  } catch (const std::exception& e) {
    std::cerr << "\033[1;31mError:\033[0m Failed to insert user " << user.id << " after login: " << e.what() << "\n";
  }
}
