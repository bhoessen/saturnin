#include "saturnin/Array.h"

namespace saturnin{
unsigned long int _saturnin_array_access_ = 0;
}

using namespace saturnin;

Array<unsigned int>::~Array() {
    delete[](data);
    data = nullptr;
}

void Array<unsigned int>::push(unsigned int elmnt) {
    if (size >= cap) {
        resize();
    }
    ASSERT(size < cap);
    data[size] = elmnt;
    size++;
}

void Array<unsigned int>::resize() {
    unsigned int newCap = (unsigned int) (cap * SATURNIN_ARRAY_UPDATE_FACTOR);
    ASSERT(newCap > cap);
    unsigned int* tmp = new unsigned int[newCap];
    ASSERT(tmp != nullptr);
    memcpy(tmp, data, cap * sizeof (unsigned int));
    delete[](data);
    data = tmp;
    cap = newCap;
}
