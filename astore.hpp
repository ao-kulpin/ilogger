#ifndef __ASTORE__
#define __ASTORE__

#include <mutex>
#include <map>
#include <assert.h>

#include "mkinput.hpp"

using namespace std;

class ActionStore {
private:
    mutex   _storeMutex;
    class Locker {
    private:
        mutex&      _ref;
    public:
        inline Locker(mutex& m) : _ref(m) {
            _ref.lock();
        }
        ~Locker () { 
            _ref.unlock();
        }
    };

    class KCount {
    public:
        int     _nums[2] = {0, 0};
    };

    class MCount {
    public:
        int     _nums[4][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

    };

    map<int, KCount> _kMap;         // keyboard actions

    typedef unsigned long long ullong;
    map<ullong, MCount> _mMap;       // mouse actions

    inline
    ullong getMouseKey(int x, int y) {
        return (ullong(unsigned(x)) << 32) | ullong(unsigned(y));
    }

    inline
    int& getCount(const MInput& mi, MCount& counters) {
        switch(mi._action) {
            case MInput::Action::press:
            case MInput::Action::release:
                return counters._nums[int(mi._action)][int(mi._button) - 1];

            case MInput::Action::wheel:
                return counters._nums[int(mi._action)][int(mi._wheelUp)];

            case MInput::Action::move:
                return counters._nums[int(mi._action)][0];

            default:
                assert(false);
                return counters._nums[0][0];
        }
    }

public:
    void put(const MKInput& mki, int mouseX, int MouseY) {
        Locker loc(_storeMutex);

        switch (mki._type) {
            case MKInput::Type::keyboard: {
                auto& ki = mki.mk._ki;
                ++ _kMap[ki._vk]._nums[int(ki._action)];
            }
            break;

            case MKInput::Type::mouse: 
                ++ getCount(mki.mk._mi, _mMap[getMouseKey(mouseX, MouseY)]);
                break;

            default:
                assert(false);
                break;
        }
    }

    bool check(const MKInput& mki, int mouseX, int MouseY) {
        Locker loc(_storeMutex);

        switch (mki._type) {
            case MKInput::Type::keyboard: {
                auto& ki = mki.mk._ki;
                auto& count = _kMap[ki._vk]._nums[int(ki._action)];
                if (count <= 0)
                    return false;
                else {
                    -- count;
                    return true;
                }
            }
            break;

            case MKInput::Type::mouse: {
                auto& count = getCount(mki.mk._mi, _mMap[getMouseKey(mouseX, MouseY)]);
                if (count <= 0)
                    return false;
                else {
                    -- count;
                    return true;
                }
            }
            break;

            default:
                assert(false);
                return false;
        }
    }
};

extern ActionStore actionStore;

#endif // __ASTORE__
