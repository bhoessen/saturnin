#define EXTRA_FRIEND ::ClauseTest
#include "ClauseTest.h"
#include "../../include/saturnin/Clause.h"
#include "../../include/saturnin/PoolList.h"

CPPUNIT_TEST_SUITE_REGISTRATION(ClauseTest);

void ClauseTest::testCreation(){
    const unsigned int sz = 4;
    saturnin::Lit c[sz];
    for(unsigned int i=0; i<sz; i++){
        c[i] = i;
    }
    saturnin::PoolList p(sz,2);
    saturnin::Clause* cl = p.createClause(c,sz,3);
    p.releaseClause(cl);

    char* mem[sizeof(saturnin::Clause)+((sz)*sizeof(saturnin::Lit))];
    saturnin::Clause* ccl = new (mem) saturnin::Clause(c, sz, sz-1);
    for(unsigned int i=0; i<sz; i++){
        CPPUNIT_ASSERT_EQUAL(c[i],ccl->getLit(i));
    }

}

void ClauseTest::testClauseSize(){
    //size and index
    size_t expected = sizeof(unsigned int)*2;
    //compact
    expected += sizeof(saturnin::Clause::compact_t);
    //data
    expected += sizeof(saturnin::Lit);
    CPPUNIT_ASSERT_EQUAL(expected, sizeof(saturnin::Clause));
    
}

void ClauseTest::testLitSetRemoval() {
    const unsigned int sz = 4;
    saturnin::Lit c[sz];
    for(unsigned int i=0; i<sz; i++){
        c[i] = i;
    }
    saturnin::PoolList p(sz,2);
    saturnin::Clause* cl = p.createClause(c,sz,3);
    CPPUNIT_ASSERT_EQUAL(sz, cl->getSize());
    cl->removeLit(2);
    CPPUNIT_ASSERT_EQUAL(sz-1, cl->getSize());
    CPPUNIT_ASSERT_EQUAL(3U, cl->getLit(2));
    cl->setLit(2,2);
    CPPUNIT_ASSERT_EQUAL(2U, cl->getLit(2));
    p.releaseClause(cl);
}


void ClauseTest::testOperators(){
    const unsigned int sz = 4;
    saturnin::Lit c[sz];
    for(unsigned int i=0; i<sz; i++){
        c[i] = i;
    }
    saturnin::PoolList p(sz,2);
    saturnin::Clause* cl = p.createClause(c,sz,3);

    saturnin::Clause& cla = *cl;
    const saturnin::Clause& clb = *cl;
    saturnin::Lit* litArray = cla;
    const saturnin::Lit* const litArrayConst = clb;
    for(unsigned int i=0; i<sz; i++){
        CPPUNIT_ASSERT_EQUAL(i,cla[i]);
        CPPUNIT_ASSERT_EQUAL(i,cla.getLit(i));
        CPPUNIT_ASSERT_EQUAL(i, litArray[i]);
        CPPUNIT_ASSERT_EQUAL(i, litArrayConst[i]);
    }
    p.releaseClause(cl);
}

void ClauseTest::testLBD(){
    const unsigned int sz = 4;
    saturnin::Lit c[sz];
    for(unsigned int i=0; i<sz; i++){
        c[i] = i;
    }
    saturnin::PoolList p(sz,2);
    saturnin::Clause* cl = p.createClause(c,sz,3);

    CPPUNIT_ASSERT_EQUAL((unsigned int)3, cl->getLBD());
    CPPUNIT_ASSERT_EQUAL(true, cl->isLearnt());

    cl->setLBD(0);
    CPPUNIT_ASSERT_EQUAL((unsigned int)0, cl->getLBD());
    CPPUNIT_ASSERT_EQUAL(false, cl->isLearnt());


    p.releaseClause(cl);
}

void ClauseTest::testAttached(){
    const unsigned int sz = 4;
    saturnin::Lit c[sz];
    for(unsigned int i=0; i<sz; i++){
        c[i] = i;
    }
    saturnin::PoolList p(sz,2);
    saturnin::Clause* cl = p.createClause(c,sz,3);
    CPPUNIT_ASSERT(cl->isAttached());
    cl->setAttached(false);
    CPPUNIT_ASSERT(!cl->isAttached());
    CPPUNIT_ASSERT_EQUAL(0U, cl->getFreezeCounter());
    cl->incrementFreezeCounter();
    CPPUNIT_ASSERT_EQUAL(1U, cl->getFreezeCounter());

}

void ClauseTest::testEquality(){
    {
        const unsigned int szA = 4;
        saturnin::Lit cA[szA];
        for(unsigned int i = 0; i<szA; i++ ){
            cA[i]=i;
        }
        saturnin::PoolList pA(szA,2);
        saturnin::Clause* clA = pA.createClause(cA,szA,3);

        const unsigned int szB = 4;
        saturnin::Lit cB[szB];
        for(unsigned int i = 0; i<szB; i++ ){
            cB[i]=szB-1-i;
        }
        saturnin::PoolList pB(szB,2);
        saturnin::Clause* clB = pB.createClause(cB,szB,3);

        CPPUNIT_ASSERT(clA->operator ==(*clB));
    }

    {
        const unsigned int szA = 4;
        saturnin::Lit cA[szA];
        for(unsigned int i = 0; i<szA; i++ ){
            cA[i]=i;
        }
        saturnin::PoolList pA(szA,2);
        saturnin::Clause* clA = pA.createClause(cA,szA,3);

        const unsigned int szB = 4;
        saturnin::Lit cB[szB];
        for(unsigned int i = 0; i<szB; i++ ){
            cB[i]=szB-1-i;
        }
        cB[3]=42;
        saturnin::PoolList pB(szB,2);
        saturnin::Clause* clB = pB.createClause(cB,szB,3);

        CPPUNIT_ASSERT(! clA->operator ==(*clB));
    }

    {
        const unsigned int szA = 3;
        saturnin::Lit cA[szA];
        for(unsigned int i = 0; i<szA; i++ ){
            cA[i]=i;
        }
        saturnin::PoolList pA(szA,2);
        saturnin::Clause* clA = pA.createClause(cA,szA,3);

        const unsigned int szB = 4;
        saturnin::Lit cB[szB];
        for(unsigned int i = 0; i<szB; i++ ){
            cB[i]=szB-1-i;
        }
        saturnin::PoolList pB(szB,2);
        saturnin::Clause* clB = pB.createClause(cB,szB,3);

        CPPUNIT_ASSERT(! clA->operator ==(*clB));
    }
}

