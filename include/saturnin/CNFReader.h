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

#ifndef SATURNIN_CNFREADER_H
#define	SATURNIN_CNFREADER_H

#include <stdio.h>
#include "Array.h"
#include "Assert.h"
#include "VariablesManager.h"
#include "Saturnin.h"

namespace saturnin {

    /**
     * The purpose of this class is to read a CNF file
     */
    class SATURNIN_EXPORT CNFReader final {
    public:
        
        /**
         * This enum contains the different errors that can occur during the
         * reading of the file
         */
        enum CNFReaderErrors{
            /** No errors where found */
            cnfError_noError,
            /** We didn't find a '0' at the end of the line */
            cnfError_noZeroEOL,
            /** We couldn't read the file */
            cnfError_fileReading,
            /** We didn't find the header */
            cnfError_noHeader,
            /** The values in the header aren't legal */
            cnfError_wrongHeader,
            /** The value of a literal isn't legal*/
            cnfError_wrongLiteralValue,
            /** The number of clauses doesn't match */
            cnfError_wrongNbClauses
        };

        /**
         * Create a new CNFReader on the given file
         * @param fileName the name of the file we have to read
         */
        CNFReader(const char* fileName);
        
        /**
         * Destructor
         */
        ~CNFReader();

        /**
         * Read the file
         * @param nbBlocks the number of blocks to read at a time
         * @return the state of the reading using the CNFReaderErrors
         */
        CNFReaderErrors read(unsigned int nbBlocks = 256);

        /**
         * Retrieve the number of clauses read
         * @return the number of clauses we found in the file
         */
        unsigned int getNbClauses() const;

        /**
         * Retrieve the number of variable used in the file. For this measure,
         * we rely on the input file. No control is performed on this value
         * @return the number of variable in the file
         */
        unsigned int getNbVar() const;
        
        /**
         * Retrieve the average length of the clauses (unary included)
         * @return the average length of the clauses
         */
        inline unsigned int getAverageLength() const {
            return (clauses.getSize() == 0) ? 0 : lengthSum / clauses.getSize();
        }
        
        /**
         * Retrieve the size of the longest clause read
         * @return the greatest length
         * @return 
         */
        inline unsigned int getMaxLength() const {
            return maxLength;
        }

        /**
         * Retrieve a clause read
         * @param i the index of the clause we want to obtain
         * @return the requested clause
         */
        const Array<Lit>& getClause(unsigned int i) const;

        /**
         * Retrieve the list of presumed true literals
         * @return an array containing the literals that are presumed true
         */
        const Array<Lit>& getPresumptions() const{
            return presumptions;
        }
        
        /**
         * Retrieve a string representation of the error
         * @param error the error we want a string representation of
         * @return a human readable description of the error
         */
        static const char* getErrorString(CNFReaderErrors error);
        
        /**
         * Retrieve the memory footprint of this Array
         * @return the memory allocated in bytes for this array
         */
        size_t getMemoryFootprint() const;

    private:

        /** The file that we use to read the clauses */
        FILE* inputFile;
        /** The array containing the array representation of the clauses */
        Array<Array<Lit> > clauses;
        /** The number of variables defined in the header of the file */
        unsigned int nbVar;
        /** The memory that was used to read the file */
        size_t memoryToRead;
        /** The array containing the literals presumed to be true*/
        Array<Lit> presumptions;
        /** The sum of the length of the different clauses */
        unsigned int lengthSum;
        /** The greatest length of a clause */
        unsigned int maxLength;

        //! @cond Doxygen_Suppress
        //non imlemented functions
        /** MAY NOT BE USED (private and not implemented) */
        CNFReader(const CNFReader&);
        /** MAY NOT BE USED (private and not implemented) */
        CNFReader& operator=(const CNFReader&);
        //! @endcond

    };

}

#endif	/* SATURNIN_CNFREADER_H */

