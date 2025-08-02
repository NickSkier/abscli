#include <iostream>
#include <optional>
#include <string>
#include <keychain.h>
#include "token_storage.hpp"

abscli::TokenStorage::TokenStorage(std::string package, std::string service) : m_package(std::move(package)), m_service(std::move(service)) { }

void abscli::TokenStorage::deleteToken(const std::string& userId) {
  keychain::deletePassword(m_package, m_service, userId, m_error);
  if (m_error) {
    std::cerr << m_error.message << "\n";
  }
}

void abscli::TokenStorage::setToken(const std::string& userId, const std::string& token) {
  keychain::setPassword(m_package, m_service, userId, token, m_error);
  if (m_error) {
    std::cerr << m_error.message << "\n";
  }
}

auto abscli::TokenStorage::getToken(const std::string& userId) -> std::optional<std::string> {
  std::string token = keychain::getPassword(m_package, m_service, userId, m_error);
  if (m_error.type == keychain::ErrorType::NotFound) {
    std::cerr << "\033[1;31m[ERROR]\033[0m Token not found for userId: " << userId << "\n";
    return std::nullopt;
  }
  else if (m_error) {
    std::cerr << m_error.message << "\n";
    return std::nullopt;
  }
  return token;
}
