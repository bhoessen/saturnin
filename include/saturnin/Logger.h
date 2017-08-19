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

#ifndef SATURNIN_LOGGER_H
#define	SATURNIN_LOGGER_H

#include <stdio.h>
#include "Saturnin.h"

namespace saturnin {

    /**
     * The logger class is used to print log messages
     * There is a main logger that can be accessed easily anywhere, but if
     * needed, a special Logger can be instantiated
     */
    class SATURNIN_EXPORT Logger final {
    public:

        /**
         * Create a new logger that will print on the given output stream
         * @param output the stream that will be used to print log messages
         */
        Logger(FILE* output = stdout);

        /**
         * Copy constructor
         * @param source the Logger we will copy from
         */
        Logger(const Logger& source);

        /**
         * Destructor. Will close the underlying output file
         */
        ~Logger();

        /**
         * Specify the output stream on which the log messages needs to be printed
         * @param output the stream that will be used to print log message
         */
        void setOutput(FILE* output);

        /**
         * Retrieve the output stream used to print log messages
         * @return the stream used to print log messages
         */
        FILE* getOutput();

        /**
         * Retrieve a reference to the main logger, logging to stdout
         * @return a reference to the main logger that can be used as a singleton
         */
        static Logger& getStdOutLogger();

        /**
         * Retrieve a reference to the main logger, logging to stderr
         * @return a reference to the main logger that can be used as a singleton
         */
        static Logger& getStdErrLogger();
        
        /**
         * Retrieve a reference to the database logger, defaulting to stdout 
         * @return a reference to the database logger that can be used as a singleton
         */
        static Logger& getDBLogger();

    private:

        /**
         * Copy-assignment operator
         * Not implemented
         * @param source the source of the copy
         * @return the destination of the copy
         */
        Logger& operator=(const Logger& source);

        /** The stream in which we will print the logs */
        FILE* stream;

    };

}

/**
 * Some function to use the Logger as if it was an output stream
 * @param l the logger on which we will print
 * @param i the integer to print
 */
SATURNIN_EXPORT saturnin::Logger& operator<<(saturnin::Logger& l, int i);

SATURNIN_EXPORT saturnin::Logger& operator<<(saturnin::Logger& l, long i);

SATURNIN_EXPORT saturnin::Logger& operator<<(saturnin::Logger& l, unsigned int i);

/**
 * Some function to use the Logger as if it was an output stream
 * @param l the logger on which we will print
 * @param d the double to print
 */
SATURNIN_EXPORT saturnin::Logger& operator<<(saturnin::Logger& l, double d);

/**
 * Some function to use the Logger as if it was an output stream
 * @param l the logger on which we will print
 * @param i the string to print
 */
SATURNIN_EXPORT saturnin::Logger& operator<<(saturnin::Logger& l, const char* i);


#endif	/* SATURNIN_LOGGER_H */

