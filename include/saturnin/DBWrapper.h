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

#ifndef SATURNIN_DBWRAPPER_H
#define SATURNIN_DBWRAPPER_H
#ifdef SATURNIN_DB

#include "VariablesManager.h"
#include "Array.h"
#include "Saturnin.h"
#include <string>

/** Forward declaration of the DB class */
typedef struct sqlite3 sqlite3;

namespace saturnin {

    /** Forward declaration */
    class QueryTreatment;
    
    /**
     * This class is a wrapper for DB calls in order to save the different
     * clauses and the resolutions that lead to them
     */
    class SATURNIN_EXPORT DBWrapper final {
    public:

        /**
         * Create a new DBWrapper object
         * @param aDBName the string necessary to retrieve access to the DB
         * @param verbose if set to true, logs will be produced
         */
        DBWrapper(const char* aDBName = nullptr, bool verbose = false);

        /** Destructor */
        ~DBWrapper();

        /**
         * Add a resolution
         * @param lits the array containing the literals of a clause used in the
         *             resolution
         * @param resolvent the integer ID of the resulting resolvent
         * @return true if the resolution was correctly added
         */
        bool addResolution(const Lit* const lits, long resolvent);
        
        /**
         * Store a clause in the DB engine
         * @param lits the literals of the clause to add in the DB Engine
         * @param id the identifier of the clause
         * @return true if the clause was successfuly added
         */
        bool addClause(const Lit* const lits, long id);

        /**
         * Sanatize the data: sort the different literals and make sure that
         * the different ids are stored and used
         */
        void sanatize();
        
        /**
         * Compute the height of each clause.
         * The height of a clause is 1+the maximum height of the clauses used in
         * a resolution to create that clause
         */
        void computeHeights();
        
        /**
         * Check that the different resolution that were added are valid
         * @return true if the resolutions are valid, false otherwise
         */
        bool checkResolutions();
        
        /**
         * Check that the given clause is contained in the database
         * @param c the clause that we want to check. Its representation is an
         *          array of Lit, ending with a Lit_Undef
         * @return true if the clause @a c is contained in the database
         */
        bool checkContainsClause(const Lit* const c);
        
        /**
         * Check whether the database has been initialized
         * @return true if a database has been initialized and therefore, used
         */
        bool isInitialized() const {
            return internalDBEngine != nullptr;
        }
        
        /**
         * Retrieve the number of clauses that were added in this database
         * @return the number of clause in the database
         */
        long nbClausesAdded() const {
            return nbClauses;
        }
        
        /**
         * Retrieve the memory footprint of this solver.
         * It will compute the memory footprint of every object used by this
         * solver
         * @return the memory footprint in bytes
         */
        size_t getMemoryFootprint() const;
        
    private:

        /**
         * Execute a given sql query
         * @param query the query to execute by the DB engine
         * @return true if the query was successful
         */
        bool executeQuery(const char* query);
        
        /**
         * Execute a given sql query and apply a given treatment to the results
         * @param query the query to execute by the DB engine
         * @param treat the object that will treat the results
         * @return true if the query was successful
         */
        bool executeQuery(const char* query, QueryTreatment* treat);
        
        /**
         * Creates the sorted string representation of a clause
         * @param clause a string representation of a clause
         * @return a string representation of a clause, with its literals sorted
         */
        std::string sortLiterals(const std::string& clause) const;
        
        /** The pointer to the interal DB engine*/
        sqlite3* internalDBEngine;
        /** The number of clauses present in the DB */
        long nbClauses;
        /** If true, logs will be produced */
        bool produceLogs;

        //! @cond Doxygen_Suppress
        //private and non declared as it may not be used
        /** MAY NOT BE USED (private and not implemented) */
        DBWrapper& operator=(const DBWrapper&);

        /** MAY NOT BE USED (private and not implemented) */
        DBWrapper(const DBWrapper&);
        //! @endcond

        
    };

}

#endif /* SATURNIN_DB */
#endif /* SATURNIN_DBWRAPPER_H */
