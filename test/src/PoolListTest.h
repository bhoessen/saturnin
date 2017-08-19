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

#ifndef POOLLISTTEST_H
#define	POOLLISTTEST_H


#include <cppunit/extensions/HelperMacros.h>

class PoolListTest : public CppUnit::TestFixture {
public:

    CPPUNIT_TEST_SUITE(PoolListTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testMultipleClauses);
    CPPUNIT_TEST(testClauseRetrieval);
    CPPUNIT_TEST(testExtend);
    CPPUNIT_TEST_SUITE_END();

    /**
     * Check the creation of a PoolList
     */
    void testCreation();

    /**
     * Check that we can create clauses and use them correctly
     */
    void testClauseRetrieval();

    /**
     * Check that everything is still working after having the capacity extended
     */
    void testExtend();

    /**
     * Check that we can manage different clauses at the same time
     */
    void testMultipleClauses();
    
};

#endif	/* POOLLISTTEST_H */

