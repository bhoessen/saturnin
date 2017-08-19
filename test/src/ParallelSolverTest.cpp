#ifdef SATURNIN_PARALLEL
#include "ParallelSolverTest.h"
#include "../../include/saturnin/ParallelSolver.h"
#include "saturnin/Solver.h"

CPPUNIT_TEST_SUITE_REGISTRATION(ParallelSolverTest);

void ParallelSolverTest::testCreation(){
    saturnin::ParallelSolver ps(7);
    CPPUNIT_ASSERT_EQUAL(wUnknown, ps.getState());
    CPPUNIT_ASSERT_EQUAL(sizeof(int)==sizeof(intptr_t) ? (size_t) 52668 :(size_t)78344,
            ps.getMemoryFootprint());
    size_t s = ps.getMemoryFootprint();
    CPPUNIT_ASSERT_EQUAL(wUnknown, ps.getWinningSolver().getState());
    CPPUNIT_ASSERT_EQUAL(s+ps.getWinningSolver().getMemoryFootprint() + sizeof(saturnin::Solver), 
            ps.getMemoryFootprint());
    ps.stop();
}

void ParallelSolverTest::testSimple() {
    saturnin::ParallelSolver ps(2);
    saturnin::CNFReader r("instances/simple.cnf");
    r.read();
    CPPUNIT_ASSERT(!ps.initialize(r));
    CPPUNIT_ASSERT(ps.solve(5));
    CPPUNIT_ASSERT_EQUAL(wTrue, ps.getWinningSolver().getState());
}

void ParallelSolverTest::testSmallUnsat() {
    saturnin::ParallelSolver ps(2);
    saturnin::CNFReader r("instances/stupidUnsat.cnf");
    r.read();
    CPPUNIT_ASSERT(ps.initialize(r));
    CPPUNIT_ASSERT_EQUAL(wFalse, ps.getWinningSolver().getState());
}


void ParallelSolverTest::testDP10(){
#ifndef VALGRIND //way too long otherwise
    const char* fileName = "instances/dp10s10.shuffled.cnf";
    saturnin::CNFReader reader(fileName);
    reader.read();
    saturnin::ParallelSolver ps(2);
    CPPUNIT_ASSERT(!ps.initialize(reader));
    ps.addPresumption(0,true);
    CPPUNIT_ASSERT(ps.solve(std::numeric_limits<unsigned int>::max()));
    CPPUNIT_ASSERT(ps.getState() == wTrue);
#endif /* VALGRIND */
}

void ParallelSolverTest::testExchange() {
#ifndef VALGRIND //way too long otherwise
    const char* fileName = "instances/all.used-as.sat04-986.simplified.cnf";
    saturnin::CNFReader reader(fileName);
    reader.read();
    saturnin::ParallelSolver ps(2);
    CPPUNIT_ASSERT(!ps.initialize(reader));
    ps.addPresumption(0,true);
    CPPUNIT_ASSERT(!ps.solve(20));
    CPPUNIT_ASSERT(ps.getState() == wUnknown);
#endif /* VALGRIND */
}


#endif /* SATURNIN_PARALLEL */
