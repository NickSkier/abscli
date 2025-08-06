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
  auto usernames = m_db.getColumnValuesFromTable("username", "users");
  auto serversUrls = m_db.getColumnValuesFromTable("absServer", "users");
  auto userIds = m_db.getColumnValuesFromTable("id", "users");
  if (usernames.empty()) { login(); }
  else {
    listItems({ usernames, serversUrls });
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

auto AppController::syncLibrariesItems() -> void {
  auto librariesIds = m_db.getColumnValuesFromTable("id", "libraries");
  auto librariesMediaType = m_db.getColumnValuesFromTable("mediaType", "libraries");

  std::vector<std::vector<abscli::models::LibraryItem>> libraryItems;
  std::vector<abscli::models::Book> books;

  for (size_t libIndex = 0; libIndex < librariesIds.size(); libIndex++) {
    const std::optional<json>& response = requestData("/api/libraries/" + librariesIds[libIndex] + "/items", "results");
    if (response) {
      const json& responseData = response.value()["results"];
      libraryItems.emplace_back();
      try {
        for (const auto& libraryItem : responseData) {
          libraryItems[libIndex].emplace_back(abscli::models::LibraryItem{
            .id          = abscli::utils::json::value(libraryItem, "id"          , ""),
            .ino         = abscli::utils::json::value(libraryItem, "ino"         , ""),
            .libraryId   = abscli::utils::json::value(libraryItem, "libraryId"   , ""),
            .folderId    = abscli::utils::json::value(libraryItem, "folderId"    , ""),
            .path        = abscli::utils::json::value(libraryItem, "path"        , ""),
            .relPath     = abscli::utils::json::value(libraryItem, "relPath"     , ""),
            .isFile      = abscli::utils::json::value(libraryItem, "isFile"      , false),
            .mtimeMs     = abscli::utils::json::value(libraryItem, "mtimeMs"     , 0),
            .ctimeMs     = abscli::utils::json::value(libraryItem, "ctimeMs"     , 0),
            .birthtimeMs = abscli::utils::json::value(libraryItem, "birthtimeMs" , 0),
            .addedAt     = abscli::utils::json::value(libraryItem, "addedAt"     , 0),
            .updatedAt   = abscli::utils::json::value(libraryItem, "updatedAt"   , 0),
            .isMissing   = abscli::utils::json::value(libraryItem, "isMissing"   , false),
            .isInvalid   = abscli::utils::json::value(libraryItem, "isInvalid"   , false),
            .mediaType   = abscli::utils::json::value(libraryItem, "mediaType"   , ""),
            .numFiles    = abscli::utils::json::value(libraryItem, "numFiles"    , 0),
            .size        = abscli::utils::json::value(libraryItem, "size"        , 0)
          });
          if (librariesMediaType[libIndex] == "book"
              && libraryItem.contains("media")
              && libraryItem["media"].contains("metadata")
             ) {
            const json& itemMedia = libraryItem["media"];
            const auto& itemMetadata = itemMedia["metadata"];
            books.emplace_back(abscli::models::Book{
              .id                = abscli::utils::json::value(itemMedia,    "id"              , ""),
              .title             = abscli::utils::json::value(itemMetadata, "title"            , ""),
              .titleIgnorePrefix = abscli::utils::json::value(itemMetadata, "titleIgnorePrefix", ""),
              .subtitle          = abscli::utils::json::value(itemMetadata, "subtitle"         , ""),
              .authorName        = abscli::utils::json::value(itemMetadata, "authorName"       , ""),
              .authorNameLF      = abscli::utils::json::value(itemMetadata, "authorNameLF"     , ""),
              .narratorName      = abscli::utils::json::value(itemMetadata, "narratorName"     , ""),
              .seriesName        = abscli::utils::json::value(itemMetadata, "seriesName"       , ""),
              .genres            =                            itemMetadata.at("genres").dump(),
              .publishedYear     = abscli::utils::json::value(itemMetadata, "publishedYear"    , ""),
              .publishedDate     = abscli::utils::json::value(itemMetadata, "publishedDate"    , ""),
              .publisher         = abscli::utils::json::value(itemMetadata, "publisher"        , ""),
              .description       = abscli::utils::json::value(itemMetadata, "description"      , ""),
              .isbn              = abscli::utils::json::value(itemMetadata, "isbn"             , ""),
              .asin              = abscli::utils::json::value(itemMetadata, "asin"             , ""),
              .language          = abscli::utils::json::value(itemMetadata, "language"         , ""),
              .abridged          = abscli::utils::json::value(itemMetadata, "abridged"         , false),
              .coverPath         = abscli::utils::json::value(itemMedia,    "coverPath"       , ""),
              .tags              =                            itemMedia.at("tags").dump(),
              .numTracks         = abscli::utils::json::value(itemMedia,    "numTracks"       , 0),
              .numAudioFiles     = abscli::utils::json::value(itemMedia,    "numAudioFiles"   , 0),
              .numChapters       = abscli::utils::json::value(itemMedia,    "numChapters"     , 0),
              .duration          = abscli::utils::json::value(itemMedia,    "duration"        , 0.0f),
              .size              = abscli::utils::json::value(itemMedia,    "size"            , 0),
              .ebookFormat       = abscli::utils::json::value(itemMedia,    "ebookFormat"     , "")
            });
          }
        }
      } catch (const std::exception& e) {
        std::cerr << "Failed to parse library from API response: " << e.what() << "\n";
        return;
      }
    }
  }
  m_db.updateLibraryItemsTable(libraryItems);
  m_db.updateBooksTable(books);
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
  auto librariesNames = m_db.getColumnValuesFromTable("name", "libraries");
  auto librariesTypes = m_db.getColumnValuesFromTable("mediaType", "libraries");
  listItems({ librariesNames, librariesTypes });
}

auto AppController::listLibraryItems() -> void {
  auto libraryItemTitle     = m_db.getColumnValuesFromTable("relPath", "libraryItems");
  auto libraryItemMediaType = m_db.getColumnValuesFromTable("mediaType", "libraryItems");
  listItems({ libraryItemTitle, libraryItemMediaType });
}

auto AppController::listItems(const std::vector<std::string>& vec) -> void {
  int index = 0;
  for (const auto& item : vec) {
    std::cout << index++ << " " << item << "\n";
  }
}

auto AppController::listItems(const std::vector<std::vector<std::string>>& vec2D) -> void {
  for (size_t i = 0; i < vec2D[0].size(); ++i) {
    std::cout << i << " ";
    for (const auto& vec : vec2D) {
      std::cout << vec[i] << " ";
    }
    std::cout << "\n";
  }
}
