/*
Copyright (c) <2017> <B.Hoessen>

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
#ifndef SATURNIN_LAUNCHER_H
#define SATURNIN_LAUNCHER_H

#include "StopWatch.h"
#include "Saturnin.h"
#include <stdio.h>
#if defined (WIN32) || defined (_MSC_VER)
#include <thread>
#include <memory>
#endif

namespace saturnin {

    class CNFReader;
    class Solver;
#ifdef SATURNIN_PARALLEL
    class ParallelSolver;
#endif

    /**
     * The purpose of this class is to create an instance of the saturnin solver
     * based on the command line arguments
     */
    class SATURNIN_EXPORT Launcher final {
    public:

        /**
         * Create a new launcher based on the command line arguments
         */
        Launcher(int argc, char** argv);

        /**
         * Retrieve the name of the instance
         * @return the name of the instance file that was given to the command line
         */
        char* getInstanceName() const {
            return instanceName;
        }

#ifdef SATURNIN_PARALLEL
        /**
         * Retrieve the solver
         * @return the instance to the parallel solver
         */
        const ParallelSolver* getSolver() const {
            return solver;
        }
#else
        /**
        * Retrieve the solver
        * @return the instance to solver
        */
        const Solver* getSolver() const {
            return solver;
        }
#endif /* SATURNIN_PARALLEL*/

        /** Start the solving of the instance */
        int solve();

        /** Stop the search of the solution */
        void stop();

        /** Retrieve the number of variables that was declared in the instance file */
        unsigned int getNbVar() const { return nbVar; }
        /** Retrieve the number of clauses that was declared in the instance file */
        unsigned int getNbClauses() const { return nbClauses; }
        /** Retrieve the average lenght of clauses */
        unsigned int getAvgClauseLength() const { return avgLenght; }
        /** Retrieve the lenght of the longest clause */
        unsigned int getMaxClauseLength() const { return maxLenght; }
        double getEllapsedTime() const;

    private:
        
        /**
         * Print a readable representation of a given amount of memory
         * It will be a string of type "%14.3 [' ',k,m,g,t]b"
         * @param f the file in which we will print the amount of memory
         * @param mem the amount of memory
         */
        void printMemoryValue(FILE* f, size_t mem) const;

        /** Perform a simplification of the instance given by the reader and add the simplified instance in the given solver */
        void simplify(Solver& s, CNFReader& reader);

        void printStats(Solver& s) const;

        int printResult(Solver& s) const;

        void printHelp(char* programName) const;


#ifdef SATURNIN_PARALLEL
        ParallelSolver* solver = nullptr;
#else 
        Solver* solver = nullptr;
#endif /* SATURNIN_PARALLEL */
        bool clean_exit;
        mutable StopWatch w;
        size_t readMem;
        char* instanceName = nullptr;
        unsigned int nbVar = 0;
        unsigned int nbClauses = 0;
        unsigned int avgLenght = 0;
        unsigned int maxLenght = 0;
#if defined (WIN32) || defined (_MSC_VER)
        std::unique_ptr<std::thread> waitingThread;
#endif
    };

}

#endif
