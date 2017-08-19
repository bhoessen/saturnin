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

#ifndef LOGGERTEST_H
#define	LOGGERTEST_H

#include <cppunit/extensions/HelperMacros.h>

class LoggerTest : public CppUnit::TestFixture {
public:

    CPPUNIT_TEST_SUITE(LoggerTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testInteger);
    CPPUNIT_TEST(testLong);
    CPPUNIT_TEST(testUnsingedInteger);
    CPPUNIT_TEST(testDouble);
    CPPUNIT_TEST(testConstChar);
    CPPUNIT_TEST(teststdoutLogger);
    CPPUNIT_TEST_SUITE_END();

    /**
     * Check the creation of a logger
     */
    void testCreation();

    /**
     * Check the write operation of an integer
     */
    void testInteger();

    /**
     * Check the write operation of a long
     */
    void testLong();

    /**
     * Check the write operation of an unsigned integer
     */
    void testUnsingedInteger();

    /**
     * Check the write operation of a double
     */
    void testDouble();

    /**
     * Check the write operation of a const char
     */
    void testConstChar();

    /**
     * Check the standard output logger
     */
    void teststdoutLogger();
    
};

#endif	/* LOGGERTEST_H */

