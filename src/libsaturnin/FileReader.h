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

#ifndef SATURNIN_FILEREADER_H
#define	SATURNIN_FILEREADER_H

#include <stdio.h>

namespace saturnin {

    /**
     * This class will help reading of a file by reading a big chunk of the file
     * and after that, we may retrieve it line by line
     * Internally, in order to try to read not too slowly, we will try to read
     * the file not line by line but through a multiple of block size at a time.
     * On linux, we will check the size of the block at runtime. On other, we
     * will just assume a block size is 4096 bytes. 
     */
    class FileReader {
    public:
        /**
         * Creates a new file reader
         * @param f the file to read
         * @param nbBlocks the number of blocks that will be read at the same
         *        time 
         */
        FileReader(FILE* f, unsigned int nbBlocks = 256);

        /**
         * Destructor
         */
        ~FileReader();

        /**
         * Retrieve the current line where we have removed the trailing \n
         * @param destination the reference to the line
         * @return the length of the line
         */
        unsigned int getNextLine(const char*& destination);

        /**
         * Check if we have read the whole file
         * @return true if the whole file was read, false otherwise
         */
        bool isEOF();
        
        inline size_t getMemoryFootprint() const{
            return bufSize * sizeof(char);
        }

    private:

        /**
         * Retrieve the current line where we have removed the trailing '\n'
         * @param destination the reference to the line
         * @param checked the position at which we now that the '\n' character
         *        isn't before
         * @return the length of the line
         */
        unsigned int getNextLine(const char*& destination, unsigned int checked);
        
        //not implemented
        FileReader(const FileReader& source);
        FileReader& operator=(const FileReader& source);

        /** The pointer to the file we are reading */
        FILE* currentFile;
        /** The size of the buffer */
        unsigned int bufSize;
        /** The current buffer */
        char* buf;
        /** The pointer to the beginning of the next line */
        unsigned int next;
        /** The number of elements read present in buf */
        unsigned int length;
        /** If true, we read the whole file */
        bool iseof;

    };

}

#endif	/* SATURNIN_FILEREADER_H */

