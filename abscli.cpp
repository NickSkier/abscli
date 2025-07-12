#include <iostream>
#include <string>
#include <cpr/cpr.h>
#include <cpr/response.h>
#include <cpr/timeout.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

auto pingServer() -> bool;

int main(int argc, char **argv) {
  if(argc != 1) {
    std::cout << argv[0] <<  "takes no arguments.\n";
    return 1;
  }

  bool appLoop = true;
  while (appLoop) {
    std::cout << "\nPing (1)\n"
                   "Exit (0)\n"
                   "Enter: ";
    int choice;
    std::cin >> choice;
    std::cout << "-------------------\n";
    switch (choice) {
    case 1: pingServer(); break;
    case 0: appLoop = false; break;
    default: std::cout << "Wrong number\n"; break;
    }
  }
  return 0;
}

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
