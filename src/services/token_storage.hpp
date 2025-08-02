#pragma once

#include <optional>
#include <string>
#include <keychain.h>

namespace abscli {
  class TokenStorage {
  public:
    TokenStorage(std::string package, std::string service);

    void deleteToken(const std::string& userId);
    void setToken(const std::string& userId, const std::string& token);
    auto getToken(const std::string& userId) -> std::optional<std::string>;

  private:
    keychain::Error m_error;
    const std::string m_package;
    const std::string m_service;
  };
}
