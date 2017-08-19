#include "PoolListTest.h"
#include "../../include/saturnin/PoolList.h"
#include "../../include/saturnin/Clause.h"
#include "../../include/saturnin/VariablesManager.h"

CPPUNIT_TEST_SUITE_REGISTRATION(PoolListTest);

void PoolListTest::testCreation() {
    {
        unsigned int sz = 3;
        unsigned int cap = 5;
        saturnin::PoolList pool(sz, cap);
        CPPUNIT_ASSERT_EQUAL(sz, pool.getClausesLength());
        CPPUNIT_ASSERT_EQUAL(0U, pool.getNbAvailableClauses());
        CPPUNIT_ASSERT_EQUAL(0U, pool.getSize());
        CPPUNIT_ASSERT_EQUAL(0U, pool.getCapacity());
    }



}

void PoolListTest::testClauseRetrieval() {
    {
        unsigned int sz = 3;
        unsigned int cap = 5;
        saturnin::PoolList pool(sz, cap);
        saturnin::Lit literals[3] = {(saturnin::Lit)0, (saturnin::Lit)2, (saturnin::Lit)4};
        saturnin::Clause* a = pool.createClause(literals, sz);
        CPPUNIT_ASSERT_EQUAL(cap - 1, pool.getNbAvailableClauses());
        CPPUNIT_ASSERT_EQUAL((unsigned int) 1, pool.getSize());
        CPPUNIT_ASSERT_EQUAL(cap, pool.getCapacity());

        CPPUNIT_ASSERT_EQUAL((unsigned int) 0, a->getLBD());
        CPPUNIT_ASSERT_EQUAL(sz, a->getSize());
        pool.releaseClause(a);
        CPPUNIT_ASSERT_EQUAL((saturnin::Clause*)NULL, a);
    }

    {
        unsigned int sz = 3;
        unsigned int cap = 2;
        saturnin::PoolList pool(sz, cap);
        CPPUNIT_ASSERT_EQUAL(sz, pool.getClausesLength());
        CPPUNIT_ASSERT_EQUAL(0U, pool.getNbAvailableClauses());
        saturnin::Lit literals[3] = {(saturnin::Lit)0, (saturnin::Lit)2, (saturnin::Lit)4};
        CPPUNIT_ASSERT_EQUAL((unsigned int) 0, pool.getSize());
        CPPUNIT_ASSERT_EQUAL(0U, pool.getCapacity());
        saturnin::Clause* a = pool.createClause(literals, sz);
        CPPUNIT_ASSERT_EQUAL((unsigned int) 1, pool.getSize());
        CPPUNIT_ASSERT_EQUAL(cap, pool.getCapacity());
        CPPUNIT_ASSERT_EQUAL(cap - 1, pool.getNbAvailableClauses());

        CPPUNIT_ASSERT_EQUAL((unsigned int) 0, a->getLBD());
        CPPUNIT_ASSERT_EQUAL(sz, a->getSize());
        pool.releaseClause(a);
        CPPUNIT_ASSERT_EQUAL((saturnin::Clause*)NULL, a);
    }

}

void PoolListTest::testExtend() {
    const unsigned int sz = 3;
    const unsigned int cap = 2;
    saturnin::PoolList pool(sz, cap);
    saturnin::Lit literals[3] = {(saturnin::Lit)0, (saturnin::Lit)2, (saturnin::Lit)4};
    const unsigned int cArraySize = 2 * cap;
    saturnin::Clause * cArray[cArraySize];
    for (unsigned int i = 0; i < cap * 2; i++) {
        cArray[i] = pool.createClause(literals, sz);
        CPPUNIT_ASSERT_EQUAL(i + 1, pool.getSize());
    }
    unsigned int goal = cap + (unsigned int) (cap*SATURNIN_POOLLIST_UPDATE_FACTOR);
    CPPUNIT_ASSERT_EQUAL(goal, pool.getCapacity());
    CPPUNIT_ASSERT_EQUAL(goal - 2*cap, pool.getNbAvailableClauses());
    for (unsigned int i = 0; i < cap * 2; i++) {
        pool.releaseClause(cArray[i]);
    }
}

void PoolListTest::testMultipleClauses() {

    const unsigned int nbClauses = 2;
    const unsigned int sz = 2;
    saturnin::Clause * array[nbClauses];
    saturnin::PoolList pool(sz, nbClauses);
    

    saturnin::Lit * vs[2];
    saturnin::Lit val0[sz] = {(saturnin::Lit)0, (saturnin::Lit)2};
    vs[0] = val0;
    saturnin::Lit val1[sz] = {(saturnin::Lit)1, (saturnin::Lit)3};
    vs[1] = val1;

    for (unsigned int i = 0; i < 2; i++) {
        array[i] = pool.createClause(vs[i], sz, sz - 1);
    }

    for (unsigned int i = 0; i < nbClauses; i++) {
        CPPUNIT_ASSERT_EQUAL(sz, array[i]->getSize());
        CPPUNIT_ASSERT_EQUAL(sz - 1, array[i]->getLBD());
        for (unsigned int k = 0; k < sz; k++) {
            CPPUNIT_ASSERT_EQUAL(vs[i][k] , array[i]->getLit(k));
        }
        pool.releaseClause(array[i]);
    }

}
