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

#ifndef SIMPLIFIERTEST_H
#define	SIMPLIFIERTEST_H

#include <cppunit/extensions/HelperMacros.h>

class SimplifierTest : public CppUnit::TestFixture {
public:
    
    CPPUNIT_TEST_SUITE(SimplifierTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testRevival);
    CPPUNIT_TEST_SUITE_END();
    
    void testCreation();
    
    void testRevival();
    
};

#endif	/* SIMPLIFIERTEST_H */

