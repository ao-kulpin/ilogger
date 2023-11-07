class Option
{
public:
    enum class IOFormat  {normal, binary};
    enum class OwnAction {normal, highlight, skip};
 private:
    int _skip = 0;                      // delay between outputs (0 - 1000)
    IOFormat _iof = IOFormat::normal;   // input/output format
    OwnAction _ownAction = OwnAction::normal; // reaction to "own" actions of utility 
public:
    bool acceptArgs(int argc, char* argv[]);
    bool acceptArgs(const char* cmdLine);
    
    int skip()              { return _skip; }
    IOFormat ioformat()     { return _iof; }
    OwnAction ownAction()   { return _ownAction; }
};

extern Option option;