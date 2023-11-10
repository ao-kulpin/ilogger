#include <iostream>

#include "sender.hpp"
#include "mkinput.hpp"

using namespace std;

static bool ParseLine(const string line, MKInput& mki);

class CoordConvertor {
private:
    LONG _xVirtScr  = GetSystemMetrics(SM_XVIRTUALSCREEN);
    LONG _yVirtScr  = GetSystemMetrics(SM_YVIRTUALSCREEN);
    LONG _cxVirtScr = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    LONG _cyVirtScr = GetSystemMetrics(SM_CYVIRTUALSCREEN);
public:
    CoordConvertor() {}
    
    LONG toAbsoluteX(LONG x) {
        if (x < _xVirtScr)
            x = _xVirtScr;

        if (x >= _cxVirtScr)
            x = _cxVirtScr - 1;

        return MulDiv(65535, x - _xVirtScr, _cxVirtScr - 1);            
    }

    LONG toAbsoluteY(LONG y) {
        if (y < _yVirtScr)
            y = _yVirtScr;

        if (y >= _cyVirtScr)
            y = _cyVirtScr - 1;

        return MulDiv(65535, y - _yVirtScr, _cyVirtScr - 1);            
    }
};

static CoordConvertor convertor;

DWORD WINAPI SenderThreadFunc(LPVOID param) {

    string line;
    while(getline(cin, line)) {
        MKInput mki;
        if (ParseLine(line, mki)) {
#ifdef __WINDOWS__            
            INPUT wi;
            mki.toWin(wi);
            if (SendInput(1, &wi, sizeof wi) != 1) {
                cerr << "\n*** Can't send message\"" << line << "\" system error: " << GetLastError() << endl << endl;
            }
#endif // __WINDOWS__            
        } else 
            cerr << "\n*** invalid input message \"" << line << "\"\n";
        // Sleep(100);
    }

    cerr << "\n*** Sender ended\n";
    return 0;
}

#define MOUSEEVENTF_VIRTUALDESK 0x4000

static const double xFactor = 65535.0 / (GetSystemMetrics(SM_CXSCREEN) - 1);
static const double yFactor = 65535.0 / (GetSystemMetrics(SM_CYSCREEN) - 1);

static bool ParseLine(const string line, MKInput& mki) {
    const char* linePtr = line.c_str();

    static const char mousePressStr  [] = "Mouse button press: ";
    static const char mouseReleaseStr[] = "Mouse button release: ";
    static const char mouseWheelStr  [] = "Mouse wheel: ";
    static const char mouseMoveStr   [] = "Mouse move: ";
    static const char keyPressStr    [] = "Key press: ";
    static const char keyReleaseStr  [] = "Key release: ";

    if (auto mousePressPtr = strstr(linePtr, mousePressStr)) {
        mousePressPtr += sizeof (mousePressStr) - 1;
        if (strstr(mousePressPtr, "LEFT")) {
            mki = MKInput(MInput(MInput::Action::press, MInput::Button::left));
            return true;
        } else if (strstr(mousePressPtr, "RIGHT")) {
            mki = MKInput(MInput(MInput::Action::press, MInput::Button::right));
            return true;
        } else if (strstr(mousePressPtr, "MIDDLE")) {
            mki = MKInput(MInput(MInput::Action::press, MInput::Button::middle));
            return true;
        } else
            return false;
    }
    else if (auto mouseReleasePtr = strstr(linePtr, mouseReleaseStr)) {
        mouseReleasePtr += sizeof (mouseReleaseStr) - 1;
        if (strstr(mouseReleasePtr, "LEFT")) {
            mki = MKInput(MInput(MInput::Action::release, MInput::Button::left));
            return true;
        } else if (strstr(mouseReleasePtr, "RIGHT")) {
            mki = MKInput(MInput(MInput::Action::release, MInput::Button::right));
            return true;
        } else if (strstr(mouseReleasePtr, "MIDDLE")) {
            mki = MKInput(MInput(MInput::Action::release, MInput::Button::middle));
            return true;
        } else
            return false;
    } else if (auto mouseWheelPtr = strstr(linePtr, mouseWheelStr)) {
        mouseWheelPtr += sizeof (mouseWheelStr) - 1;
        if (strstr(mouseWheelPtr, "UP")) {
            mki = MKInput(MInput(MInput::Action::wheel, MInput::Button::none, true));
            return true;
        } else if (strstr(mouseWheelPtr, "DOWN")) {
            mki = MKInput(MInput(MInput::Action::wheel, MInput::Button::none, false));
            return true;
        } else
            return false;
    } else if (auto mouseMovePtr = strstr(linePtr, mouseMoveStr)) {
        mouseMovePtr += sizeof (mouseMoveStr) - 1;
        if (auto xPtr = strstr(mouseMovePtr, "X=")) {
            xPtr += 2; // length of "X="
            char* endPtr = 0;
            auto dx = convertor.toAbsoluteX(strtol(xPtr, &endPtr, 10));
            if (xPtr == endPtr)
                // no number found
                return false;
            if (auto yPtr = strstr(endPtr, "Y=")) {
                yPtr += 2; // length of "Y="
                endPtr = 0;
                auto dy = convertor.toAbsoluteY(strtol(yPtr, &endPtr, 10));
                if (yPtr == endPtr)
                    // no number found
                    return false;
                mki = MKInput(MInput(MInput::Action::move, MInput::Button::none, false, dx, dy));
                return true;                    
            } else
                return false;

            } else
                return false;             

    } else if (auto keyPressPtr = strstr(linePtr, keyPressStr)) {
        keyPressPtr += sizeof(keyPressStr) - 1;
        char* endPtr = 0;
        auto vk = strtol(keyPressPtr, &endPtr, 10);
        if (keyPressPtr == endPtr) 
            // no number found
            return false;
        mki = MKInput(KInput(KInput::Action::press, vk));
        return true;            
    } else if (auto keyReleasePtr = strstr(linePtr, keyReleaseStr)) {
        keyReleasePtr += sizeof(keyReleaseStr) - 1;
        char* endPtr = 0;
        auto vk = strtol(keyReleasePtr, &endPtr, 10);
        if (keyReleasePtr == endPtr) 
            // no number found
            return false;
        mki = MKInput(KInput(KInput::Action::release, vk));
        return true;            
    } else
        // unknown message
        return false;
 }