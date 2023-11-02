#include <iostream>

#include "sender.hpp"

using namespace std;

DWORD WINAPI SenderThreadFunc(LPVOID param) {

    while(true) {
        cout << "Sender works ...\n";
        Sleep(1000);
    }
}