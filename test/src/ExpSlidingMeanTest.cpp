#include "ExpSlidingMeanTest.h"
#include "../../include/saturnin/ExpSlidingMean.h"

CPPUNIT_TEST_SUITE_REGISTRATION(ExpSlidingMeanTest);

void ExpSlidingMeanTest::testCreation() {
    saturnin::ExpSlidingMean m(3, 0.0);
    CPPUNIT_ASSERT_EQUAL(0.5, m.getAlpha());
    CPPUNIT_ASSERT_EQUAL(0.0, m.getMean());
}

void ExpSlidingMeanTest::testInsertion() {
    saturnin::ExpSlidingMean m(3, 0.0);
    CPPUNIT_ASSERT_EQUAL(0.5, m.getAlpha());
    CPPUNIT_ASSERT_EQUAL(0.0, m.getMean());
    
    for(int i=0; i<6; i++){
        m.addValue(1.0);
    }
    
    CPPUNIT_ASSERT_EQUAL(0.5, m.getAlpha());
    double dif = 1.0 - m.getMean();
    if(dif<0){
        dif *= -1;
    }
    //as we are working with doubles, equality checking is not a good idea
    //therefore, we check that we aren't too far from the actual value
    CPPUNIT_ASSERT(dif < 0.05);
    
}

void ExpSlidingMeanTest::testCopy() {
    saturnin::ExpSlidingMean m(3, 0.0);
    m.addValue(3.14159);
    
    saturnin::ExpSlidingMean a(m);
    CPPUNIT_ASSERT_EQUAL(m.getAlpha(), a.getAlpha());
    CPPUNIT_ASSERT_EQUAL(m.getMean(), a.getMean());
    
    saturnin::ExpSlidingMean b(2, 25);
    b = m;
    CPPUNIT_ASSERT_EQUAL(m.getAlpha(), b.getAlpha());
    CPPUNIT_ASSERT_EQUAL(m.getMean(), b.getMean());
}
