#include "../Utils.h"
#include <stdint.h>
#include <ctype.h>

int64_t saturnin::strntol(const char* nptr, const char** endptr, std::ptrdiff_t length, int& nbRead) {
    ASSERT_EQUAL(0, nbRead);
    int i = 0;
    while (i < length && isspace(*nptr)) {
        nptr++;
        i++;
    }
    if (i == length) {
        *endptr = nptr;
        return 0;
    }
    bool isNegative = false;
    if (*nptr == '-') {
        isNegative = true;
        nptr++;
    }
    int64_t value = 0;
    while (i < length && *nptr >= '0' && *nptr <= '9') {
        value = value * 10 + (*nptr) - '0';
        nptr++;
        i++;
        nbRead++;
    }
    if (isNegative) {
        value *= -1;
    }
    *endptr = nptr;
    return value;
}
