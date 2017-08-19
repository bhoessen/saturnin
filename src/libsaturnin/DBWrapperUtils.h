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

#ifndef SATURNIN_DBWRAPPERUTILS_H
#define	SATURNIN_DBWRAPPERUTILS_H

#ifdef SATURNIN_DB
#include <stdlib.h>
#include <sqlite3.h>

namespace saturnin {

    class PreparedStatement {
    public:

        PreparedStatement(sqlite3* aDBEngine, const char* query, bool verbose);

        ~PreparedStatement();

        inline operator sqlite3_stmt*() {
            return preparedStatement;
        }

    private:

        /** Non implemented */
        PreparedStatement(const PreparedStatement&);
        PreparedStatement& operator=(const PreparedStatement&);

        sqlite3* internalDBEngine;
        sqlite3_stmt* preparedStatement;
    };
    
    /**
     * A pure virtual class that will be used as a way to treat the results of
     * a query
     */
    class QueryTreatment{
    public:
        /**
         * Virtual destructor
         */
        virtual ~QueryTreatment() { }
        
        /**
         * The pure virtual function that needs to be implemented according to
         * the treatment needed
         * @param nbColumns the number of columns
         * @param columnsValue the value of the columns
         * @param columnsName the names of the columns
         * @return TODO
         */
        virtual int execute(int nbColumns, char** columnsValue, char** columnsName) = 0;
    };

}

#endif /* SATURNIN_DB */

#endif	/* SATURNIN_DBWRAPPERUTILS_H */

