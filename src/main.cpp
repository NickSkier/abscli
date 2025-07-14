#include <iostream>
#include "services/app_controller.hpp"

int main(int argc, char **argv) {
  if(argc != 1) {
    std::cout << argv[0] <<  "takes no arguments.\n";
    return 1;
  }

  AppController appController;
  bool appLoop = true;
  while (appLoop) {
    std::cout << "\nLogin          (1)\n"
                   "Sync user data (2)\n"
                   "Sync libraries (3)\n"
                   "List libraries (4)\n"
                   "Exit           (0)\n"
                   "Enter: ";
    int choice;
    std::cin >> choice;
    std::cout << "-------------------\n";
    switch (choice) {
    case 1: appController.login(); break;
    case 2: appController.syncUserData(); break;
    case 3: appController.syncLibraries(); break;
    case 4: appController.listLibraries(); break;
    case 0: appLoop = false; break;
    default: std::cout << "Wrong number\n"; break;
    }
  }
  return 0;
}
