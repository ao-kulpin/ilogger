#undef __WINDOWS__
#define __WINDOWS__

#ifdef __WINDOWS__
#include <windows.h>

#define MOUSEEVENTF_VIRTUALDESK 0x4000
#endif

#include <assert.h>

class MInput {          // mouse input
public:
    enum class  Action {press, release, wheel, move};
    Action      _action;

    enum class  Button {none, left, middle, right};
    Button      _button = Button::none;

    long        _dx = 0;    // position
    long        _dy = 0;    //

    bool        _wheelUp = false;
    MInput(Action action, Button button, bool wheelUp = false, long dx = 0, long dy = 0) 
            : _action(action), _button(button), _dx(dx), _dy(dy), _wheelUp(wheelUp) {}

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
                        wmi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
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
};

class KInput {              // keybord input
public:
    enum class  Action {press, release};
    Action     _action;
    unsigned    _vk;       // virtual-key code
    KInput(Action action, unsigned vk) : _action(action), _vk(vk) {} 

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
};

class MKInput               // mouse/keyboard input
{
public:
    enum class Type {mouse, keyboard};
    Type    _type;
    union 
    {
        MInput _mi;
        KInput _ki;
    };
    
    MKInput(const MInput& mi) {
        _type = Type::mouse;
        _mi = mi;
    }

    MKInput(const KInput& ki) {
        _type = Type::keyboard;
        _ki = ki;
    }
#ifdef __WINDOWS__
    void toWin(INPUT& wi) {
        memset(&wi, 0, sizeof wi);
        switch(_type) {
            case Type::mouse:
                wi.type = INPUT_MOUSE;
                _mi.toWin(wi.mi);
                break;

            case Type::keyboard:
                wi.type = INPUT_KEYBOARD;
                _ki.toWin(wi.ki);
                break;

            default:
                assert(false);
                break;
        }
    }
#endif // __WINDOWS__
};

