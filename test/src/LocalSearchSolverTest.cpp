#include "LocalSearchSolverTest.h"
#include "../../include/saturnin/LocalSearchSolver.h"
#include "../../include/saturnin/Clause.h"
#include "../../include/saturnin/CNFReader.h"
#include "../../include/saturnin/Simplifier.h"

CPPUNIT_TEST_SUITE_REGISTRATION(LocalSearchSolverTest);

void LocalSearchSolverTest::testCreation() {
    saturnin::LocalSearchSolver ls(4, 4);
    CPPUNIT_ASSERT(wTrue == ls.getState());
    CPPUNIT_ASSERT(ls.isModel());
}

void LocalSearchSolverTest::testStupidInstance() {
    saturnin::LocalSearchSolver ls(4, 4);
    saturnin::Array<saturnin::Lit> a{1U, 3U, 5U, 7U};
    ls.addClause(a);
    saturnin::Array<saturnin::Lit> b{0U, 2U, 4U, 6U};
    ls.addClause(b);
    ls.setTabooLength(0U);
    CPPUNIT_ASSERT(!ls.isModel());
    CPPUNIT_ASSERT(wUnknown == ls.getState());
    CPPUNIT_ASSERT_EQUAL(4U, ls.getNbVar());
    CPPUNIT_ASSERT(ls.solve(10));
    CPPUNIT_ASSERT(wTrue == ls.getState());
    CPPUNIT_ASSERT(ls.isModel());
    CPPUNIT_ASSERT(ls.solve(1));
}

void LocalSearchSolverTest::testSimpleInstance() {
    saturnin::CNFReader r("instances/simple.cnf");
    r.read();
    saturnin::LocalSearchSolver ls(r.getNbVar(), r.getNbClauses());
    for (unsigned int i = 0; i < r.getNbClauses(); i++) {
        ls.addClause(r.getClause(i));
    }
    saturnin::Array<saturnin::Lit> cl(r.getNbVar());
    for (unsigned int i = 0; i < r.getNbVar(); i++) {
        cl.push(saturnin::VariablesManager::getLit(i, false));
    }
    ls.addClause(cl);
    CPPUNIT_ASSERT(!ls.isModel());
    CPPUNIT_ASSERT(wUnknown == ls.getState());
    CPPUNIT_ASSERT(ls.solve(r.getNbVar()*4));
    CPPUNIT_ASSERT(wTrue == ls.getState());
    CPPUNIT_ASSERT(ls.isModel());
}

void LocalSearchSolverTest::readInstanceAndFillLSSolver(const char* fileName, saturnin::LocalSearchSolver& ls) {
    saturnin::CNFReader r(fileName);
    r.read();
    saturnin::Simplifier s(r.getNbVar(), r.getNbClauses());
    for (unsigned int i = 0; i < r.getNbClauses(); i++) {
        const saturnin::Lit* lits = r.getClause(i);
        s.addClause(lits, r.getClause(i).getSize());
    }
    
    s.revival(r.getAverageLength());

    //Add the unary clauses
    const saturnin::Array<saturnin::Lit>& provenLits = s.getProvenLit();
    for (unsigned int i = 0; i < provenLits.getSize(); i++) {
        saturnin::Lit l = provenLits.get(i);
        ASSERT(l != saturnin::lit_Undef);
        saturnin::Array<saturnin::Lit> cl(1U);
        cl.push(l);
        ASSERT(ls.addClause(cl));
    }
    
#ifdef DEBUG
    for(unsigned int v = 0; v < r.getNbVar(); v++){
        ASSERT_EQUAL(s.getVarValue(v), ls.getProvenValue(v));
    }
#endif /* DEBUG */

    //Add the binary clauses
    for (unsigned int l = 0; l < r.getNbVar()*2; l++) {
        //either the clause is SAT and the clause doesn't need to be added
        //either the clause should be unary and has been already propagated
        if (s.getVarValue(saturnin::VariablesManager::getVar(l)) != wUnknown) continue;
        ASSERT_EQUAL(wUnknown, ls.getProvenValue(saturnin::VariablesManager::getVar(l)));
        
        const saturnin::Array<saturnin::Lit>& binClauses = s.getBinaryWith(l);
        for (unsigned int j = 0; j < binClauses.getSize(); j++) {
            saturnin::Lit tmp = binClauses.get(j);
            if (tmp < l && s.getVarValue(saturnin::VariablesManager::getVar(tmp)) == wUnknown) {
                saturnin::Array<saturnin::Lit> cl(2U);
                cl.push(l);
                cl.push(tmp);
                ASSERT(wUnknown == ls.getProvenValue(saturnin::VariablesManager::getVar(tmp)));
                ls.addClause(cl);
            }
        }

    }

    //add the rest of the clauses
    const saturnin::Array<saturnin::Clause*>& clauses = s.getInitialClauses();
    for (unsigned int i = 0; i < clauses.getSize(); i++) {
        ls.addClause(clauses.get(i)->lits(), clauses.get(i)->getSize());
    }
}

void LocalSearchSolverTest::testDP04s() {

    saturnin::LocalSearchSolver ls(1075, 3152);
    readInstanceAndFillLSSolver("instances/dp04s04.shuffled.cnf", ls);
    ls.setTabooLength(55);

    CPPUNIT_ASSERT(!ls.isModel());
    CPPUNIT_ASSERT(wUnknown == ls.getState());
    ls.solve(ls.getNbVar()*4);
    CPPUNIT_ASSERT(wFalse != ls.getState());
    CPPUNIT_ASSERT(0U < ls.getNbFlip());
}


