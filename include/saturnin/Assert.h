/*
Copyright (c) <2012> <B.Hoessen>

This file is part of saturnin.

saturnin is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

saturnin is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with saturnin.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SATURNIN_ASSERT_H
#define	SATURNIN_ASSERT_H

#include <cstdint>

#ifdef DEBUG

#include "Logger.h"

#include <assert.h>
#include <stdlib.h>

#if defined _WIN32 || defined __CYGWIN__

#define SATURNIN_TRACE(logger) {}

#elif SUNSTUDIO

#define SATURNIN_TRACE(logger) {}

#else
#include <execinfo.h>
#include <cxxabi.h>



#include "Logger.h"

#define MAX_FRAMES 100

//Code for SATURNIN_TRACE obtained at:
//http://mykospark.net/2009/09/runtime-backtrace-in-c-with-name-demangling/
//In order to work, binaries must be compiled with the -rdynamic option
#define SATURNIN_TRACE(logger) {void* addresses[MAX_FRAMES];\
    int bt_size = backtrace(addresses, MAX_FRAMES);\
    char** symbols = backtrace_symbols(addresses, bt_size);\
    for (int x = 0; x < bt_size; ++x) {\
        size_t dem_size;\
        int status;\
        char temp[512];\
        char* demangled;\
        if (1 == sscanf(symbols[x], "%*[^(]%*[^_]%511[^)+]", temp)) {\
          if (nullptr != (demangled = abi::__cxa_demangle(temp, nullptr, &dem_size, &status))) {\
            logger << "#" << x << " " << demangled << "\n";\
            free(demangled);\
          }\
        }else if (1 == sscanf(symbols[x], "%511s", temp)) {\
          logger << "#" << x << " " << temp << "\n";\
        }else {\
          logger << "#" << x << " " << symbols[x] << "\n";\
        }\
    }\
    free(symbols);\
}
#endif

namespace saturnin {

    SATURNIN_EXPORT void _saturnin_incNbAssert();
    SATURNIN_EXPORT uint64_t _saturnin_nbAssert();

    template<typename T, typename B>
    void checkEquality(const T& expected, const T& actual, const char* nameEx, const char* nameAct, const char* fileName, int lineNb, B& out) {
        _saturnin_incNbAssert();
        if (expected != actual) {
            out << nameEx << " == " << nameAct << " is not respected\n";
            out << "Expected: " << expected << "\n";
            out << "Actual:   " << actual << "\n";
            out << "In " << fileName << ":" << lineNb << "\n";
            SATURNIN_TRACE(out);
            assert((expected) == (actual));
        }
    }

    template<typename B>
    inline void checkTruth(bool condition, const char* nameCond, const char* fileName, int lineNb, B& out) {
        _saturnin_incNbAssert();
        if (!(condition)) {
            out << nameCond << " is not respected\n";
            out << "In " << fileName << ":" << lineNb << "\n";
            SATURNIN_TRACE(out);
            assert(condition);
        }

    }
}


#define ASSERT(condition) {saturnin::checkTruth(condition, #condition, __FILE__, __LINE__, saturnin::Logger::getStdErrLogger());}

#define ASSERT_EQUAL(expected, actual) {saturnin::checkEquality(expected, actual, #expected, #actual, __FILE__, __LINE__, saturnin::Logger::getStdErrLogger());}

#define SKIP { int __skip_tmp = 0; __skip_tmp++;}

#define NB_ASSERTED saturnin::_saturnin_nbAssert()

#else

namespace saturnin {

    template<typename T, typename B>
    void checkEquality(const T&, const T&, const char*, const char*, const char*, int, B&) { }

    template<typename B>
    inline void checkTruth(bool , const char* , const char* , int , B& ) { }

}
#define ASSERT(condition) {}
#define ASSERT_EQUAL(a, b) {}
#define SKIP {}
#define NB_ASSERTED 0
#define SATURNIN_TRACE(a) {}
#endif

#endif	/* SATURNIN_ASSERT_H */

