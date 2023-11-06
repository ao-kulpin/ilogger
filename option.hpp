class Option
{
private:
    int _skip = 0; // delay between outputs (0 - 1000)
public:
    bool acceptArgs(int argc, char* argv[]);
    bool acceptArgs(const char* cmdLine);
    int skip()              { return _skip; }
};

extern Option option;