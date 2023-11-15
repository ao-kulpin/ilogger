#include <iostream>
#include <Windows.h>
#include <fcntl.h>
#include <assert.h>
#include <thread>

#include "sender.hpp"
#include "option.hpp"
#include "astore.hpp"
#include "mkinput.hpp"
#include "mtrack.hpp"

using namespace std;

Option       option;
ActionStore  actionStore;
static
MouseTracker mtrack;

inline static void writePrefix(bool ownAction) { // write "own action prefix" to the cout
    if(ownAction && option.ownAction() == Option::OwnAction::highlight)
        cout << "==>";
}

class OutSkipper {
private:
    long long   _nextRunTime = 0;          // time when skipped inerval ends
    long        _skipInterval = 0;         // in 100-nanosecond units;     
public:
    void    start() {
        _skipInterval = option.skip() * 10000; // miliiseconds -> 100-nanosecond units
    }

    bool    isSkipped() {
        if (_skipInterval == 0)
            return false;

        SYSTEMTIME st;
        GetSystemTime(&st);

        FILETIME ft;
        SystemTimeToFileTime(&st, &ft);

        ULARGE_INTEGER uli;
        uli.u.LowPart = ft.dwLowDateTime;
        uli.u.HighPart = ft.dwHighDateTime;

        unsigned long long now = uli.QuadPart;

        if (now < _nextRunTime) 
            // skipping period
            return true;
        else {
            _nextRunTime = now + _skipInterval;
            return false;
        }
    }
};

static OutSkipper outSkipper;

class OutWriter {   // textual/binary output to stdout
private:    
    bool                _bin = false;
    Option::OwnAction   _act = Option::OwnAction::normal;
public:
    bool isBinary()     { return _bin; }    
    void start() {
        _bin = (option.ioformat() == Option::IOFormat::binary);
        _act = option.ownAction();
        if (_bin) {
            // reopen cout in binary mode
            _setmode(_fileno(stdout), _O_BINARY);
        }
    }

    template <class T>
    inline OutWriter& write(T& data, unsigned len) {
        // binary output
        assert(_bin);
        cout.write((const char*) &data, len);
        return *this;
    }

    template <class T>
    inline OutWriter& write(T& data) {
        // binary output
        return write(data, sizeof data);
    }

    void writeMKI(const MKInput& mki) {
        const bool ownAct = actionStore.check(mki, mtrack.getX(), mtrack.getY());
        if (ownAct && _act == Option::OwnAction::skip)
            // skipped with --ownacton skip
            return;

        writeActPrefix(ownAct);

        if (_bin)
            write(mki._type);  // 0 - mouse, 1 - keyboard

        switch(mki._type) {
            case MKInput::Type::mouse:
                writeMI(mki.mk._mi);
                break;

            case MKInput::Type::keyboard:
                writeKI(mki.mk._ki);
                break;

            default:
                assert(false);
                break;
        }
    }

    inline static 
    const char* buttonStr(MInput::Button b) {
        switch (b) {
            case MInput::Button::left:
                return "LEFT";

            case MInput::Button::middle:
                return "MIDDLE";

            case MInput::Button::right:
                return "RIGHT";

            default:
                return "";
        }
    }

    void writeMI(const MInput& mi) {
        if (_bin) {
            write(mi._action);
            switch(mi._action) {
                case MInput::Action::press:
                case MInput::Action::release:
                    write(mi._button);
                    break;

                case MInput::Action::wheel: {
                    unsigned char w = mi._wheelUp ? 1: 0;
                    write(w);
                }
                break;

                case MInput::Action::move: 
                    write (mi._dx);
                    write (mi._dy);
                    break;

                default:
                    assert(false);
                    break;
            }
        } else {
            switch(mi._action) {
                case MInput::Action::press:
                    cout << "Mouse button press: " << buttonStr(mi._button) << endl;
                    break;

                case MInput::Action::release:
                    cout << "Mouse button release: " << buttonStr(mi._button) << endl;
                    break;

                case MInput::Action::wheel:
                    if (mi._wheelUp) 
                        cout << "Mouse wheel: UP\n";
                    else
                        cout << "Mouse wheel: DOWN\n";
                    break;

                case MInput::Action::move:
                    cout << "Mouse move: X=" << mi._dx << ", Y=" << mi._dy << endl;
                    break;

                default:
                    assert(false);
                    cout << "Mouse unknown\n";
                    break;
            }                    
        }
    }

    void writeKI(const KInput& ki) {
        if (_bin) {
            write(ki._action);
            unsigned short vk = ki._vk;
            write(vk);
        } else {
            switch(ki._action) {
                case KInput::Action::press:
                    cout << "Key press: ";
                    break;

                case KInput::Action::release:
                    cout << "Key release: ";
                    break;

                default:
                    assert(false);
                    cout << "Key unknown: ";
                    break;
            }
            cout << ki._vk << endl;
        }
    }

    void writeActPrefix(bool ownAct) {
        if (_bin) {
            // binary prefix
            write("ilog", 4);

            unsigned char actByte = (ownAct ? 1: 0);
            write(actByte);

        } else if (ownAct && _act == Option::OwnAction::highlight)
            // textual prefix
            cout << "==>";
    }

    void binSignature() {
        assert(_bin);
        write("ilog", 4);
    }

    void actPrefix() {
        if (_act != Option::OwnAction::skip) {
            if (_bin) {

            }

        }


    }

    void binKey() {
        binSignature();
        static const auto k = MKInput::Type::keyboard; 
        write(k);
    }

    void binKeyPress(unsigned short vk) {
        binKey();
        static const auto act = KInput::Action::press;
        write(act);
        write(vk);
     ////   cerr << "Key press: " << vk << endl;
    }

    void keyRelease(unsigned short vk) {
        if (_act != Option::OwnAction::skip) {

        }
    }
    
    void binKeyRelease(unsigned short vk) {
        binKey();
        static const auto act = KInput::Action::release;
        write(act);
        write(vk);
    //// cerr << "Key release: " << vk << endl;
    }

    void binMouse() {
        binSignature();
        static const auto m = MKInput::Type::mouse; 
        write(m);
    }

    void binMousePress(const MInput::Button& button) {
        binMouse();
        static const auto p = MInput::Action::press;
        write(p);
        write(button);
    }

    void binMouseRelease(const MInput::Button& button) {
        binMouse();
        static const auto r = MInput::Action::release;
        write(r);
        write(button);
    }

    void binMouseWheel(bool wheelUp) {
        binMouse();
        static const auto w = MInput::Action::wheel;
        write(w);
        unsigned char u = wheelUp ? 1: 0;
        write(u);
    }

    void binMouseMove(int dx, int dy) {
        binMouse();
        static const auto m = MInput::Action::move;
        write(m);
        write(dx);
        write(dy);
    }
};

static OutWriter ow;

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && !outSkipper.isSkipped()) {
        if (wParam == WM_KEYDOWN) {
            KBDLLHOOKSTRUCT* kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
            DWORD keyCode = kbdStruct->vkCode;
            ow.writeMKI(MKInput(KInput(KInput::Action::press, keyCode)));
        }
        else if (wParam == WM_KEYUP) {
            KBDLLHOOKSTRUCT* kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
            DWORD keyCode = kbdStruct->vkCode;
            ow.writeMKI(MKInput(KInput(KInput::Action::release, keyCode)));
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static int wheelDelta = 0;

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {

    if (nCode >= 0  && !outSkipper.isSkipped()) {
        if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN) {
            switch (wParam) {
            case WM_LBUTTONDOWN:
                if (ow.isBinary())
                    ow.binMousePress(MInput::Button::left);
                else
                    cout << "Mouse button press: LEFT" << endl;
                break;

            case WM_RBUTTONDOWN:
                if (ow.isBinary())
                    ow.binMousePress(MInput::Button::right);
                else
                    cout << "Mouse button press: RIGHT" << endl;
                break;

            case WM_MBUTTONDOWN:
                if (ow.isBinary())
                    ow.binMousePress(MInput::Button::middle);
                else
                    cout << "Mouse button press: MIDDLE" << endl;
                break;

            }
        }
        else if (wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP || wParam == WM_MBUTTONUP) {
            switch (wParam) {
            case WM_LBUTTONUP:
                if (ow.isBinary())
                    ow.binMouseRelease(MInput::Button::left);
                else
                    cout << "Mouse button release: LEFT" << endl;
                break;

            case WM_RBUTTONUP:
                if (ow.isBinary())
                    ow.binMouseRelease(MInput::Button::right);
                else
                    cout << "Mouse button release: RIGHT" << endl;
                break;

            case WM_MBUTTONUP:
                if (ow.isBinary())
                    ow.binMouseRelease(MInput::Button::middle);
                else
                    cout << "Mouse button release: MIDDLE" << endl;
                break;
            }
        }
        else if (wParam == WM_MOUSEWHEEL) {
            MSLLHOOKSTRUCT* pMhs = (MSLLHOOKSTRUCT*)lParam;
            short zDelta = HIWORD(pMhs->mouseData);
            if (ow.isBinary())
                ow.binMouseWheel(zDelta > 0);
            else if(zDelta>0)
                cout << "Mouse wheel: UP" << endl;
            else if(zDelta<0)
                cout << "Mouse wheel: DOWN" << endl;
        }
        else if (wParam == WM_MOUSEMOVE) {
            MSLLHOOKSTRUCT* mouseInfo = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
            int x = mouseInfo->pt.x;
            int y = mouseInfo->pt.y;
            if (ow.isBinary())
                ow.binMouseMove(x, y);
            else
                cout << "Mouse move: X=" << x << ", Y=" << y << endl;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
    LPSTR lpszCmdLine, int nCmdShow) {
    if (!option.acceptArgs(lpszCmdLine)) {
        cerr << "\n*** Invalid arguments: \"" << lpszCmdLine << "\"\n";
        return 1;
    }
cerr << "--skip " << option.skip() << endl;
cerr << "--ioformat " << int(option.ioformat()) << endl;
cerr << "--ownaction " << int(option.ownAction()) << endl;

    ow.start();
    outSkipper.start();

    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(NULL), 0);
    HHOOK mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, GetModuleHandle(NULL), 0);

    thread senderThread(SenderThreadFunc);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(keyboardHook);
    UnhookWindowsHookEx(mouseHook);

    return 0;
}
