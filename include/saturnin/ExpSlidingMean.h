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

#ifndef SATURNIN_EXPSLIDINGMEAN_H
#define	SATURNIN_EXPSLIDINGMEAN_H

#include "Saturnin.h"

namespace saturnin {

    /**
     * This class aims to help at computing an exponential sliding mean.
     * Let m be the mean and x the value to add, then we compute the sliding
     * mean by applying the following formula: m = alpha * x + (1-alpha) * m
     * Where alpha is coefficient between 0 and 1
     */
    class SATURNIN_EXPORT ExpSlidingMean final {
    public:

        /**
         * Create a new exponential sliding mean
         * @param N the number of values we would like in our sliding mean
         * @param initialValue the initial value of the sliding mean
         */
        ExpSlidingMean(unsigned int N, double initialValue);

        /**
         * Copy constructor
         * @param source the exponential sliding mean to copy
         */
        ExpSlidingMean(const ExpSlidingMean& source);

        /**
         * Copy assignment operator
         * @param source the exponential sliding mean to copy
         * @return the destination of the copy
         */
        ExpSlidingMean& operator=(const ExpSlidingMean& source);

        /**
         * Destructor
         */
        ~ExpSlidingMean();

        /**
         * Add a value to the exponential sliding mean
         * The value of the mean using the formula: m = alpha * @a v + alpha * m
         * @param v the value to add to the sliding mean
         */
        inline void addValue(double v) {
            mean = alpha * v + beta * mean;
        }

        /**
         * Retrieve the alpha coefficient
         * @return the coefficient applied to new values
         */
        inline double getAlpha() const {
            return alpha;
        }

        /**
         * Retrieve the mean
         * @return the computed mean
         */
        inline double getMean() const {
            return mean;
        }

    private:

        /** The value of the mean */
        double mean;
        /** The coefficient applied to new values when updating */
        double alpha;
        /** The coefficient applied to the mean when updating */
        double beta;
    };

}

#endif	/* SATURNIN_EXPSLIDINGMEAN_H */

