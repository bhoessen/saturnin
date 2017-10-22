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

#ifndef SATURNIN_MEAN_H
#define	SATURNIN_MEAN_H

#include "Assert.h"
#include "Saturnin.h"

namespace saturnin{
    
    /**
     * This class aims to help the computation of a mean
     */
    class SATURNIN_EXPORT Mean final {
    public:
        
        /**
         * Create a new mean
         */
        Mean();
        
        /**
         * Copy constructor
         * @param source the source of the copy
         */
        Mean(const Mean& source);
        
        /**
         * Copy assignment operator
         * @param source the source of the copy
         * @return the destination of the copy
         */
        Mean& operator=(const Mean& source);
        
        /**
         * Retrieve the mean value
         * @return the mean value
         */
        inline double getMean() const{
            ASSERT(nbElements > 0);
            return sum / nbElements;
        }
        
        /**
         * Add a new value to the mean
         * @param v the value to add to the mean
         */
        inline void addValue(double v){
            sum += v;
            nbElements++;
        }
        
        /**
         * Retrieve the number of values we added to compute the mean
         * @return the number of values added
         */
        inline uint64_t getNbValues() const{
            return nbElements;
        }
        
    private:
        
        /** The sum of the values added */
        double sum;
        /** The number of elements added */
        uint64_t nbElements;
        
    };
    
}

#endif	/* SATURNIN_MEAN_H */

