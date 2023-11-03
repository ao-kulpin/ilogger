#include <iostream>

#include "sender.hpp"

using namespace std;

static bool ParseLine(const string line, INPUT& msg);

DWORD WINAPI SenderThreadFunc(LPVOID param) {

    string line;
    while(getline(cin, line)) {
        INPUT msg;
        if (ParseLine(line, msg)) {
            if (SendInput(1, &msg, sizeof msg) != 1) {
                cerr << "\n*** Can't send message\"" << line << "\" system error: " << GetLastError() << endl << endl;
            }
        } else 
            cerr << "\n*** invalid input message \"" << line << "\"\n";
        Sleep(100);
    }

    cerr << "\n*** Sender ended\n";
}

static bool ParseLine(const string line, INPUT& msg) {
    memset(&msg, 0, sizeof msg);
    auto& mi = msg.mi;
    auto& ki = msg.ki;

    const char* linePtr = line.c_str();

    char mousePressStr  [] = "Mouse button press: ";
    char mouseReleaseStr[] = "Mouse button release: ";
    char mouseWheelStr  [] = "Mouse Wheel: ";
    char mouseMoveStr   [] = "Mouse move: ";
    char keyPressStr    [] = "Key press: ";
    char keyReleaseStr  [] = "Key release: ";

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
        mi.dwFlags = MOUSEEVENTF_WHEEL;
        mouseWheelPtr += sizeof (mouseWheelStr) - 1;
        if (strstr(mouseWheelPtr, " UP")) {
            mi.mouseData = WHEEL_DELTA;
            return true;
        } else if (strstr(mouseWheelPtr, " DOWN")) {
            mi.mouseData = -WHEEL_DELTA;
            return true;
        } else
            return false;
    } else if (auto mouseMovePtr = strstr(linePtr, mouseMoveStr)) {
        msg.type = INPUT_MOUSE;
        mi.dwFlags = MOUSEEVENTF_MOVE;

        mouseMovePtr += sizeof (mouseMoveStr) - 1;
        if (auto xPtr = strstr(mouseMovePtr, "X=")) {
            xPtr += 2; // length of "X="
            char* endPtr = 0;
            mi.dx = strtol(xPtr, &endPtr, 10);
            if (xPtr == endPtr)
                // no number found
                return false;
            if (auto yPtr = strstr(endPtr, "Y=")) {
                yPtr += 2; // length of "Y="
                endPtr = 0;
                mi.dy = strtol(yPtr, &endPtr, 10);
                if (yPtr == endPtr)
                    // no number found
                    return false;
                return true;                    
            } else
                return false;

            } else
                return false;             

    } else if (auto keyPressPtr = strstr(linePtr, keyPressStr)) {
        msg.type = INPUT_KEYBOARD;
        keyPressPtr += sizeof(keyPressStr) - 1;
        char* endPtr = 0;
        ki.wVk = strtol(keyPressPtr, &endPtr, 10);
        if (keyPressPtr == endPtr) 
            // no number found
            return false;
        return true;            
    } else if (auto keyReleasePtr = strstr(linePtr, keyReleaseStr)) {
        msg.type = INPUT_KEYBOARD;
        ki.dwFlags = KEYEVENTF_KEYUP;
        keyReleasePtr += sizeof(keyReleaseStr) - 1;
        char* endPtr = 0;
        ki.wVk = strtol(keyReleasePtr, &endPtr, 10);
        if (keyReleasePtr == endPtr) 
            // no number found
            return false;
        return true;            
    } else
        // unknown message
        return false;
 }