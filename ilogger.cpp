#define __MACOS__ 1

#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <assert.h>
#include <thread>
#include <chrono>

#include "sender.hpp"
#include "option.hpp"
#include "astore.hpp"
#include "mkinput.hpp"
#include "mtrack.hpp"

#ifdef __WINDOWS__

#include <Windows.h>

#endif // __WINDOWS__

#ifdef __LINUX__

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <linux/input.h>
#include <X11/XKBlib.h>
#include <X11/extensions/record.h>
#include <X11/Xlibint.h>

#include "displock.hpp"

#endif // __LINUX__

#ifdef __MACOS__

#include <ApplicationServices/ApplicationServices.h>

#endif // __MACOS__


using namespace std;

Option       option;
ActionStore  actionStore;
static
MouseTracker mtrack;                        // mouse position stored into the Action Store

class OutSkipper {
private:
    decltype(std::chrono::steady_clock::now()) _skipMoment;
    int         _skip;              
public:
    void    start() {
        _skip = option.skip();
    }

    bool    isSkipped() {
        if (_skip == 0)
            return false;
        static const chrono::milliseconds ms1(1);            
        const auto now = std::chrono::steady_clock::now();    
        if ((now - _skipMoment) / ms1 < _skip) 
            // skipping period is now
            return true;
        else {
            // skipping periond is out    
            _skipMoment = now;
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
#ifdef __WINDOWS__        
        if (_bin) {
            // reopen cout in binary mode
            _setmode(_fileno(stdout), _O_BINARY);
        }
#endif // __WINDOWS__        
    }

    //// static int outsize;        
    template <class T>
    inline 
    OutWriter& write(T& data, unsigned len) {
        // binary output
        assert(_bin);
        cout.write((const char*) &data, len);
///// outsize += len;
///// cerr << "+++ write "  << len  << " " << outsize << endl; 
        return *this;
    }

    template <class T>
    inline 
    OutWriter& write(T& data) {
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
        cout.flush();
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
};

/////// int OutWriter::outsize = 0;

static OutWriter ow;

#ifdef __WINDOWS__

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && !outSkipper.isSkipped()) {
        if (wParam == WM_KEYDOWN) {
            KBDLLHOOKSTRUCT* kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
            ow.writeMKI(MKInput(KInput(KInput::Action::press, kbdStruct->vkCode)));
        }
        else if (wParam == WM_KEYUP) {
            KBDLLHOOKSTRUCT* kbdStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
            ow.writeMKI(MKInput(KInput(KInput::Action::release, kbdStruct->vkCode)));
        }
    }

    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {

    if (nCode >= 0  && !outSkipper.isSkipped()) {
        if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN) {
            MInput::Button button = MInput::Button::none;

            switch (wParam) {
            case WM_LBUTTONDOWN:
                button = MInput::Button::left;
                break;

            case WM_RBUTTONDOWN:
                button = MInput::Button::right;
                break;

            case WM_MBUTTONDOWN:
                button = MInput::Button::middle;
                break;
            }
            ow.writeMKI(MKInput(MInput(MInput::Action::press, button)));
        }
        else if (wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP || wParam == WM_MBUTTONUP) {
            MInput::Button button = MInput::Button::none;

            switch (wParam) {
            case WM_LBUTTONUP:
                button = MInput::Button::left;
                break;

            case WM_RBUTTONUP:
                button = MInput::Button::right;
                break;

            case WM_MBUTTONUP:
                button = MInput::Button::middle;
                break;
            }
            ow.writeMKI(MKInput(MInput(MInput::Action::release, button)));
        }
        else if (wParam == WM_MOUSEWHEEL) {
            MSLLHOOKSTRUCT* pMhs = (MSLLHOOKSTRUCT*)lParam;
            short zDelta = HIWORD(pMhs->mouseData);
            ow.writeMKI(MKInput(MInput(MInput::Action::wheel, MInput::Button::none, zDelta > 0)));
        }
        else if (wParam == WM_MOUSEMOVE) {
            MSLLHOOKSTRUCT* mouseInfo = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
            int x = mouseInfo->pt.x;
            int y = mouseInfo->pt.y;
            mtrack.set(x, y);
            ow.writeMKI(MKInput(MInput(MInput::Action::move, MInput::Button::none, false, x, y)));
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

// cerr << "--skip " << option.skip() << endl;
// cerr << "--ioformat " << int(option.ioformat()) << endl;
// cerr << "--ownaction " << int(option.ownAction()) << endl;

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

#endif // __WINDOWS__

#ifdef __LINUX__

static Display* pDisplay = 0;
static XRecordRange* pRange = 0;
static XRecordContext context;

//struct taken from libxnee
typedef union {
	unsigned char		type;
	xEvent				event;
	xResourceReq		req;
	xGenericReply		reply;
	xError				error;
	xConnSetupPrefix	setup;
} XRecordDatum;

static bool hookRunning = false;

static
inline
MInput::Button getButton(int n) {
    switch(n) {
        case 1:
            return MInput::Button::left;

        case 2:
            return MInput::Button::middle;

        case 3:
            return MInput::Button::right;

        default:
        /////// cerr << "n: " << n << endl;
            return MInput::Button::none;
    }
}

static
void EventProc (XPointer, XRecordInterceptData *pRecord) {
    DisplayLocker dl(pDisplay);
    
    hookRunning = true;
    if (pRecord->category == XRecordFromServer && !outSkipper.isSkipped()) {
        const XRecordDatum& data = *(XRecordDatum *) pRecord->data;
        switch(data.type) { 
            case KeyPress:
                ow.writeMKI(MKInput(KInput(KInput::Action::press, data.event.u.u.detail)));
                break;

            case KeyRelease:
                ow.writeMKI(MKInput(KInput(KInput::Action::release, data.event.u.u.detail)));
                break;

            case ButtonPress:  {
                auto bn = data.event.u.u.detail;
                if (bn == 4 || bn == 5)
                    // Wheel's event
                    ow.writeMKI(MKInput(MInput(MInput::Action::wheel, MInput::Button::none, bn == 5)));
                else
                    // Button press    
                    ow.writeMKI(MKInput(MInput(MInput::Action::press, getButton(bn))));
                break;         
            }          

            case ButtonRelease:  {
                auto bn = data.event.u.u.detail;
                if (bn == 4 || bn == 5)
                    // Wheel's aftershock
                    break;
                else
                    // Button release
                    ow.writeMKI(MKInput(MInput(MInput::Action::release, getButton(data.event.u.u.detail))));
                break;           
            }

            case MotionNotify: {
                const auto& ptr = data.event.u.keyButtonPointer;
                auto x = ptr.rootX;
                auto y = ptr.rootY;
                mtrack.set(x, y);
                ow.writeMKI(MKInput(MInput(MInput::Action::move, MInput::Button::none, 
                                           false, x, y)));
                break;
            }

            default:
                break;
        }
    }

    XRecordFreeData(pRecord);
    XFlush(pDisplay);
}

int main(int argc, char* argv[]) {
    if (!option.acceptArgs(argc, argv)) {
        cerr << "\n*** Invalid arguments: ";
        for (int i = 1; i < argc; ++i)
            cerr << " " << argv[i];
        cout << endl;

        return 1;
    }

    ow.start();
    outSkipper.start();

    ////////////auto xit = XInitThreads();
    //////// cerr << "XInitThreads: " << xit << endl;

    pDisplay = XOpenDisplay(0);

    if (!pDisplay) {
        cerr << "\n*** Can't open X11 display\n";
        return 1;
    }

    {
        DisplayLocker dl(pDisplay);

        XRecordClientSpec clients = XRecordAllClients;
        pRange = ::XRecordAllocRange();
        pRange->device_events = {KeyPress, MotionNotify};
        context = ::XRecordCreateContext(pDisplay, XRecordFromServerTime, &clients, 1, &pRange, 1);

        if (!XRecordEnableContextAsync(pDisplay, context, EventProc, 0)) {
           cerr << "\n*** XRecordEnableContextAsync failed\n";
           return 1;
        }
    }

    thread senderThread(SenderThreadFunc);

    while (hookRunning) {
        XRecordProcessReplies(pDisplay);
        this_thread::sleep_for(1ms);
    }

    XCloseDisplay(pDisplay);
    return 0;
}

#endif // __LINUX__

#ifdef __MACOS__

static
CGEventRef eventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {

    return event;
}
 
int main(int argc, char* argv[]) {
    if (!option.acceptArgs(argc, argv)) {
        cerr << "\n*** Invalid arguments: ";
        for (int i = 1; i < argc; ++i)
            cerr << " " << argv[i];
        cout << endl;

        return 1;
    }

    ow.start();
    outSkipper.start();

    CFMachPortRef eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault, 
                                                kCGEventMaskForAllEvents, eventCallback, 0);

    if (!eventTap) {
        std::cerr << "Failed to create event tap" << std::endl;
        return 1;
    }

    CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    
    CFRunLoopRun();
    
    CFRelease(runLoopSource);
    CFRelease(eventTap);
 

}

#endif // __MACOS__