#ifndef __MTRACK__
#define __MTRACK__

class MouseTracker {
private:    
    int     _x = 0;
    int     _y = 0;
public:
    inline
    void    set(int x, int y) {
        _x = x;
        _y = y;
    }
    inline
    int getX()          { return _x; }
    inline
    int getY()          { return _y; }
};

#endif // __MTRACK__