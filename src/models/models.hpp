#pragma once

#include <string>
#include <cstdint>

namespace abscli::models {
  struct User {
    std::string   id;
    std::string   username;
    std::string   absServer;
    std::string   email;
    std::string   type;
    std::string   seriesHideFromContinueListening;
    std::string   bookmarks;
    bool          isActive;
    bool          isLocked;
    std::int64_t  lastSeen;
    std::int64_t  createdAt;
    std::string   permissions;
    std::string   librariesAccessible;
    std::string   itemTagsSelected;
    std::string   accessToken;
  };

  struct Library {
    std::string   id;
    std::string   name;
    std::int32_t  displayOrder;
    std::string   icon;
    std::string   mediaType;
    std::string   settings;
    std::int64_t  createdAt;
    std::int64_t  lastUpdate;
  };

  struct LibraryItem {
    std::string   id;
    std::string   ino;
    std::string   libraryId;
    std::string   folderId;
    std::string   path;
    std::string   relPath;
    bool          isFile;
    std::int64_t  mtimeMs;
    std::int64_t  ctimeMs;
    std::int64_t  birthtimeMs;
    std::int64_t  addedAt;
    std::int64_t  updatedAt;
    bool          isMissing;
    bool          isInvalid;
    std::string   mediaType;
    std::int32_t  numFiles;
    std::int64_t  size;
  };
}
