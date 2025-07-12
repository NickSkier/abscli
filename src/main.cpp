#include <iostream>
#include "network/http_client.hpp"

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
