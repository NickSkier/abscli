#pragma once

#include <string>

class AppController {
public:
  void login();

private:
  std::string m_serverUrl;
  std::string m_accessToken;
};

