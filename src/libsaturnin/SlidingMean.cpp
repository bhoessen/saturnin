#include "saturnin/SlidingMean.h"
#include <stddef.h>

using namespace saturnin;

SlidingMean::SlidingMean(unsigned int maxV) : array(nullptr), cap(maxV), size(0), pos(0), sum(0.0) {
    array = new double[cap];
#ifdef DEBUG
    for (unsigned int i = 0; i < cap; i++) {
        array[i] = 0.0;
    }
#endif
}

SlidingMean::SlidingMean(const SlidingMean& source) : array(nullptr), cap(source.cap), size(source.size), pos(source.pos), sum(source.sum) {
    array = new double[cap];
    for (unsigned int i = 0; i < cap; i++) {
        array[i] = source.array[i];
    }
}

SlidingMean& SlidingMean::operator=(const SlidingMean& source) {
    if (array != nullptr) {
        delete[](array);
    }
    cap = source.cap;
    size = source.size;
    pos = source.pos;
    sum = source.sum;
    array = new double[cap];
    for (unsigned int i = 0; i < cap; i++) {
        array[i] = source.array[i];
    }
    return *this;
}

SlidingMean::~SlidingMean() {
    delete[](array);
    array = nullptr;
}

