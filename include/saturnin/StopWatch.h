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

#ifndef SATURNIN_STOPWATCH_H
#define	SATURNIN_STOPWATCH_H

#include <sys/timeb.h>
#include "Saturnin.h"

namespace saturnin {

    /**
     * This class represent a stopwatch
     */
    class SATURNIN_EXPORT StopWatch final {
    public:

        /**
         * Create a new stopwatch
         */
        StopWatch();

        /**
         * Start the stopwatch
         */
        void start();

        /**
         * Stop the stopwatch
         */
        void stop();

        /**
         * Retrieve the time elapsed in seconds
         * @return the number of seconds with a millisec precision
         */
        double getTimeEllapsed() const;

        /**
         * Retrieve the number of seconds elapsed
         * @return the number of seconds
         */
        int getSecondsEllapsed();

        /**
         * Retrieve the time in seconds ellapsed since the start
         * @return the number of seconds since the start
         */
        double getIntermediate() const;

        /**
         * Retrieve the number of millisec elapsed
         * @return the number of millisec
         */
        long getMilliEllapsed();

    private:
        struct timeb before, after;
        long sec;
        unsigned short milli;

    };

}




#endif	/* SATURNIN_STOPWATCH_H */

