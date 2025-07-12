#include <iostream>
#include <string>
#include <cpr/cpr.h>
#include <cpr/response.h>
#include <cpr/timeout.h>
#include <nlohmann/json.hpp>
#include "http_client.hpp"

using json = nlohmann::json;

namespace HttpClient {
  auto postRequest(const std::string& hostUrl,
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

    return json::parse(response.text);
  }

  auto pingServer(const std::string& serverUrl) -> bool {
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
}
