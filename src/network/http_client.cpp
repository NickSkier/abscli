#include <iostream>
#include <string>
#include <cpr/cpr.h>
#include <cpr/response.h>
#include <cpr/timeout.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

auto pingServer() -> bool {
  std::cout << "Enter server URL: ";
  std::string serverUrl;
  std::cin >> serverUrl;
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
