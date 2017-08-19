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

#ifndef PARALLELSOLVERTEST_H
#define	PARALLELSOLVERTEST_H

#ifdef SATURNIN_PARALLEL

#include <cppunit/extensions/HelperMacros.h>

class ParallelSolverTest : public CppUnit::TestFixture {
public:

    CPPUNIT_TEST_SUITE(ParallelSolverTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testDP10);
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testSmallUnsat);
    CPPUNIT_TEST(testExchange);
    CPPUNIT_TEST_SUITE_END();
    
    void testCreation();
    void testSmallUnsat();
    void testDP10();
    void testSimple();
    void testExchange();
};

#endif /* SATURNIN_PARALLEL */

#endif	/* PARALLELSOLVERTEST_H */

