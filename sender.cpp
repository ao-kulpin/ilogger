#include <iostream>
#include <fcntl.h>

#include "sender.hpp"
#include "mkinput.hpp"
#include "option.hpp"

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

class InputReader {  // binary input from the stdin
private:    
    bool    _bin        = false;
    bool    _eof        = false;
    bool    _invalid    = false;
public:
    bool isBinary()     { return _bin; }    
    bool eof()          { return _eof; }
    bool invalid()      { return _invalid; }
    void start() {
        _bin = (option.ioformat() == Option::IOFormat::binary);
        if (_bin) {
            // reopen cin in binary mode
            _setmode(_fileno(stdin), _O_BINARY);
        }
    }

    template <class T>
    inline InputReader& read(T& data, unsigned len) {
        // binary input
        assert(_bin);
        cin.read((char*) &data, len);
        return *this;
    }

    template <class T>
    inline InputReader& read(T& data) {
        return read(data, sizeof data);
    }

    bool binSignature() {
        char buf[5] = {0};
        read(buf, 4);
        _invalid = strcmp(buf, "ilog") != 0;
        _eof = cin.eof();
        if (_invalid)
            cerr << "\n*** Ivalid binary signature\n";
        return !_invalid;
    }

    bool binMKInput(MKInput& mki) {
        if (!binSignature() || _eof) {
            return false;
        }

        read(mki._type);
        switch (mki._type) {
            case MKInput::Type::mouse:
                _invalid = !binMInput(mki.mk._mi);
                break;

            case MKInput::Type::keyboard:
                _invalid = !binKInput(mki.mk._ki);
                break;

            default:
                cerr << "\n*** Ivalid MKInput::Type " << int(mki._type) << endl;
                _invalid = true;
                break;
        }
        return !_invalid;
    }

    bool binMInput(MInput& mi) {
        read(mi._action);
        switch(mi._action) {
            case MInput::Action::press:
            case MInput::Action::release:
                read(mi._button);

                switch(mi._button) {
                    case MInput::Button::left:
                    case MInput::Button::middle:
                    case MInput::Button::right:
                        return true;
                    default:    
                        cerr << "\n*** Ivalid MInput::Button " << int(mi._button) << endl;
                        return false;
                }

            case MInput::Action::wheel:
                unsigned char u;
                read(u);
                mi._wheelUp = (u == 1);
                if (u > 1) {
                    cerr << "\n*** Ivalid MInput::wheelUp " << int(u) << endl;
                    return false;
                } else
                    return true;

            case MInput::Action::move:
                long dx;
                long dy;
                read(dx);
                read(dy);
                mi._dx = convertor.toAbsoluteX(dx);
                mi._dy = convertor.toAbsoluteY(dy);
                return true;

            default:
                cerr << "\n*** Ivalid MInput::Action " << int(mi._action) << endl;
                return false;
        }
    }

    bool binKInput(KInput& ki) {
        read(ki._action);
        switch (ki._action) {
            case KInput::Action::press:
            case KInput::Action::release:
                break;

            default:
                cerr << "\n*** Ivalid KInput::Action " << int(ki._action) << endl;
                return false;
        }
        read(ki._vk);
        return true;
    }
};

static InputReader ir;

void SenderThreadFunc() {
    ir.start();
    while(true) {
        bool valid = false;
        MKInput mki;
        if (ir.isBinary()) {
            // binary stdin
            valid = ir.binMKInput(mki);
            if (ir.eof() || !valid) {
                // end of the stdin or invalid input
                break;
            }

        } else {
            // textual stdin
            string line;
            getline(cin, line);
            if (cin.eof())
                // end of the stdin
                break;

            valid = !cin.fail() && ParseLine(line, mki);
            if (!valid)
               cerr << "\n*** invalid input message \"" << line << "\"\n";
        }
        if (valid) {
#ifdef __WINDOWS__            
            INPUT wi;
            mki.toWin(wi);
            if (SendInput(1, &wi, sizeof wi) != 1) {
                cerr << "\n*** Can't send message\n";
            }
#endif // __WINDOWS__            
        } 
    }

    cerr << "\n*** Sender's thread ended\n";
}

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