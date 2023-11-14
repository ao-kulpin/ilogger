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
    map<int, KCount> _kMap;         // keyboard actions

public:
    void put(const MKInput& mki, int mouseX, int MouseY) {
        Locker loc(_storeMutex);

        switch (mki._type) {
            case MKInput::Type::keyboard: {
                auto& key = mki.mk._ki;
                ++ _kMap[key._vk]._nums[int(key._action)];
            }
            break;

            case MKInput::Type::mouse:
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
                auto& key = mki.mk._ki;
                auto& count = _kMap[key._vk]._nums[int(key._action)];
                if (count == 0)
                    return false;
                else {
                    ++ count;
                    return true;
                }
            }
            break;

            case MKInput::Type::mouse:
                return false;

            default:
                assert(false);
                return false;
        }
    }
};

extern ActionStore actionStore;

#endif // __ASTORE__
