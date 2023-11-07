#include <iostream>
#include <getopt.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "option.hpp"

using namespace std;

bool Option::acceptArgs(int argc, char* argv[]) {
    int optIndex = 0;

    static struct option longOptions[] = {
        {"skip", required_argument, 0, 1},
        {"ioformat", required_argument, 0, 2},
        {"ownaction", required_argument, 0, 3},
        { 0, 0, 0, 0}                       // end of option's list
    };

    int c = 0;
    while (c != -1)
    {
        int curind = optind;
        c = getopt_long(argc, argv, "\x01\x02\x03", longOptions, &optIndex);
        switch (c)
        {
        case -1: // end of list
            {
                const char* rest = argv[curind];
                if (rest && *rest) {
                    // unrecognized rest
                    cerr << "\n*** Unknown argument \"" << argv[curind] << "\"\n";
                    return false;
                }
            }
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
            }
            break;
   
        case 2: // --ioformat
            {
                assert(optIndex == 1);

                if (!strcmp(optarg, "normal")) {
                    _iof = IOFormat::normal;

                } else if (!strcmp(optarg, "binary")) {
                    _iof = IOFormat::binary;

                } else {
                    cerr << "\n*** Invalid --ioformat \"" << optarg << "\"\n";
                    return false;
                }
            }
            break;

        case 3: // --ownaction
            {
                assert(optIndex == 2);

                if (!strcmp(optarg, "normal")) {
                    _ownAction = OwnAction::normal;

                } else if (!strcmp(optarg, "highlight")) {
                    _ownAction = OwnAction::highlight;

                } else if (!strcmp(optarg, "skip")) {
                    _ownAction = OwnAction::skip;

                } else {
                    cerr << "\n*** Invalid --ownaction \"" << optarg << "\"\n";
                    return false;
                }
            }
            break;

        case '?':
            cerr << "\n*** Unknown option \"" << argv[curind] << "\"\n";
            return false;

        default:
            cerr << "\n*** Unknown option \"" << argv[curind] << "\"\n";
            return false;
        }
    } 

    return true;
}

bool Option::acceptArgs(const char* cmdLine) {
    // Split cmdLine ino arguments
    int argc = 1;
    char argBuf[1024] = { 0 };
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
    return acceptArgs(argc, argv);
}
