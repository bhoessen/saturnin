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

#ifndef ARRAYTEST_H
#define	ARRAYTEST_H

#include <cppunit/extensions/HelperMacros.h>

class ArrayTest : public CppUnit::TestFixture {
public:

    CPPUNIT_TEST_SUITE(ArrayTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testPush);
    CPPUNIT_TEST(testGet);
    CPPUNIT_TEST(testOperator);
    CPPUNIT_TEST(testPop);
    CPPUNIT_TEST(testResize);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST(testPointers);
    CPPUNIT_TEST(testPushResize);
    CPPUNIT_TEST(testInitializerList);
    CPPUNIT_TEST_SUITE_END();

    /**
     * The aim of this test is to check that on creation, the specified capacity
     * is taken into account, and that the initial size is 0
     */
    void testCreation();

    /**
     * This test aims to check that the capacity increase when we perform a push
     */
    void testPush();

    /**
     * This test aims to push the first element of a vector to itself when the
     * push provokes a resize
     */
    void testPushResize();

    /**
     * This test aims to check that the values that are put in the array
     * are the same values we obtain through the get operation
     */
    void testGet();

    /**
     * This test aims to check that the values that are put in the array
     * are the same values we obtain through the [] operator, and that we
     * can also modify them
     */
    void testOperator();

    /**
     * Check the resize method
     */
    void testPop();
    
    /**
     * Remove more than one element at a time
     */
    void testPopALot();

    /**
     * Check that the behavior of the Array stay consistent after a resize is
     * performed
     */
    void testResize();


    /**
     * Check the copy operators
     */
    void testCopy();
    
    /**
     * Test the use of pointers stored inside the array
     */
    void testPointers();
    
    /**
     * Test the initializer list
     */
    void testInitializerList();

};

#endif	/* ARRAYTEST_H */

