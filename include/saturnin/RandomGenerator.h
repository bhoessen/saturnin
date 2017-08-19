/*
Copyright (c) <2015> <B.Hoessen>

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

#ifndef SATURNIN_RANDOMGENERATOR_H
#define	SATURNIN_RANDOMGENERATOR_H

#include "Saturnin.h"

namespace saturnin{
    
    /**
     * This class defines a non cryptographic random generator able to generate
     * a random unsigned char
     */
    class SATURNIN_EXPORT RandomGenerator final {
    public:
        /**
         * Constructor
         * @param initialValue the initial value that is provided to initialize
         *        the random generator
         */
        RandomGenerator(unsigned int initialValue = 0);
        
        /**
         * Retrieve the next random number
         * @return the next random number
         */
        unsigned char getNext();
        
    private:
        /** 
         * The value from which we can derive the value of the next random
         * number to provide */
        unsigned char next;
    };
    
}

#endif	/* RANDOMGENERATOR_H */

