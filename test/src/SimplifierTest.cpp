#include "SimplifierTest.h"
#include "../../include/saturnin/CNFReader.h"
#include "../../include/saturnin/Simplifier.h"

CPPUNIT_TEST_SUITE_REGISTRATION(SimplifierTest);


void SimplifierTest::testCreation() {
    saturnin::Simplifier simp(4,5);
    CPPUNIT_ASSERT_EQUAL(4U, simp.getNbVar());
    CPPUNIT_ASSERT_EQUAL(0U, simp.getNbBinaryClauses());
    CPPUNIT_ASSERT_EQUAL(0U, simp.getInitialClauses().getSize());
}

void SimplifierTest::testRevival() {
    saturnin::CNFReader r("instances/dp04u03.shuffled.cnf");
    r.read();
    saturnin::Simplifier s(r.getNbVar(), r.getNbClauses());
    for (unsigned int i = 0; i < r.getNbClauses(); i++) {
        const saturnin::Lit* lits = r.getClause(i);
        s.addClause(lits, r.getClause(i).getSize());
    }
    s.revival(3);
    CPPUNIT_ASSERT_EQUAL(62U, s.getNbClausesReduced());
}


