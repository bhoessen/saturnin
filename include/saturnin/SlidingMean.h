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

#ifndef SATURNIN_SLIDINGMEAN_H
#define	SATURNIN_SLIDINGMEAN_H

#include "Assert.h"
#include "Saturnin.h"

namespace saturnin {

    /**
     * This class aims to compute the mean of n elements. Whenever a new element
     * needs to be added in the mean, we remove the oldest value to replace it
     * by the new value
     */
    class SATURNIN_EXPORT SlidingMean final {
    public:

        /**
         * Create a new SlidingMean object given a capacity
         * @param maxV the maximum number of value that can be used to compute
         *        the mean. That value will be the capacity of this object
         */
        SlidingMean(unsigned int maxV = 100);

        /**
         * Copy constructor
         * @param source the element that we will copy
         */
        SlidingMean(const SlidingMean& source);

        /**
         * copy assignment operator
         * @param source the elemtn that we will copy
         * @return the destination of the copy
         */
        SlidingMean& operator=(const SlidingMean& source);

        /**
         * Destructor
         */
        ~SlidingMean();

        /**
         * Add a new value to the mean
         * @param v the value to add
         */
        inline void addValue(double v) {
            if (size < cap) {
                sum += v;
                size++;
            } else {
                sum = sum - array[pos] + v;
            }
            array[pos] = v;
            pos = (pos + 1) % cap;
        }

        /**
         * Retrieve the mean of the values present
         * @return the mean
         */
        double getMean() const {
            ASSERT(size>0);
            return sum / size;
        }

        /**
         * Check whether this Mean object is full or not. In other words, we
         * check that we inserted at least as many values as the capacity
         * @return the capacity of the Mean
         */
        bool isFull() const {
            return size == cap;
        }
        
        /**
         * Check the number of values added and taken into account for this mean
         * @return the number of values added that will be taken into account
         *         for the mean.
         */
        unsigned int getNbValues() const {
            return size;
        }

        /**
         * Reset this mean. It will remove every value added
         */
        void reset() {
            sum = 0.0;
            pos = 0;
            size = 0;
        }

    private:

        /** The array containing the values added */
        double* array;
        /** The capacity of the array */
        unsigned int cap;
        /** The number of elements in the array */
        unsigned int size;
        /** The position for the next value to insert in the array */
        unsigned int pos;
        /** the sum of the elements in the array */
        double sum;
    };

}

#endif	/* SATURNIN_SLIDINGMEAN_H */

