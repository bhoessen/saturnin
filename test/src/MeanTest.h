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

#ifndef MEANTEST_H
#define	MEANTEST_H

#include <cppunit/extensions/HelperMacros.h>

class MeanTest : public CppUnit::TestFixture {
public:

    CPPUNIT_TEST_SUITE(MeanTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testInsertion);
    CPPUNIT_TEST(testMean);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST_SUITE_END();
    
    /**
     * Check the creation of the Mean object
     */
    void testCreation();
    
    /**
     * Check the insertion of values
     */
    void testInsertion();
    
    /**
     * Check that the computed mean is correct
     */
    void testMean();    

    /**
     * Test the copy
     */
    void testCopy();
};

#endif	/* MEANTEST_H */

