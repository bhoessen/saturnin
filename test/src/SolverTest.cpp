#define _CRT_SECURE_NO_WARNINGS

#include "SolverTest.h"
#include "../../include/saturnin/Solver.h"
#include "../../include/saturnin/CNFReader.h"
#include "../../include/saturnin/Logger.h"

#include <cstdint>

CPPUNIT_TEST_SUITE_REGISTRATION(SolverTest);

void SolverTest::testCreation() {
    unsigned int nbVar = 2;
    saturnin::Solver s(nbVar, 4);
    CPPUNIT_ASSERT_EQUAL(nbVar, s.getNbVar());
    for (unsigned int i = 0; i < nbVar; i++) {
        CPPUNIT_ASSERT(wUnknown == s.getVarValue(i));
    }
}

void SolverTest::testClauseSizeAddition() {
    unsigned int nbVar = 311;
    unsigned int nbClauses = nbVar - 2;
    saturnin::Solver s(nbVar, nbClauses);
    saturnin::Array<saturnin::Lit> tmp(nbVar);
    for(unsigned int i = 0; i < nbVar; i++) {
        tmp.push(saturnin::VariablesManager::getLit(i, false));
    }
    for(unsigned int i = 0; i < nbClauses; i++) {
        s.addClause((saturnin::Lit*)tmp, nbVar - i);
        CPPUNIT_ASSERT_EQUAL(i + 1, s.getNbClauses());
    }
}


void SolverTest::testAddClause() {

    {
        //First test: add facts and a contradictory fact
        saturnin::Solver s(2, 4);

        saturnin::Lit a = 0;
        saturnin::Var va = saturnin::VariablesManager::getVar(a);
        saturnin::Lit na = 1;
        saturnin::Lit b = 2;
        saturnin::Var vb = saturnin::VariablesManager::getVar(b);

        CPPUNIT_ASSERT(s.addClause(&a, 1));
        CPPUNIT_ASSERT(wTrue == s.getVarValue(va));
        CPPUNIT_ASSERT(s.addClause(&b, 1));
        CPPUNIT_ASSERT(wTrue == s.getVarValue(vb));
        CPPUNIT_ASSERT(!s.addClause(&na, 1));
    }

    {
        //2 1 0
        //-1 0
        //3 -1 0
        //1 -2 -3 0
        saturnin::Lit a  = 0;
        saturnin::Lit na = 1;
        saturnin::Lit b  = 2;
        saturnin::Lit nb = 3;
        saturnin::Lit c  = 4;
        saturnin::Lit nc = 5;
        saturnin::Lit c1[] = { b,a};
        saturnin::Lit c2[] = {na};
        saturnin::Lit c3[] = {c,na};
        saturnin::Lit c4[] = {a,nb,nc};

        saturnin::Solver s(3,4);
        s.setVerbosity(0);
        CPPUNIT_ASSERT(s.addClause(c1,2));
        CPPUNIT_ASSERT(s.addClause(c2,1));
        CPPUNIT_ASSERT(s.addClause(c3,2));
        CPPUNIT_ASSERT(s.addClause(c4,3));
        CPPUNIT_ASSERT(s.getState() != wFalse);
        CPPUNIT_ASSERT(s.solve(1));
        CPPUNIT_ASSERT(s.getState() == wTrue);
    }

    {
        //Add a clause with some duplicate literals
        saturnin::Solver s(2, 4);

        saturnin::Lit a = 0;
        saturnin::Lit b = 2;
        saturnin::Lit array[] = {a, a, b};

        CPPUNIT_ASSERT(s.addClause(array, 3));
        CPPUNIT_ASSERT(wUnknown == s.getVarValue(saturnin::VariablesManager::getVar(a)));
        CPPUNIT_ASSERT(wUnknown == s.getVarValue(saturnin::VariablesManager::getVar(b)));
    }

    {
        //add a clause that will be empty
        saturnin::Solver s(2, 4);

        saturnin::Lit a = 0;
        saturnin::Lit na = 1;
        saturnin::Lit b = 2;
        saturnin::Lit nb = 3;
        saturnin::Lit array[] = {a, b};

        CPPUNIT_ASSERT(s.addClause(&na, 1));
        CPPUNIT_ASSERT(wFalse == s.getVarValue(saturnin::VariablesManager::getVar(a)));
        CPPUNIT_ASSERT(s.addClause(&nb, 1));
        CPPUNIT_ASSERT(wFalse == s.getVarValue(saturnin::VariablesManager::getVar(b)));
        CPPUNIT_ASSERT(!s.addClause(array, 2));
    }

    {
        //add a clause containing a and ¬a
        saturnin::Solver s(1, 4);

        saturnin::Lit a = 0;
        saturnin::Lit na = 1;
        saturnin::Lit array[] = {a, na, 2};

        CPPUNIT_ASSERT(s.addClause(array, 3));
        CPPUNIT_ASSERT(wUnknown == s.getVarValue(saturnin::VariablesManager::getVar(a)));
    }

    {
        //add a clause with a proven literal
        saturnin::Solver s(2, 4);

        saturnin::Lit a = 0;
        saturnin::Lit b = 2;
        saturnin::Lit array[] = {a, b};

        CPPUNIT_ASSERT(s.addClause(&b, 1));
        CPPUNIT_ASSERT(wTrue == s.getVarValue(saturnin::VariablesManager::getVar(b)));
        CPPUNIT_ASSERT(s.addClause(array, 2));
        CPPUNIT_ASSERT(wUnknown == s.getVarValue(saturnin::VariablesManager::getVar(a)));
    }

    {
        //add a clause that can be simplified to a fact
        saturnin::Solver s(2, 4);

        saturnin::Lit a = 0;
        saturnin::Lit b = 2;
        saturnin::Lit nb = 3;
        saturnin::Lit array[] = {a, b};

        CPPUNIT_ASSERT(s.addClause(&nb, 1));
        CPPUNIT_ASSERT(wFalse == s.getVarValue(saturnin::VariablesManager::getVar(b)));
        bool bb = s.addClause(array, 2);
        CPPUNIT_ASSERT(bb);
    }

    {
        //add a clause that can be simplified to the empty clause
        saturnin::Solver s(2,4);
        saturnin::Lit a = 0;
        saturnin::Lit na = 1;
        saturnin::Lit b = 2;
        saturnin::Lit nb = 3;
        saturnin::Lit c = 4;
        saturnin::Lit nc = 5;
        saturnin::Lit array[] = {a, b, c};
        CPPUNIT_ASSERT(s.addClause(&nb, 1));
        CPPUNIT_ASSERT(s.addClause(&na, 1));
        CPPUNIT_ASSERT(s.addClause(&nc, 1));
        CPPUNIT_ASSERT(wFalse == s.getVarValue(saturnin::VariablesManager::getVar(b)));
        CPPUNIT_ASSERT(!s.addClause(array, 3));
        CPPUNIT_ASSERT(s.getState() == wFalse);
    }

}

void SolverTest::testPropagation() {

    {
        //add a binary clause, then a fact
        //add a clause that can be simplified to a fact
        saturnin::Solver s(2, 4);

        saturnin::Lit a = 0;
        saturnin::Lit b = 2;
        saturnin::Lit nb = 3;
        saturnin::Lit array[] = {a, b};

        CPPUNIT_ASSERT(s.addClause(array, 2));
        CPPUNIT_ASSERT(wUnknown == s.getVarValue(saturnin::VariablesManager::getVar(a)));
        CPPUNIT_ASSERT(wUnknown == s.getVarValue(saturnin::VariablesManager::getVar(b)));
        CPPUNIT_ASSERT(s.addClause(&nb, 1));
        CPPUNIT_ASSERT(wTrue == s.getVarValue(saturnin::VariablesManager::getVar(a)));
        CPPUNIT_ASSERT(wFalse == s.getVarValue(saturnin::VariablesManager::getVar(b)));

    }

    {
        //force the lookup for a new watch
        //add a binary clause, then a fact
        //add a clause that can be simplified to a fact
        saturnin::Solver s(3, 4);

        saturnin::Lit a = 0;
        saturnin::Lit b = 2;
        saturnin::Lit nb = 3;
        saturnin::Lit c = 4;
        saturnin::Lit array[] = {a, b, c};

        CPPUNIT_ASSERT(s.addClause(array, 3));
        CPPUNIT_ASSERT(wUnknown == s.getVarValue(saturnin::VariablesManager::getVar(a)));
        CPPUNIT_ASSERT(wUnknown == s.getVarValue(saturnin::VariablesManager::getVar(b)));
        CPPUNIT_ASSERT(wUnknown == s.getVarValue(saturnin::VariablesManager::getVar(c)));
        CPPUNIT_ASSERT(s.addClause(&nb, 1));
        CPPUNIT_ASSERT(wUnknown == s.getVarValue(saturnin::VariablesManager::getVar(a)));
        CPPUNIT_ASSERT(wUnknown == s.getVarValue(saturnin::VariablesManager::getVar(c)));
        CPPUNIT_ASSERT(wFalse == s.getVarValue(saturnin::VariablesManager::getVar(b)));
    }


    {
        //force a propagation where the first watcher will be kept.
        //It can be done when the first watch has as other watched literal
        //a proven one

        saturnin::Solver s(4, 4);

        saturnin::Lit a = 0;
        saturnin::Lit na = 1;
        saturnin::Lit b = 2;
        saturnin::Lit nb = 3;
        saturnin::Lit c = 4;
        saturnin::Lit d = 6;
        saturnin::Lit clause2[] = {a, b};
        saturnin::Lit clause[] = {na, c};
        saturnin::Lit array[] = {na, d, c};


        CPPUNIT_ASSERT(s.addClause(array, 3));
        CPPUNIT_ASSERT(s.addClause(clause2, 2));
        CPPUNIT_ASSERT(s.addClause(clause, 2));
        CPPUNIT_ASSERT(s.addClause(clause, 2));

        CPPUNIT_ASSERT(wUnknown == s.getVarValue(saturnin::VariablesManager::getVar(a)));
        CPPUNIT_ASSERT(wUnknown == s.getVarValue(saturnin::VariablesManager::getVar(b)));
        CPPUNIT_ASSERT(wUnknown == s.getVarValue(saturnin::VariablesManager::getVar(c)));
        CPPUNIT_ASSERT(wUnknown == s.getVarValue(saturnin::VariablesManager::getVar(d)));
        CPPUNIT_ASSERT(s.addClause(&nb, 1));
        CPPUNIT_ASSERT(wTrue == s.getVarValue(saturnin::VariablesManager::getVar(a)));
        CPPUNIT_ASSERT(wFalse == s.getVarValue(saturnin::VariablesManager::getVar(b)));
        CPPUNIT_ASSERT(wTrue == s.getVarValue(saturnin::VariablesManager::getVar(c)));
        CPPUNIT_ASSERT(wUnknown == s.getVarValue(saturnin::VariablesManager::getVar(d)));
    }

    {
        //test a propagation that will lead to a conflict
        saturnin::Solver s(4, 4);

        saturnin::Lit a = 0;
        saturnin::Lit na = 1;
        saturnin::Lit b = 2;
        saturnin::Lit nb = 3;
        saturnin::Lit c = 4;
        saturnin::Lit nc = 5;
        saturnin::Lit d = 6;
        saturnin::Lit clause2[] = {a, b};
        saturnin::Lit clause[] = {na, b};
        saturnin::Lit array[] = {na, d, c};
        saturnin::Lit clause3[] = {na, b, nc};
        CPPUNIT_ASSERT(s.addClause(array, 3));
        CPPUNIT_ASSERT(s.addClause(clause2, 2));
        CPPUNIT_ASSERT(s.addClause(clause, 2));
        CPPUNIT_ASSERT(s.addClause(clause3, 2));
        CPPUNIT_ASSERT(!s.addClause(&nb, 1));
    }
}

void SolverTest::testSearchNoConflict() {

    {
        saturnin::Solver s(4, 4);
        s.setVerbosity(0);

        saturnin::Lit a = 0;
        saturnin::Lit na = 1;
        saturnin::Lit b = 2;
        saturnin::Lit nb = 3;
        saturnin::Lit c = 4;
        saturnin::Lit d = 6;
        saturnin::Lit clause2[] = {a, b};
        saturnin::Lit clause[] = {na, c};
        saturnin::Lit array[] = {na, d, c};


        s.addClause(array, 3);
        s.addClause(clause2, 2);
        s.addClause(clause, 2);
        s.addClause(clause, 2);
        s.addClause(&nb, 1);

        CPPUNIT_ASSERT(wTrue == s.getVarValue(saturnin::VariablesManager::getVar(a)));
        CPPUNIT_ASSERT(wFalse == s.getVarValue(saturnin::VariablesManager::getVar(b)));
        CPPUNIT_ASSERT(wTrue == s.getVarValue(saturnin::VariablesManager::getVar(c)));
        CPPUNIT_ASSERT(wUnknown == s.getVarValue(saturnin::VariablesManager::getVar(d)));

        CPPUNIT_ASSERT_EQUAL((unsigned long) 3, s.getNbPropagation());
        CPPUNIT_ASSERT_EQUAL((unsigned long) 0, s.getNbConflict());
        CPPUNIT_ASSERT_EQUAL((unsigned long) 0, s.getNbRestarts());
        CPPUNIT_ASSERT(s.solve(1));

        CPPUNIT_ASSERT(s.validate());

        CPPUNIT_ASSERT(wTrue == s.getVarValue(saturnin::VariablesManager::getVar(a)));
        CPPUNIT_ASSERT(wFalse == s.getVarValue(saturnin::VariablesManager::getVar(b)));
        CPPUNIT_ASSERT(wTrue == s.getVarValue(saturnin::VariablesManager::getVar(c)));
        CPPUNIT_ASSERT(wUnknown != s.getVarValue(saturnin::VariablesManager::getVar(d)));
    }

}

void SolverTest::testSearchConflict() {

    //This test may fail if the heuristic for choosing variables is changed

    saturnin::Lit a = 0;
    saturnin::Lit na = 1;
    saturnin::Lit b = 2;
    saturnin::Lit nb = 3;
    saturnin::Lit c = 4;
    saturnin::Lit nc = 5;

    {
        saturnin::Solver s(3, 4);
        s.setVerbosity(0);

        saturnin::Lit clause2[] = {c, nb};
        saturnin::Lit clause[] = {c, b};
        saturnin::Lit array[] = {a, b, c};
        saturnin::Lit clause3[] = {na, nb, nc};

        CPPUNIT_ASSERT(s.addClause(array, 3));
        CPPUNIT_ASSERT(s.addClause(clause, 2));
        CPPUNIT_ASSERT(s.addClause(clause2, 2));
        CPPUNIT_ASSERT(s.addClause(clause3, 3));
        bool solFound = s.solve(1);
        CPPUNIT_ASSERT(solFound);
        CPPUNIT_ASSERT(s.getState() == wTrue);

        CPPUNIT_ASSERT_EQUAL((unsigned long) 1, s.getNbConflict());

        CPPUNIT_ASSERT(s.validate());

    }

    {
        saturnin::Solver s(3, 4);
        s.setVerbosity(0);

        saturnin::Lit clause2[] = {c, b};
        saturnin::Lit clause[] = {nc, b};
        saturnin::Lit array[] = {a, b, c};

        CPPUNIT_ASSERT(s.addClause(array, 3));
        CPPUNIT_ASSERT(s.addClause(clause, 2));
        CPPUNIT_ASSERT(s.addClause(clause2, 2));
        bool solFound = s.solve(1);
        CPPUNIT_ASSERT(solFound);

        CPPUNIT_ASSERT_EQUAL((unsigned long) 0, s.getNbConflict());

        CPPUNIT_ASSERT(s.validate());

    }
}

void SolverTest::testSimplification(){
    {
        saturnin::Var x01 = 0;
        saturnin::Var x02 = 1;
        saturnin::Var x03 = 2;
        saturnin::Var x04 = 3;
        saturnin::Var x05 = 4;

        saturnin::Lit clause1[] = {x01, x02 * 2, x03 * 2, x04 * 2, x05 * 2};
        saturnin::Lit clause2[] = {x01, x02 * 2, x03 * 2, x04 * 2};
        saturnin::Lit clause3[] = {x01};
        saturnin::Lit clause4[] = {x02 * 2};
        saturnin::Lit clause5[] = {x03 * 2};

        saturnin::Solver s(5, 5);
        CPPUNIT_ASSERT(s.addClause(clause1, 5));
        CPPUNIT_ASSERT(s.addClause(clause2, 4));
        CPPUNIT_ASSERT(s.addClause(clause3, 1));
        CPPUNIT_ASSERT(s.addClause(clause4, 1));
        CPPUNIT_ASSERT(s.addClause(clause5, 1));

        CPPUNIT_ASSERT(s.simplify() == wTrue);
    }

    {
        saturnin::Var x01 = 0;
        saturnin::Var x02 = 1;
        saturnin::Var x03 = 2;
        saturnin::Var x04 = 3;
        saturnin::Var x05 = 4;

        saturnin::Lit clause1[] = {x01, x02 * 2, x03 * 2, x04 * 2, x05 * 2};
        saturnin::Lit clause2[] = {x01, x02 * 2, x03 * 2, x04 * 2 + 1};
        saturnin::Lit clause3[] = {x01 * 2 + 1 };

        saturnin::Solver s(5, 5);
        CPPUNIT_ASSERT(s.addClause(clause1, 5));
        CPPUNIT_ASSERT(s.addClause(clause2, 4));
        CPPUNIT_ASSERT(s.addClause(clause3, 1));

        saturnin::wbool v = s.simplify();
        CPPUNIT_ASSERT(v == wUnknown);
        
    }

    {
        saturnin::Var x01 = 0;
        saturnin::Var x02 = 1;

        saturnin::Lit clause1[] = {x01, x02 * 2};
        saturnin::Lit clause2[] = {x01, x02 * 2 + 1};
        saturnin::Lit clause3[] = {x01 * 2 + 1 };

        saturnin::Solver s(5, 5);
        CPPUNIT_ASSERT(s.addClause(clause1, 2));
        CPPUNIT_ASSERT(s.addClause(clause2, 2));
        CPPUNIT_ASSERT(!s.addClause(clause3, 1));

        saturnin::wbool v = s.simplify();
        CPPUNIT_ASSERT(v == wFalse);

    }

}

void SolverTest::testPhasedElimination() {
    {
        saturnin::Var x01 = 0;
        saturnin::Var x02 = 1;

        saturnin::Lit clause1[] = {x01, x02 * 2};

        saturnin::Solver s(5, 5);
        CPPUNIT_ASSERT(s.addClause(clause1, 2));

        s.phasedElimination();
        
        CPPUNIT_ASSERT(wTrue == s.getVarValue(x01));
        CPPUNIT_ASSERT(wTrue == s.getVarValue(x02));

    }

    {
        saturnin::Var x01 = 321;
        saturnin::Var x02 = 330;
        saturnin::Var x03 = 345;

        saturnin::Lit clause1[] = {
            saturnin::VariablesManager::getLit(x01,true),
            saturnin::VariablesManager::getLit(x02,false),
            saturnin::VariablesManager::getLit(x03,true)
        };

        saturnin::Solver s(345, 1);
        CPPUNIT_ASSERT(s.addClause(clause1, 3));

        CPPUNIT_ASSERT(wUnknown == s.phasedElimination());

    }
}

void SolverTest::testLargerProblem() {
    saturnin::Var x01 = 0;
    saturnin::Var x02 = 1;
    saturnin::Var x03 = 2;
    saturnin::Var x04 = 3;
    saturnin::Var x05 = 4;
    saturnin::Var x06 = 5;
    saturnin::Var x07 = 6;
    saturnin::Var x08 = 7;
    saturnin::Var x09 = 8;
    saturnin::Var x10 = 9;
    saturnin::Var x11 = 10;
    saturnin::Var x12 = 11;
    saturnin::Var x13 = 12;
    saturnin::Var x14 = 13;
    saturnin::Var x15 = 14;
    saturnin::Var x16 = 15;

    saturnin::Lit clause1[] = {x01, x02 * 2};
    saturnin::Lit clause2[] = {x02 * 2 + 1, x03 * 2 + 1};
    saturnin::Lit clause3[] = {x02 * 2 + 1, x04 * 2 + 1, x05 * 2 + 1};
    saturnin::Lit clause4[] = {x03 * 2, x05 * 2, x06 * 2};
    saturnin::Lit clause5[] = {x06 * 2 + 1, x07 * 2, x08 * 2};
    saturnin::Lit clause6[] = {x04 * 2 + 1, x08 * 2, x09 * 2};
    saturnin::Lit clause7[] = {x09 * 2 + 1, x10 * 2, x11 * 2};
    saturnin::Lit clause8[] = {x08 * 2, x11 * 2 + 1, x12 * 2 + 1};
    saturnin::Lit clause9[] = {x12 * 2, x13 * 2 + 1};
    saturnin::Lit clause10[] = {x12 * 2, x11 * 2};
    saturnin::Lit clause11[] = {x12 * 2, x06 * 2 + 1, x15 * 2};
    saturnin::Lit clause12[] = {x13 * 2, x14 * 2 + 1, x16 * 2 + 1};
    saturnin::Lit clause13[] = {x14 * 2 + 1, x15 * 2, x16 * 2};
    saturnin::Lit clause14[] = {x01 * 2, x02 * 2 + 1, x03 * 2, x04 * 2, x05 * 2, x06 * 2 + 1, x07 * 2, x08 * 2 + 1, x09 * 2, x10 * 2, x11 * 2, x12 * 2 + 1, x13 * 2, x14 * 2, x15 * 2, x16 * 2};


    saturnin::Solver s(16, 14);
    CPPUNIT_ASSERT(s.addClause(clause1, 2));
    CPPUNIT_ASSERT(s.addClause(clause2, 2));
    CPPUNIT_ASSERT(s.addClause(clause3, 3));
    CPPUNIT_ASSERT(s.addClause(clause4, 3));
    CPPUNIT_ASSERT(s.addClause(clause5, 3));
    CPPUNIT_ASSERT(s.addClause(clause6, 3));
    CPPUNIT_ASSERT(s.addClause(clause7, 3));
    CPPUNIT_ASSERT(s.addClause(clause8, 3));
    CPPUNIT_ASSERT(s.addClause(clause9, 2));
    CPPUNIT_ASSERT(s.addClause(clause10, 2));
    CPPUNIT_ASSERT(s.addClause(clause11, 3));
    CPPUNIT_ASSERT(s.addClause(clause12, 3));
    CPPUNIT_ASSERT(s.addClause(clause13, 3));
    CPPUNIT_ASSERT(s.addClause(clause14, 16));

    s.setVerbosity(0);

    CPPUNIT_ASSERT(s.solve(1));

    CPPUNIT_ASSERT(s.validate());

}

void SolverTest::createPigeonHoleProblem(unsigned int n, saturnin::Solver& s, bool print) {
    //Create the n+1 clause of size n
    for (unsigned int i = 0; i < n + 1; i++) {
        saturnin::Lit* clause = new saturnin::Lit[n];
        for (unsigned int j = 0; j < n; j++) {
            clause[j] = saturnin::VariablesManager::getLit((i * n) + j, true);
            if (print) {
                printf("%02d ", saturnin::VariablesManager::getVar(clause[j]));
            }
        }
        if (print) {
            printf("\n");
        }
        CPPUNIT_ASSERT(s.addClause(clause, n));
        delete[](clause);
    }

    //create the other clauses needed
    for (unsigned int j = 0; j < n; j++) {
        for (unsigned int i = 1; i <= n; i++) {
            for (unsigned int k = i + 1; k <= n + 1; k++) {
                saturnin::Lit clause[] = {
                    saturnin::VariablesManager::getLit(n * (i - 1) + j, false),
                    saturnin::VariablesManager::getLit(n * (k - 1) + j, false)
                };
                if (print) {
                    printf("-%02d -%02d\n",
                            saturnin::VariablesManager::getVar(clause[0]),
                            saturnin::VariablesManager::getVar(clause[1]));
                }
                CPPUNIT_ASSERT(s.addClause(clause, 2));
            }
        }
    }

}

void SolverTest::testPigeonHole() {
    saturnin::Solver p2(6, 9);
    p2.setVerbosity(0);
    createPigeonHoleProblem(2, p2);
    CPPUNIT_ASSERT(p2.solve(1));
    CPPUNIT_ASSERT(wFalse == p2.getState());

    saturnin::Solver p3(20, 45);
    p3.setVerbosity(0);
    createPigeonHoleProblem(3, p3);
    CPPUNIT_ASSERT(p3.solve(1));
    CPPUNIT_ASSERT(wFalse == p3.getState());

    saturnin::Solver p4(20, 45);
    p4.setVerbosity(0);
    createPigeonHoleProblem(4, p4);
    CPPUNIT_ASSERT(p4.solve(1));
    CPPUNIT_ASSERT(wFalse == p4.getState());
}

void SolverTest::testOneSolution3() {

    {
        saturnin::Solver p3(3, 7);
        createOneSolutionProblem(p3, 3);
        CPPUNIT_ASSERT_EQUAL(7U, p3.getNbClauses());

        p3.setVerbosity(0);

        CPPUNIT_ASSERT(p3.solve(1));
        CPPUNIT_ASSERT(wTrue == p3.getState());

        saturnin::Array<saturnin::Lit> c(3);
        for (unsigned int i = 0; i < 3U; i++) {
            CPPUNIT_ASSERT(wTrue == p3.getVarValue(i));
            c.push(saturnin::VariablesManager::getLit(i, false));
        }
    }


    //Check that if we add the negation of the answer, we won't obtain any
    //other answer
    {
        saturnin::Solver p3(3, 7);
        createOneSolutionProblem(p3, 3);
        CPPUNIT_ASSERT_EQUAL(7U, p3.getNbClauses());
        saturnin::Array<saturnin::Lit> c(3);
        for (unsigned int i = 0; i < 3U; i++) {
            c.push(saturnin::VariablesManager::getLit(i, false));
        }
        p3.addClause(c);

        p3.setVerbosity(0);

        CPPUNIT_ASSERT(p3.solve(1));
        CPPUNIT_ASSERT(wFalse == p3.getState());
    }


}

void SolverTest::testOneSolution7() {
    saturnin::Solver p7(7, 127);
    createOneSolutionProblem(p7, 7);
    CPPUNIT_ASSERT_EQUAL(127U, p7.getNbClauses());

    p7.setVerbosity(0);


    CPPUNIT_ASSERT(p7.solve(1));
    CPPUNIT_ASSERT(wTrue == p7.getState());
    for (unsigned int i = 0; i < 7U; i++) {
        CPPUNIT_ASSERT(wTrue == p7.getVarValue(i));
    }
}

void SolverTest::createOneSolutionProblem(saturnin::Solver& s, unsigned int nbVar) {
    if (nbVar > 31) {
        nbVar = 31;
    } else if (nbVar < 1) {
        nbVar = 1;
    }
    unsigned int nbClauses = (1 << nbVar) - 1;
    for (unsigned int i = 0; i < nbClauses; i++) {
        saturnin::Array<saturnin::Lit> c(nbVar);
        for (unsigned int j = 0; j < nbVar; j++) {
            bool sign = (i & (1 << j)) == 0;
            c.push(saturnin::VariablesManager::getLit(j, sign));
        }
        s.addClause(c);
    }
}

class FileHelper {
public:

    FileHelper(const char* fileName, const char* mode = "r") : f(NULL) {
        CPPUNIT_ASSERT(mode != NULL);
        f = fopen(fileName, mode);
        CPPUNIT_ASSERT(f != NULL);
    }

    FileHelper(FILE* aFile) : f(aFile) {
    }

    ~FileHelper() {
        fflush(f);
        fclose(f);
    }

    FILE* getFile() {
        return f;
    }

private:
    FileHelper& operator=(const FileHelper&);
    FileHelper(const FileHelper&);
    FILE* f;
};

void SolverTest::testPrintClauses() {

    const char* fileName = ".SolverTestTestPrintClauses~";

    saturnin::Solver s(3, 4);
    saturnin::Lit a = 0;
    saturnin::Lit b = 2;
    saturnin::Lit nb = 3;
    saturnin::Lit c = 4;
    saturnin::Lit nc = 5;
    saturnin::Lit clause2[] = {a, b, c};
    saturnin::Lit clause[] = {a, b, nc};


    CPPUNIT_ASSERT(s.addClause(clause2, 3));
    CPPUNIT_ASSERT(s.addClause(clause, 3));
    CPPUNIT_ASSERT(s.addClause(&nb, 1));
    CPPUNIT_ASSERT(s.addClause(&a, 1));
    FileHelper* help = new FileHelper(fileName, "w");
    s.printClauses(true, ", ", help->getFile());
    fprintf(help->getFile(), " | ");
    s.printClauses(false, ", ", help->getFile());
    delete(help);


    const char* expected = "\x1B[0;32m1\x1B[0;30m \x1B[0;30m3\x1B[0;30m \x1B[0;31m2\x1B[0;30m, \x1B[0;32m1\x1B[0;30m \x1B[0;30m-3\x1B[0;30m \x1B[0;31m2\x1B[0;30m | 1 3 2, 1 -3 2";

    const long bufSize = 1024;
    char buf[bufSize + 1];
    buf[bufSize] = '\0';
    FileHelper readFile(fileName);
    CPPUNIT_ASSERT(NULL != fgets(buf, bufSize, readFile.getFile()));
    CPPUNIT_ASSERT_EQUAL(strlen(expected), strlen(buf));
    for (size_t i = 0; i < strlen(expected); i++) {
        int e = expected[i];
        int actual = buf[i];
        char msg[256];
        sprintf((char*) msg, "At position %lu: %c instead of %c", (long unsigned int)i, actual, e);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, e, actual);
    }


}

void SolverTest::testSolveInstance(const char* fileName, saturnin::wbool result, const char* answer) const {
    saturnin::CNFReader* reader = new saturnin::CNFReader(fileName);

    saturnin::CNFReader::CNFReaderErrors error = reader->read();

    if (error != saturnin::CNFReader::cnfError_noError) {
        char buf[1024];
        snprintf(buf, 1024, "Error during file parsing: %s\n", saturnin::CNFReader::getErrorString(error));
        CPPUNIT_ASSERT_MESSAGE(buf, false);
    }

    unsigned int nbVar = reader->getNbVar();
    unsigned int nbClauses = reader->getNbClauses();

    saturnin::Solver s(nbVar, nbClauses);

    for (unsigned int i = 0; i < nbClauses; i++) {
        s.addClause(reader->getClause(i));
    }

    delete(reader);
    reader = NULL;

    s.setVerbosity(answer != NULL ? 1001 : 0);

    if (answer != NULL) {
        const char* sol = answer;
        saturnin::Array<saturnin::Lit> answerArray(s.getNbVar());
        bool stop = false;
        while (!stop) {
            char* tmp = NULL;
            long l = strtol(sol, &tmp, 10);
            if (l == 0) {
                stop = true;
            } else {
                int v = l;
                if (v < 0) {
                    v *= -1;
                }
                saturnin::Lit ll = saturnin::VariablesManager::getLit(v - 1, l > 0);
                answerArray.push(ll);
            }
            sol = tmp;
        }
        //s.addAnswer(answer);
    }

    CPPUNIT_ASSERT(s.solve(703));
    CPPUNIT_ASSERT(s.getState() == result);
    CPPUNIT_ASSERT(s.validate());
}

void SolverTest::testCompleteSAT() {
    const char* fileName = "instances/dp04s04.shuffled.cnf";
    testSolveInstance(fileName, wTrue);
}

void SolverTest::testCompleteUNSAT() {
    const char* fileName = "instances/dp04u03.shuffled.cnf";
    testSolveInstance(fileName, wFalse);
}

void SolverTest::testdp10() {
#ifndef VALGRIND //Way too long otherwise!
    const char* fileName = "instances/dp10s10.shuffled.cnf";
    testSolveInstance(fileName, wTrue);
#endif /* VALGRIND */
}

void SolverTest::testBigInstance() {
#ifndef VALGRIND //Way too long otherwise!
    const char* fileName = "instances/openstacks-cnfdd.cnf";
    testSolveInstance(fileName, wTrue);
#endif /* VALGRIND */
}

void SolverTest::testPresumedInstance() {
    const char* fileName = "instances/presumed.cnf";
    saturnin::CNFReader* reader = new saturnin::CNFReader(fileName);

    saturnin::CNFReader::CNFReaderErrors error = reader->read();

    if (error != saturnin::CNFReader::cnfError_noError) {
        char buf[1024];
        snprintf(buf, 1024, "Error during file parsing: %s\n", saturnin::CNFReader::getErrorString(error));
        CPPUNIT_ASSERT_MESSAGE(buf, false);
    }

    unsigned int nbVar = reader->getNbVar();
    unsigned int nbClauses = reader->getNbClauses();

    saturnin::Solver s(nbVar, nbClauses);

    for (unsigned int i = 0; i < nbClauses; i++) {
        s.addClause(reader->getClause(i));
    }

    const saturnin::Array<saturnin::Lit>& pres = reader->getPresumptions();
    for(unsigned int i=0; i< pres.getSize(); i++){
        s.addPresumption(saturnin::VariablesManager::getVar(pres[i]), saturnin::VariablesManager::getLitSign(pres[i]), true);
    }

    delete(reader);
    reader = NULL;

    s.setVerbosity(0);

    CPPUNIT_ASSERT(s.solve(703));
    CPPUNIT_ASSERT(s.getState() == wTrue);
    CPPUNIT_ASSERT(s.validate());
    CPPUNIT_ASSERT(s.getVarValue(0) == wTrue);
}

void SolverTest::testDBChecks() {
    const char* logFile = ".SolverTest.testDBChecks.log~";
    remove(logFile);
    FileHelper f(logFile, "w+");
    const char* dbFile = ".SolverTest.testDBChecks.db~";
    remove(dbFile);
    saturnin::Logger::getDBLogger().setOutput(f.getFile());
    saturnin::Solver p7(7, 127, dbFile);
    createOneSolutionProblem(p7, 7);
    CPPUNIT_ASSERT_EQUAL(127U, p7.getNbClauses());

    p7.setVerbosity(0);
    p7.solve(5U);
    bool valid = p7.validate();
    saturnin::Logger::getDBLogger().setOutput(saturnin::Logger::getStdOutLogger().getOutput());
    CPPUNIT_ASSERT(valid);
    
}

void SolverTest::testMemoryUsage(){
    saturnin::Solver p7(7, 127);
    createOneSolutionProblem(p7, 7);
    CPPUNIT_ASSERT_EQUAL(127U, p7.getNbClauses());

    p7.setVerbosity(1);

    {
#if defined _WIN32 || defined __CYGWIN__
        FILE* f = fopen("nul", "w+");
#else
        FILE* f = fopen("/dev/null", "w+");
#endif
        FILE* original = saturnin::Logger::getStdOutLogger().getOutput();
        saturnin::Logger::getStdOutLogger().setOutput(f);
        CPPUNIT_ASSERT(p7.solve(1));
        saturnin::Logger::getStdOutLogger().setOutput(original);
        fclose(f);
    }
    CPPUNIT_ASSERT(wTrue == p7.getState());
    for (unsigned int i = 0; i < 7U; i++) {
        CPPUNIT_ASSERT(wTrue == p7.getVarValue(i));
    }
    
    size_t expected = sizeof(int) == sizeof(std::intptr_t) ? 35787 : 55563;
    if (sizeof(long) == 8)
        expected += 60;
#ifdef SATURNIN_DB
    expected += (sizeof(int) == sizeof(std::intptr_t) ? 12 : 24);
#endif /* SATURNIN_DB */
#ifdef PROFILE
    expected += 216;
#endif /* PROFILE */
#ifdef SATURNIN_PARALLEL
    expected += (sizeof(int) == sizeof(std::intptr_t) ? 20 : 40);
#ifdef PROFILE
    expected += 16;
#endif /* PROFILE */
#endif /* SATURNIN_PARALLEL */
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Changes has been made to the memory model of the solver? If so, you may change the expected value.",
       expected, sizeof(saturnin::Solver) + p7.getMemoryFootprint());

}
