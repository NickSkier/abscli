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

  struct Book {
    std::string   id;
    std::string   title;
    std::string   titleIgnorePrefix;
    std::string   subtitle;
    std::string   authorName;
    std::string   authorNameLF;
    std::string   narratorName;
    std::string   seriesName;
    std::string   genres;
    std::string   publishedYear;
    std::string   publishedDate;
    std::string   publisher;
    std::string   description;
    std::string   isbn;
    std::string   asin;
    std::string   language;
    // bool          explicit;
    bool          abridged;
    std::string   coverPath;
    std::string   tags;
    std::int32_t  numTracks;
    std::int32_t  numAudioFiles;
    std::int32_t  numChapters;
    float         duration;
    std::int64_t  size;
    std::string   ebookFormat;
  };
}
