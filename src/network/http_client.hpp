#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace abscli::http {
  auto postRequest(const std::string& hostUrl,
                   const std::string& endpoint,
                   const std::string& payload) -> json;

  auto pingServer(const std::string& serverUrl) -> bool;
}
