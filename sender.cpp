#include <iostream>

#include "sender.hpp"

using namespace std;

static bool ParseLine(const string line, INPUT& msg);

DWORD WINAPI SenderThreadFunc(LPVOID param) {

    string line;
    while(getline(cin, line)) {
        cout << "\nSender got " << line << endl;
        INPUT msg;
        if (!ParseLine(line, msg)) {

        }
        Sleep(100);
    }

    cout << "\nSender ended\n";

}

static bool ParseLine(const string line, INPUT& msg) {
    memset(&msg, 0, sizeof msg);
    return true;
}