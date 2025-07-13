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
}
