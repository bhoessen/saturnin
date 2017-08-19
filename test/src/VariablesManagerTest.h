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

#ifndef VARIABLESMANAGERTEST_H
#define	VARIABLESMANAGERTEST_H

#include <cppunit/extensions/HelperMacros.h>



class VariablesManagerTest : public CppUnit::TestFixture {
public:

    CPPUNIT_TEST_SUITE(VariablesManagerTest);
    CPPUNIT_TEST(testGetVar);
    CPPUNIT_TEST(testGetLit);
    CPPUNIT_TEST(testGetLitSign);
    CPPUNIT_TEST(testOppositeLit);
    CPPUNIT_TEST(testToDimacs);
    CPPUNIT_TEST_SUITE_END();

    void testGetVar();

    void testGetLit();

    void testGetLitSign();

    void testOppositeLit();
    
    void testToDimacs();

};

#endif	/* VARIABLESMANAGERTEST_H */

