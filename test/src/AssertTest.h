/*
Copyright (c) <2013> <B.Hoessen>

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
#ifndef ASSERTTEST_H
#define	ASSERTTEST_H

#include <cppunit/extensions/HelperMacros.h>

class AssertTest : public CppUnit::TestFixture {
public:

    CPPUNIT_TEST_SUITE(AssertTest);
    CPPUNIT_TEST(testEquality);
    CPPUNIT_TEST(testTruth);
    CPPUNIT_TEST_SUITE_END();

    /**
     * Check the checkEquality method
     */
    void testEquality();

    /**
     * Check the checkTruth method
     */
    void testTruth();

};

#endif	/* ASSERTTEST_H */

