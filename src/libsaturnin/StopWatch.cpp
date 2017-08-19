#include "saturnin/StopWatch.h"

using namespace saturnin;

StopWatch::StopWatch() : before(), after(), sec(0), milli(0) {
    ftime(&before);
    ftime(&after);
}

void StopWatch::start() {
    ftime(&before);
}

void StopWatch::stop() {
    ftime(&after);
    sec = (long) (after.time - before.time);
    milli = after.millitm - before.millitm;
    if (milli > 1000) {
        sec--;
        milli += 1000;
    }
}

double StopWatch::getIntermediate() const {
    struct timeb temp;
    ftime(&temp);
    long nbSec = (long)(temp.time - before.time);
    auto nbMilli = temp.millitm - before.millitm;
    if (nbMilli > 1000) {
        nbSec--;
        nbMilli += 1000;
    }
    return nbSec + (nbMilli / 1000.0);
}

int StopWatch::getSecondsEllapsed() {
    if (milli > 0) {
        return sec + 1;
    } else {
        return sec;
    }
}

long StopWatch::getMilliEllapsed() {
    return sec * 1000 + milli;
}

double StopWatch::getTimeEllapsed() const {
    return sec + (milli / 1000.0);
}

