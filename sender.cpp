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
    auto& mi = msg.mi;

    const char* linePtr = line.c_str();

    char mousePressStr  [] = "Mouse button press:";
    char mouseReleaseStr[] = "Mouse button release:";
    char mouseWheelStr[]   = "Mouse Wheel:";
    if (auto mousePressPtr = strstr(linePtr, mousePressStr)) {
        msg.type = INPUT_MOUSE;
        mousePressPtr += sizeof (mousePressStr) - 1;
        if (strstr(mousePressPtr, " LEFT")) {
            mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            return true;
        } else if (strstr(mousePressPtr, " RIGHT")) {
            mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
            return true;
        } else if (strstr(mousePressPtr, " MIDDLE")) {
            mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
            return true;
        } else
            return false;
    }
    else if (auto mouseReleasePtr = strstr(linePtr, mouseReleaseStr)) {
        msg.type = INPUT_MOUSE;
        mouseReleasePtr += sizeof (mouseReleaseStr) - 1;
        if (strstr(mouseReleasePtr, " LEFT")) {
            mi.dwFlags = MOUSEEVENTF_LEFTUP;
            return true;
        } else if (strstr(mouseReleasePtr, " RIGHT")) {
            mi.dwFlags = MOUSEEVENTF_RIGHTUP;
            return true;
        } else if (strstr(mouseReleasePtr, " MIDDLE")) {
            mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
            return true;
        } else
            return false;
    } else if (auto mouseWheelPtr = strstr(linePtr, mouseWheelStr)) {
        msg.type = INPUT_MOUSE;
        mouseWheelPtr += sizeof (mouseWheelStr) - 1;
        if (strstr(mouseWheelPtr, " UP")) {
            mi.mouseData = WHEEL_DELTA;
            return true;
        } else if (strstr(mouseWheelPtr, " DOWN")) {
            mi.mouseData = -WHEEL_DELTA;
            return true;
        } else
            return false;
    }
    

    return true;
}