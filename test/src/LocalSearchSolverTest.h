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

#ifndef LOCALSEARCHSOLVERTEST_H
#define	LOCALSEARCHSOLVERTEST_H

#include <cppunit/extensions/HelperMacros.h>

namespace saturnin {
    class LocalSearchSolver;
}

class LocalSearchSolverTest : public CppUnit::TestFixture {
public:

    CPPUNIT_TEST_SUITE(LocalSearchSolverTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testStupidInstance);
    CPPUNIT_TEST(testSimpleInstance);
    CPPUNIT_TEST(testDP04s);
    CPPUNIT_TEST_SUITE_END();
    
    void testCreation();
    
    void testStupidInstance();
    
    void testSimpleInstance();
    
    void testDP04s();

private:
    
    void readInstanceAndFillLSSolver(const char* fileName, saturnin::LocalSearchSolver& ls);
};

#endif	/* LOCALSEARCHSOLVERTEST_H */

