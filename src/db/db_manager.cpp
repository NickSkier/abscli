#include <filesystem>
#include <iostream>
#include <string>
#include <set>
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

  const char* sqlCreateLibrariesTable = "CREATE TABLE IF NOT EXISTS libraries("
                                    "id UUID PRIMARY                 KEY,"
                                    "name                            VARCHAR(255),"
                                    "displayOrder                    INTEGER,"
                                    "icon                            VARCHAR(255),"
                                    "mediaType                       VARCHAR(255),"
                                    "settings                        JSON,"
                                    "createdAt                       DATETIME NOT NULL,"
                                    "lastUpdate                      DATETIME NOT NULL);";

  const char* sqlCreateLibraryItems = "CREATE TABLE IF NOT EXISTS libraryItems("
                                    "id UUID PRIMARY                 KEY,"
                                    "ino                             VARCHAR(255),"
                                    "libraryId UUID                  KEY,"
                                    "folderId UUID                   KEY,"
                                    "path                            VARCHAR(255),"
                                    "relPath                         VARCHAR(255),"
                                    "isFile                          TINYINT(1),"
                                    "mtimeMs                         DATETIME,"
                                    "ctimeMs                         DATETIME,"
                                    "birthtimeMs                     DATETIME,"
                                    "addedAt                         DATETIME,"
                                    "updatedAt                       DATETIME,"
                                    "isMissing                       TINYINT(1),"
                                    "isInvalid                       TINYINT(1),"
                                    "mediaType                       VARCHAR(255),"
                                    "numFiles                        INTEGER,"
                                    "size                            BIGINT);";

  int responseCode;
  responseCode = sqlite3_exec(m_absclidb, sqlCreateUsersTable,     nullptr, nullptr, nullptr);
  responseCode = sqlite3_exec(m_absclidb, sqlCreateLibrariesTable, nullptr, nullptr, nullptr);
  responseCode = sqlite3_exec(m_absclidb, sqlCreateLibraryItems,   nullptr, nullptr, nullptr);
  return responseCode;
}

auto abscli::db::DbManager::getColumnValuesFromTable(std::string columnName, std::string tableName) const -> std::vector<std::string> {
  std::vector<std::string> values;
  try {
    Statement stmt(m_absclidb, "SELECT " + columnName + " FROM " + tableName +  ";");
    while (stmt.step(m_absclidb)) {
      std::string value = stmt.get_column_text(0);
      values.emplace_back(value);
    }
  } catch (const std::exception& e) {
    std::cerr << "\033[1;31m[ERROR]\033[0m Failed while collecting [" + columnName + "] form [" + tableName + "]: " << e.what() << "\n";
  }
  return values;
}

auto abscli::db::DbManager::getUserColumnValue(const std::string& username, const std::string& columnName) -> std::optional<std::string> {
  std::array allowedColums = {"id", "username", "absServer", "accessToken"};
  if (std::ranges::find(allowedColums, columnName) == std::ranges::end(allowedColums)) {
    std::cerr << "\033[1;31mERROR:\033[0m Invalid or forbidden column name specified for search: [" << columnName  << "] in [users]\n";
    return std::nullopt;
  }
  try {
    std::string sql = "SELECT " + columnName + " FROM users WHERE username = ?;";
    Statement stmt(m_absclidb, sql);
    stmt.bind(1, username);
    if (stmt.step(m_absclidb)) {
      std::cout << "User's " + columnName + " found!\n";
      return stmt.get_column_text(0);
    }
    std::cout << "User's " + columnName + " not found!\n";
    return std::nullopt;
  } catch (const std::exception& e) {
    std::cerr << "\033[1;31mERROR:\033[0m Failed during user's " + columnName + " search: " << e.what() << "\n";
  }
  return std::nullopt;
}

void abscli::db::DbManager::updateUsersTableAfterLogin(const abscli::models::User& user) {
  try {
    abscli::db::Statement stmt(m_absclidb, "INSERT INTO users ("
                               "id, username, absServer, createdAt, accessToken) "
                               "VALUES (?, ?, ?, ?, ?) "
                               "ON CONFLICT(id) DO UPDATE SET "
                               "username    = excluded.username, "
                               "absServer   = excluded.absServer, "
                               "createdAt   = excluded.createdAt, "
                               "accessToken = excluded.accessToken;"
    );
    stmt.bind(1, user.id);
    stmt.bind(2, user.username);
    stmt.bind(3, user.absServer);
    stmt.bind(4, user.createdAt);
    stmt.bind(5, user.accessToken);
    stmt.step(m_absclidb);
  } catch (const std::exception& e) {
    std::cerr << "\033[1;31m[ERROR]\033[0m Failed to insert user " << user.id << " after login: " << e.what() << "\n";
  }
}

auto abscli::db::DbManager::updateUsersTable(const abscli::models::User& user) -> void {
  try {
    Statement stmt(m_absclidb, "INSERT INTO users ("
                               "id, username, absServer, "
                               "email, type, "
                               "seriesHideFromContinueListening, "
                               "bookmarks, isActive, isLocked, "
                               "lastSeen, createdAt, "
                               "permissions, librariesAccessible, "
                               "itemTagsSelected) "
                               "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
                               "ON CONFLICT(id) DO UPDATE SET "
                               "username            = excluded.username, "
                               "absServer           = excluded.absServer, "
                               "email               = excluded.email, "
                               "type                = excluded.type, "
                               "seriesHideFromContinueListening = excluded.seriesHideFromContinueListening, "
                               "bookmarks           = excluded.bookmarks, "
                               "isActive            = excluded.isActive, "
                               "isLocked            = excluded.isLocked, "
                               "lastSeen            = excluded.lastSeen, "
                               "createdAt           = excluded.createdAt, "
                               "permissions         = excluded.permissions, "
                               "librariesAccessible = excluded.librariesAccessible, "
                               "itemTagsSelected    = excluded.itemTagsSelected;"
    );
    stmt.bind(1, user.id);
    stmt.bind(2, user.username);
    stmt.bind(3, user.absServer);
    stmt.bind(4, user.email);
    stmt.bind(5, user.type);
    stmt.bind(6, user.seriesHideFromContinueListening);
    stmt.bind(7, user.bookmarks);
    stmt.bind(8, user.isActive);
    stmt.bind(9, user.isLocked);
    stmt.bind(10, user.lastSeen);
    stmt.bind(11, user.createdAt);
    stmt.bind(12, user.permissions);
    stmt.bind(13, user.librariesAccessible);
    stmt.bind(14, user.itemTagsSelected);
    stmt.step(m_absclidb);
  } catch (const std::exception& e) {
    std::cerr << "\033[1;31m[ERROR]\033[0m Failed to insert user " << user.id << ": " << e.what() << "\n";
  }
}

auto abscli::db::DbManager::updateLibrariesTable(const std::vector<abscli::models::Library>& libaries) -> void {
  std::set<std::string> jsonIds;
  std::string libraryId;
  try {
    Statement stmt(m_absclidb, "INSERT INTO libraries (id, name, displayOrder, "
                                                      "icon, mediaType, settings, "
                                                      "createdAt, lastUpdate) "
                                                "VALUES (?, ?, ?, ?, ?, ?, ?, ?) "
                                                "ON CONFLICT(id) DO UPDATE SET "
                                                "name         = excluded.name, "
                                                "displayOrder = excluded.displayOrder, "
                                                "icon         = excluded.icon, "
                                                "mediaType    = excluded.mediaType, "
                                                "settings     = excluded.settings, "
                                                "createdAt    = excluded.createdAt, "
                                                "lastUpdate   = excluded.lastUpdate;"
    );
    for (const auto& lib : libaries) {
      jsonIds.insert(lib.id);
      libraryId = lib.id;
      stmt.bind(1, lib.id);
      stmt.bind(2, lib.name);
      stmt.bind(3, lib.displayOrder);
      stmt.bind(4, lib.icon);
      stmt.bind(5, lib.mediaType);
      stmt.bind(6, lib.settings);
      stmt.bind(7, lib.createdAt);
      stmt.bind(8, lib.lastUpdate);
      stmt.step(m_absclidb);
      stmt.reset();
    }

    std::string deleteSql = "DELETE FROM libraries WHERE id NOT in (";
    std::string placeholders;
    for (size_t i = 0; i < jsonIds.size(); ++i) {
      placeholders += (i == 0 ? "?" : ", ?");
    }
    deleteSql += placeholders + ");";

    Statement deleteStmt(m_absclidb, deleteSql);

    int index = 1;
    for (const std::string& libId : jsonIds) {
      deleteStmt.bind(index++, libId);
    }
    deleteStmt.step(m_absclidb);

  } catch (const std::exception& e) {
    std::cerr << "\033[1;31m[ERROR]\033[0m Failed to insert library " << libraryId << ": " << e.what() << "\n";
  }
}

auto abscli::db::DbManager::updateLibraryItemsTable(const std::vector<std::vector<abscli::models::LibraryItem>>& libaryItems) -> void {
  std::set<std::string> jsonIds;
  std::string libraryItemId;
  try {
  Statement stmt(m_absclidb, "INSERT INTO libraryItems (id, ino, libraryId, folderId, "
                                                        "path, relPath, isFile, mtimeMs, "
                                                        "ctimeMs, birthtimeMs, addedAt, updatedAt, "
                                                        "isMissing, isInvalid, mediaType, numFiles, size) "
                                            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) " 
                                            "ON CONFLICT(id) DO UPDATE SET "
                                            "ino         = excluded.ino, "
                                            "libraryId   = excluded.libraryId, "
                                            "folderId    = excluded.folderId, "
                                            "path        = excluded.path, "
                                            "relPath     = excluded.relPath, "
                                            "isFile      = excluded.isFile, "
                                            "mtimeMs     = excluded.mtimeMs, "
                                            "ctimeMs     = excluded.ctimeMs, "
                                            "birthtimeMs = excluded.birthtimeMs, "
                                            "addedAt     = excluded.addedAt, "
                                            "updatedAt   = excluded.updatedAt, "
                                            "isMissing   = excluded.isMissing, "
                                            "isInvalid   = excluded.isInvalid, "
                                            "mediaType   = excluded.mediaType, "
                                            "numFiles    = excluded.numFiles, "
                                            "size        = excluded.size;"
    );
    for (const auto& library : libaryItems) {
      for (const auto& libItem : library) {
        std::cout << "Inserting " << libItem.relPath << "\n";
        jsonIds.insert(libItem.id);
        libraryItemId = libItem.id;
        stmt.bind(1,  libItem.id);
        stmt.bind(2,  libItem.ino);
        stmt.bind(3,  libItem.libraryId);
        stmt.bind(4,  libItem.folderId);
        stmt.bind(5,  libItem.path);
        stmt.bind(6,  libItem.relPath);
        stmt.bind(7,  libItem.isFile);
        stmt.bind(8,  libItem.mtimeMs);
        stmt.bind(9,  libItem.ctimeMs);
        stmt.bind(10, libItem.birthtimeMs);
        stmt.bind(11, libItem.addedAt);
        stmt.bind(12, libItem.updatedAt);
        stmt.bind(13, libItem.isMissing);
        stmt.bind(14, libItem.isInvalid);
        stmt.bind(15, libItem.mediaType);
        stmt.bind(16, libItem.numFiles);
        stmt.bind(17, libItem.size);
        stmt.step(m_absclidb);
        stmt.reset();
      }
    }

    std::string deleteSql = "DELETE FROM libraryItems WHERE id NOT in (";
    std::string placeholders;
    for (size_t i = 0; i < jsonIds.size(); ++i) {
      placeholders += (i == 0 ? "?" : ", ?");
    }
    deleteSql += placeholders + ");";

    Statement deleteStmt(m_absclidb, deleteSql);

    int index = 1;
    for (const std::string& libItemId : jsonIds) {
      deleteStmt.bind(index++, libItemId);
    }
    deleteStmt.step(m_absclidb);

  } catch (const std::exception& e) {
    std::cerr << "\033[1;31m[ERROR]\033[0m Failed to insert libraryItem " << libraryItemId << ": " << e.what() << "\n";
  }
}
