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

#ifndef HEAPTEST_H
#define	HEAPTEST_H

#include <cppunit/extensions/HelperMacros.h>

class HeapTest : public CppUnit::TestFixture {
public:

    CPPUNIT_TEST_SUITE(HeapTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testInsertion);
    CPPUNIT_TEST(testOperator);
    CPPUNIT_TEST(testGetMin);
    CPPUNIT_TEST(testUpdate);
    CPPUNIT_TEST(testPresent);
    CPPUNIT_TEST(testContainLong);
    CPPUNIT_TEST_SUITE_END();

    /**
     * Check the creation of a heap
     */
    void testCreation();

    /**
     * Check the insertion of elements
     */
    void testInsertion();

    /**
     * Check that we can retrieve the value of the elements we add in the heap
     */
    void testOperator();

    /**
     * Check that the retrieval of the minimum is correct
     */
    void testGetMin();

    /**
     * Check the update of a value present in the heap
     */
    void testUpdate();
    
    /**
     * Re-create a bug found
     */
    void testPresent();
    
    /**
     * A big test to be sure that we don't reproduce a bug
     */
    void testContainLong();


};

#endif	/* SOLVERTEST_H */

