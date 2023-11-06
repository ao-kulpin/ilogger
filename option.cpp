#include <iostream>
#include <getopt.h>
#include <assert.h>
#include <stdlib.h>

#include "option.hpp"

using namespace std;

bool Option::acceptArgs(int argc, char* argv[]) {
    int optIndex = 0;

    static struct option longOptions[] = {
        {"skip", required_argument, 0, 1},
        { 0, 0, 0, 0}                       // end of option's list
    };

    int c = 0;
    while (c != -1)
    {
        c = getopt_long(argc, argv, 0, longOptions, &optIndex);
        switch (c)
        {
        case -1: // end of list
            break;

        case 1: // --skip
        {
            assert(optIndex == 0);

            char* endPtr = 0;
            _skip = strtol(optarg, &endPtr, 10);
            if ( _skip < 0 || _skip > 1000 || endPtr == optarg) {
                cerr << "\n*** Invalid --skip \"" << optarg << "\"\n";
                return false;
            }

            break;
        }
        
        default:
            break;
        }
    } 

    return true;
}

bool Option::acceptArgs(const char* cmdLine) {
    // Split cmdLine ino arguments
cerr << "acceptArgs1 "    << cmdLine << endl;
    int argc = 1;
    char argBuf[1024];
    char fn[] = "ilogger";
    char* argv[128] = {fn, argBuf};
    const char* iPtr = cmdLine;
    char* oPtr = argBuf;
    for (const char* iPtr = cmdLine; true; ++iPtr) {
        char c = *iPtr;
        if (c == 0 || isspace(c)) {
            *oPtr = 0;                  // end of an argument
            if (argv[argc] == oPtr) 
                --argc;                 // skip the empty argument    
            argv[++argc] = ++oPtr;      // begin a next argument
        }
        if (c == 0)
            break;      // end of cmdLine
        else if (!isspace(c))
            *oPtr++ = c;
    }
cerr << "acceptArgs2 "    << argc << " " << argv[0] << " " << argv[1] << " " << argv[2] << endl;
    return acceptArgs(argc, argv);
}
