#include "SlidingMeanTest.h"
#include "../../include/saturnin/SlidingMean.h"

CPPUNIT_TEST_SUITE_REGISTRATION(SlidingMeanTest);

void SlidingMeanTest::testCreation() {
    saturnin::SlidingMean m(5);
    CPPUNIT_ASSERT(!m.isFull());
}

void SlidingMeanTest::testInsertion() {
    unsigned int cap = 5;
    saturnin::SlidingMean m(cap);
    for (unsigned int i = 0; i < cap * 2; i++) {
        CPPUNIT_ASSERT(((i >= cap) == m.isFull()));
        m.addValue(1.0);
    }
}

void SlidingMeanTest::testMean() {
    unsigned int cap = 5;
    saturnin::SlidingMean m(cap);
    for (unsigned int i = 0; i < cap; i++) {
        m.addValue(1.0);
        CPPUNIT_ASSERT_EQUAL(1.0, m.getMean());
    }

    m.addValue(6.0);
    CPPUNIT_ASSERT_EQUAL(2.0, m.getMean());
}

void SlidingMeanTest::testReset() {
    unsigned int cap = 5;
    saturnin::SlidingMean m(cap);
    for (unsigned int i = 0; i < cap; i++) {
        m.addValue(1.0);
        CPPUNIT_ASSERT_EQUAL(1.0, m.getMean());
        CPPUNIT_ASSERT_EQUAL(i+1, m.getNbValues());
    }
    CPPUNIT_ASSERT(m.isFull());
    
    m.reset();
    CPPUNIT_ASSERT(!m.isFull());
    m.addValue(0.0);
    CPPUNIT_ASSERT_EQUAL(0.0, m.getMean());
    
}

void SlidingMeanTest::testCopy(){
    unsigned int cap = 5;
    saturnin::SlidingMean m(cap);
    for(unsigned int i=0; i<cap-1; i++){
        m.addValue(1.0);
    }
    CPPUNIT_ASSERT(!m.isFull());
    CPPUNIT_ASSERT_EQUAL(1.0, m.getMean());
    
    saturnin::SlidingMean a;
    a = m;
    CPPUNIT_ASSERT(!a.isFull());
    CPPUNIT_ASSERT_EQUAL(1.0, a.getMean());
    CPPUNIT_ASSERT_EQUAL(m.getNbValues(), a.getNbValues());
    a.addValue(1.0);
    CPPUNIT_ASSERT(a.isFull());
    saturnin::SlidingMean b(m);
    CPPUNIT_ASSERT(!b.isFull());
    CPPUNIT_ASSERT_EQUAL(1.0, b.getMean());
    b.addValue(1.0);
    CPPUNIT_ASSERT(b.isFull());
}
