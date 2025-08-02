#pragma once

#include <nlohmann/json.hpp>

namespace abscli::utils::json {
  template<typename T>
  auto value(const nlohmann::json& jsonObj, const std::string& key, const T& defaultReturn) -> T {
    if (jsonObj.contains(key) && !jsonObj[key].is_null()) {
      return jsonObj.at(key).get<T>();
    }
    return defaultReturn;
  }

  inline auto value(const nlohmann::json& jsonObj, const std::string& key, const char* defaultReturn) -> std::string {
    return value<std::string>(jsonObj, key, std::string(defaultReturn));
  }
}
