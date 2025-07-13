#include <iostream>
#include <string>
#include <cpr/cpr.h>
#include <cpr/response.h>
#include <cpr/timeout.h>
#include <nlohmann/json.hpp>
#include "http_client.hpp"

using json = nlohmann::json;

namespace {
  auto validateResponse(const cpr::Response& httpResponse) -> json {
    json response;
    if (httpResponse.status_code >= 200 && httpResponse.status_code < 300) {
      if (httpResponse.text.empty()) { return json{}; }
      std::cout << "Successful request!\n" << "\n";
      try {
        return json::parse(httpResponse.text);
      } catch (const json::parse_error& e) {
        throw std::runtime_error("Failed to parse JSON response: " + std::string(e.what()));
      }
    }
    std::string errorMessage = "[ERROR] Request for URL " + std::string(httpResponse.url)
                               + " failed with status code: " +
                               std::to_string(httpResponse.status_code) +
                               ". Body: [" + std::string(httpResponse.text) + "]";
    throw std::runtime_error(errorMessage);
    return json{};
  }
}

auto abscli::http::getRequest(const std::string& hostUrl,
                            const std::string& endpoint,
                            const std::string& token) -> json {
  std::cout << hostUrl + endpoint << "\n";
  cpr::Header header = {{"Authorization", "Bearer " + token}};
  cpr::Response response = cpr::Get(
    cpr::Url(hostUrl + endpoint),
    cpr::Header(header)
  );
  return validateResponse(response);
}

auto abscli::http::postRequest(const std::string& hostUrl,
                             const std::string& endpoint,
                             const std::string& payload) -> json {
  std::cout << hostUrl + endpoint << "\n";

  cpr::Header headers;
  cpr::Body body;

  body = payload;
  headers = {{"Content-Type", "application/json"}};

  cpr::Response response = cpr::Post(
    cpr::Url(hostUrl + endpoint),
    body,
    headers
  );

  return validateResponse(response);
}

auto abscli::http::pingServer(const std::string& serverUrl) -> bool {
  cpr::Response pingResponse = cpr::Get(
                                cpr::Url{serverUrl + "/ping"},
                                cpr::Timeout(2000));

  std::cout << "Ping Status:\n";

  if (pingResponse.status_code == 200 && pingResponse.text != "") {
    json response = json::parse(pingResponse.text);
    if (response.value("success", false)) {
      std::cout << "Ping successfull!\n";
      return true;
    }
  }
  std::cerr << "[ERROR] Failed to check \"" + serverUrl + "\" status!\n";
  return false;
}
