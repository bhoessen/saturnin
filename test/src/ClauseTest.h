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

#ifndef CLAUSETEST_H
#define	CLAUSETEST_H

#include <cppunit/extensions/HelperMacros.h>

class ClauseTest : public CppUnit::TestFixture {
public:

    CPPUNIT_TEST_SUITE(ClauseTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testOperators);
    CPPUNIT_TEST(testLBD);
    CPPUNIT_TEST(testEquality);
    CPPUNIT_TEST(testAttached);
    CPPUNIT_TEST(testClauseSize);
    CPPUNIT_TEST(testLitSetRemoval);
    CPPUNIT_TEST_SUITE_END();

    void testCreation();

    void testOperators();

    void testLBD();

    void testEquality();

    void testAttached();

    void testClauseSize();
    
    void testLitSetRemoval();

};


#endif	/* CLAUSETEST_H */

