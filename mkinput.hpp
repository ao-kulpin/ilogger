#ifndef __MKINPUT__
#define __MKINPUT__

#ifdef __WINDOWS__
#include <windows.h>

#define MOUSEEVENTF_VIRTUALDESK 0x4000
#endif // __WINDOWS__

#ifdef __MACOS__
#include <ApplicationServices/ApplicationServices.h>
#endif // __MACOS__

#include <assert.h>

class MInput {          // mouse input
public:
    enum class  Action: unsigned char {press, release, wheel, move};
    Action      _action;

    enum class  Button: unsigned char {none, left, middle, right};
    Button      _button = Button::none;

    int         _dx = 0;    // position
    int         _dy = 0;    //

    bool        _wheelUp = false;
    MInput(Action action, Button button, bool wheelUp = false, int dx = 0, int dy = 0) 
            : _action(action), _button(button), _dx(dx), _dy(dy), _wheelUp(wheelUp) {}

    MInput() {     // default/cleaning constructor
        memset(this, 0, sizeof *this);
    }

#ifdef __WINDOWS__ 
    void toWin(MOUSEINPUT& wmi) {
        memset(&wmi, 0, sizeof wmi);
        switch(_action) {
            case Action::press:
                switch(_button) {
                    case Button::left:
                        wmi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                        break;
                        
                    case Button::middle:
                        wmi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
                        break;
                        
                    case Button::right:
                        wmi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
                        break;

                    default:
                        assert(false);
                        break;
                }
                break;

            case Action::release:
                switch(_button) {
                    case Button::left:
                        wmi.dwFlags = MOUSEEVENTF_LEFTUP;
                        break;
                        
                    case Button::middle:
                        wmi.dwFlags = MOUSEEVENTF_MIDDLEUP;
                        break;
                        
                    case Button::right:
                        wmi.dwFlags = MOUSEEVENTF_RIGHTUP;
                        break;

                    default:
                        assert(false);
                        break;
                }
                break;

            case Action::wheel:
                wmi.dwFlags = MOUSEEVENTF_WHEEL;
                wmi.mouseData = _wheelUp ? WHEEL_DELTA : -WHEEL_DELTA;
                break;

            case Action::move:
                wmi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK;
                wmi.dx = _dx;
                wmi.dy = _dy;
                break;

            default:
                assert(false);
                break;
        }
    }
#endif // __WINDOWS

#ifdef __MACOS__
private:
    CGEventType getType() {
        switch (_action) {
            case Action::press:
                switch(_button) {
                    case Button::left:
                        return kCGEventLeftMouseDown;

                    case Button::right:
                        return kCGEventRightMouseDown;

                    case Button::middle:
                        return kCGEventOtherMouseDown;

                    default:
                        assert(false);
                        return kCGEventNull;
                }

            case Action::release:
                switch(_button) {
                    case Button::left:
                        return kCGEventLeftMouseUp;

                    case Button::right:
                        return kCGEventRightMouseUp;

                    case Button::middle:
                        return kCGEventOtherMouseUp;

                    default:
                        assert(false);
                        return kCGEventNull;
                }

            default:
                assert(false);
                return kCGEventNull;
        }
    }

    CGMouseButton getButton() {
        switch(_button) {
            case Button::left:
                return kCGMouseButtonLeft;

            case Button::right:
                return kCGMouseButtonRight;

            default:
                return kCGMouseButtonCenter;
                
        }
    }

public:
    CGEventRef toMac() {
        switch(_action) {
            case Action::press: 
            case Action::release: {
                CGEventRef cge = CGEventCreateMouseEvent(0, getType(), CGPoint{0, 0}, getButton());
                return cge;
            }

            case Action::move: {
                CGEventRef cge = CGEventCreateMouseEvent(0, kCGEventMouseMoved, 
                                                         CGPoint{CGFloat(_dx), CGFloat(_dy)}, 
                                                         kCGMouseButtonLeft);
                return cge;
            }

            case Action::wheel:
                return CGEventCreateScrollWheelEvent(0, kCGScrollEventUnitLine, 1, _wheelUp ? -5 : 5);

            default:
                assert(false);
                return 0;
        }
        return 0;
    }
#endif // __MACOS__
};

class KInput {              // keybord input
public:
    enum class  Action: unsigned char {press, release};
    Action     _action;
    unsigned short _vk;     // virtual-key code
    KInput(Action action, unsigned vk) : _action(action), _vk(vk) {}

    KInput() {     // default/cleaning constructor
        memset(this, 0, sizeof *this);
    }

#ifdef __WINDOWS__
    void toWin(KEYBDINPUT& wki) {
        memset(&wki, 0, sizeof wki);
        switch(_action) {
            case Action::press:
                break;

            case Action::release:
                wki.dwFlags = KEYEVENTF_KEYUP;
                break;

            default:
                assert(false);
                break;
        }
        wki.wVk = _vk;
    }
#endif // __WINDOWS__

#ifdef __MACOS__
    inline
    CGEventRef toMac() {
        return CGEventCreateKeyboardEvent(NULL, _vk, _action == Action::press);
    }
#endif // __MACOS__
};

class MKInput {              // mouse/keyboard input
public:
    enum class Type: unsigned char {mouse, keyboard};
    Type    _type;
    union MKData
    {
        MKData() {
            memset(this, 0, sizeof *this);
        }
        
        MInput _mi;
        KInput _ki;
    } mk;
    
    MKInput() {     // default/cleaning constructor
        memset(this, 0, sizeof *this);
    }

    MKInput(const MInput& mi) {
        _type = Type::mouse;
        mk._mi = mi;
    }

    MKInput(const KInput& ki) {
        _type = Type::keyboard;
        mk._ki = ki;
    }
#ifdef __WINDOWS__
    void toWin(INPUT& wi) {
        memset(&wi, 0, sizeof wi);
        switch(_type) {
            case Type::mouse:
                wi.type = INPUT_MOUSE;
                mk._mi.toWin(wi.mi);
                break;

            case Type::keyboard:
                wi.type = INPUT_KEYBOARD;
                mk._ki.toWin(wi.ki);
                break;

            default:
                assert(false);
                break;
        }
    }
#endif // __WINDOWS__

#ifdef __MACOS__
    CGEventRef toMac() {
        switch(_type) {
            case Type::mouse:
                return mk._mi.toMac();

            case Type::keyboard:
                return mk._ki.toMac();

            default:
                assert(false);    
                return 0;
                
        }
    }
#endif // __MACOS__
};

#endif // __MKINPUT__
