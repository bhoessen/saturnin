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

#ifndef SOLVERTEST_H
#define	SOLVERTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "../../include/saturnin/Solver.h"

class SolverTest : public CppUnit::TestFixture {
public:

    CPPUNIT_TEST_SUITE(SolverTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testAddClause);
    CPPUNIT_TEST(testClauseSizeAddition);
    CPPUNIT_TEST(testPropagation);
    CPPUNIT_TEST(testPrintClauses);
    CPPUNIT_TEST(testSimplification);
    CPPUNIT_TEST(testSearchNoConflict);
    CPPUNIT_TEST(testSearchConflict);
    CPPUNIT_TEST(testLargerProblem);
    CPPUNIT_TEST(testPigeonHole);
    CPPUNIT_TEST(testOneSolution3);
    CPPUNIT_TEST(testOneSolution7);
    CPPUNIT_TEST(testCompleteUNSAT);
    CPPUNIT_TEST(testCompleteSAT);
    CPPUNIT_TEST(testdp10);
    CPPUNIT_TEST(testPhasedElimination);
    CPPUNIT_TEST(testBigInstance);
    CPPUNIT_TEST(testPresumedInstance);
    CPPUNIT_TEST(testDBChecks);
    CPPUNIT_TEST_SUITE_END();

    /**
     * Check that the DB save works
     */
    void testDBChecks();

    /**
     * Test on a real instance: dp4s
     */
    void testCompleteSAT();

    /**
     * Test on a real instance: dp4u
     */
    void testCompleteUNSAT();

    /**
     * Test on the dp10 instance
     */
    void testdp10();

    /**
     * Test on a quite big instance
     */
    void testBigInstance();

    /**
     * Test an instance with presumptions
     */
    void testPresumedInstance();

    /**
     * Check the equality finder
     */
    void testCheckEquality();

    /**
     * Check the creation of a clause
     */
    void testCreation();

    /**
     * Check the addition of clauses
     */
    void testAddClause();

    /**
     * Check the propagations
     */
    void testPropagation();

    /**
     * Check the print of clauses
     */
    void testPrintClauses();

    /**
     * Check the search process without having conflicts
     */
    void testSearchNoConflict();

    /**
     * Check the search process with some conflicts
     * This test is dependent of the variable selection heuristic
     */
    void testSearchConflict();


    /**
     * Try to solve a problem defined by hand a bit larger
     */
    void testLargerProblem();

    /**
     * Try to solve a pigeon hole problem
     */
    void testPigeonHole();
    
    /**
     * test a problem with only one solution on a problem with 3 var
     */
    void testOneSolution3();
    
    /**
     * test a problem with only one solution on a problem with 7 var
     */
    void testOneSolution7();

    /**
     * Check the simplification method of the solver
     */
    void testSimplification();

    /**
     * Check the assignation of pure variables
     */
    void testPhasedElimination();
    
    /**
     * Test the addition of clause of different size
     */
    void testClauseSizeAddition();

private:
    
    void testSolveInstance(const char* fileName, saturnin::wbool result, const char* answer = NULL) const;
    
    void createPigeonHoleProblem(unsigned int n, saturnin::Solver& s, bool print = false);
    
    void createOneSolutionProblem(saturnin::Solver& s, unsigned int nbVar);

};

#endif	/* SOLVERTEST_H */

