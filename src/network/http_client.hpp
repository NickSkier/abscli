#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace abscli::http {
  auto getRequest(const std::string& hostUrl,
                  const std::string& endpoint,
                  const std::string& token) -> json;

  auto postRequest(const std::string& hostUrl,
                   const std::string& endpoint,
                   const std::string& payloadOrToken,
                   bool isRefresh = false) -> json;

  auto pingServer(const std::string& serverUrl) -> bool;
}
