/*
Copyright (c) <2014> <B.Hoessen>

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

#ifndef SATURNIN_UTILS_H
#define	SATURNIN_UTILS_H

#include <stddef.h>
#include <stdint.h>
#include <cstddef>
#include "saturnin/Assert.h"

namespace saturnin {

    /**
     * Read a integer from a C string
     * Our own implementation of strtol except that we can specify the lenght of the
     * input instead of relying on a NUL terminated string.
     * @param nptr the pointer to the start of string
     * @param endptr the pointer that will be set to where the parsing stopped
     * @param length the lenght of string @a nptr
     * @param nbRead the number of character that were read for the number returned
     * @return the value we were able to read
     */
    int64_t strntol(const char *nptr, const char **endptr, std::ptrdiff_t length, int& nbRead);

    /**
     * Template to create a new array on the heap and having it automatically
     * deleted with the destruction of this helper
     */
    template<typename T>
    class ArrayHelper {
    public:

        /**
         * Creates a new array of a given size
         * @param sz the size of the new array
         */
        ArrayHelper(unsigned int sz) : data(nullptr) {
            data = new T[sz];
        }

        /** Destructor */
        ~ArrayHelper() {
            delete[](data);
        }

        /**
         * Retrieve access to the array
         * @return the underlying array
         */
        operator T*() {
            return data;
        }

        /**
         * Retrieve a const access to the array
         * @return the underlying array
         */
        operator const T*() const {
            return data;
        }

    private:

        /** Non implemented */
        ArrayHelper& operator=(const ArrayHelper&);
        ArrayHelper(const ArrayHelper&);

        /** The pointer to the allocated array */
        T* data;
    };

}

#ifdef PROFILE

#if defined (WIN32) || defined (_MSC_VER)
#include <intrin.h>
#define SATURNIN_BEGIN_PROFILE uint64_t _saturnin__smsvc_rdtsc_ = __rdtsc();
#define SATURNIN_START_PROFILE(v) v = __rdtsc();
#define SATURNIN_STOP_PROFILE(start, end) end = __rdtsc() - start;
#define SATURNIN_END_PROFILE(v) v += __rdtsc() - _saturnin__smsvc_rdtsc_;
#else
__inline__ uint64_t __saturnin__get_cycles_count_() {
    unsigned a, d;
    asm volatile("rdtsc" : "=a" (a), "=d" (d));
    return ((uint64_t) a) | (((uint64_t) d) << 32);
}

#define SATURNIN_BEGIN_PROFILE uint64_t __saturnin__sgcc__ = __saturnin__get_cycles_count_();
#define SATURNIN_END_PROFILE(v) v += __saturnin__get_cycles_count_() - __saturnin__sgcc__;
#endif
#else
#define SATURNIN_BEGIN_PROFILE {}
#define SATURNIN_END_PROFILE(f) {}
#endif /* PROFILE */

#endif	/* SATURNIN_UTILS_H */

