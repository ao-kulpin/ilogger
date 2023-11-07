#include <iostream>
#include <Windows.h>

#include "sender.hpp"
#include "option.hpp"

Option option;

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_KEYDOWN) {
            KBDLLHOOKSTRUCT* kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
            DWORD keyCode = kbdStruct->vkCode;
            std::cout << "Key press: " << keyCode << std::endl;
        }
        else if (wParam == WM_KEYUP) {
            KBDLLHOOKSTRUCT* kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
            DWORD keyCode = kbdStruct->vkCode;
            std::cout << "Key release: " << keyCode << std::endl;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static int wheelDelta = 0;

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {

    if (nCode >= 0) {
        if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN) {
            switch (wParam) {
            case WM_LBUTTONDOWN:
                std::cout << "Mouse button press: LEFT" << std::endl;
                break;
            case WM_RBUTTONDOWN:
                std::cout << "Mouse button press: RIGHT" << std::endl;
                break;
            case WM_MBUTTONDOWN:
                std::cout << "Mouse button press: MIDDLE" << std::endl;
                break;
            }
        }
        else if (wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP || wParam == WM_MBUTTONUP) {
            switch (wParam) {
            case WM_LBUTTONUP:
                std::cout << "Mouse button release: LEFT" << std::endl;
                break;
            case WM_RBUTTONUP:
                std::cout << "Mouse button release: RIGHT" << std::endl;
                break;
            case WM_MBUTTONUP:
                std::cout << "Mouse button release: MIDDLE" << std::endl;
                break;
            }
        }
        else if (wParam == WM_MOUSEWHEEL) {
            MSLLHOOKSTRUCT* pMhs = (MSLLHOOKSTRUCT*)lParam;
            short zDelta = HIWORD(pMhs->mouseData);
            if(zDelta>0)
                std::cout << "Mouse wheel: UP" << std::endl;
            if(zDelta<0)
                std::cout << "Mouse wheel: DOWN" << std::endl;
        }
        else if (wParam == WM_MOUSEMOVE) {
            MSLLHOOKSTRUCT* mouseInfo = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
            int x = mouseInfo->pt.x;
            int y = mouseInfo->pt.y;
            std::cout << "Mouse move: X=" << x << ", Y=" << y << std::endl;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
    LPSTR lpszCmdLine, int nCmdShow) {
std::cerr << "cmdLine: \"" << lpszCmdLine << "\"\n";
    if (!option.acceptArgs(lpszCmdLine)) {
        std::cerr << "\n*** Invalid arguments: \"" << lpszCmdLine << "\"\n";
        return 1;
    }
std::cerr << "--skip " << option.skip() << std::endl;
    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
    HHOOK mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, GetModuleHandle(NULL), 0);

    DWORD senderId;
    auto senderHandle = CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            SenderThreadFunc,       // thread function name
            0,                      // argument to thread function 
            0,                      // use default creation flags 
            &senderId);             // returns the thread identifier 

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(keyboardHook);
    UnhookWindowsHookEx(mouseHook);

    return 0;
}
