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

#ifndef FIFOTEST_H
#define	FIFOTEST_H

#include <cppunit/extensions/HelperMacros.h>

class FifoTest : public CppUnit::TestFixture {
public:

    CPPUNIT_TEST_SUITE(FifoTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST(testPushPop);
    CPPUNIT_TEST(testPushAround);
    CPPUNIT_TEST(testIncreaseCapacity);
    CPPUNIT_TEST_SUITE_END();

    /**
     * Check the initial values of the fifo
     */
    void testCreation();

    /**
     * Check the different copy statements
     */
    void testCopy();

    /**
     * Check that we can add some elements in the fifo
     */
    void testPushPop();

    /**
     * Check that the array is well used after we pushed more than the capacity
     * and popped elements in between
     */
    void testPushAround();

    /**
     * Check that the increase of the array works properly
     */
    void testIncreaseCapacity();
};

#endif	/* FIFOTEST_H */

