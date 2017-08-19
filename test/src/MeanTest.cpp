#include "MeanTest.h"
#include "../../include/saturnin/Mean.h"

CPPUNIT_TEST_SUITE_REGISTRATION(MeanTest);

void MeanTest::testCreation(){
    saturnin::Mean m;
    CPPUNIT_ASSERT_EQUAL((unsigned long int)0, m.getNbValues());
}
    
void MeanTest::testInsertion(){
    saturnin::Mean m;
    for(int i=0; i<=10; i++){
        m.addValue(i);
    }
    CPPUNIT_ASSERT_EQUAL((unsigned long int)11, m.getNbValues());
    
}
    
void MeanTest::testMean(){
    saturnin::Mean m;
    for(int i=0; i<=10; i++){
        m.addValue(i);
    }
    CPPUNIT_ASSERT_EQUAL(5.0,m.getMean());
}

void MeanTest::testCopy(){
    saturnin::Mean m;
    m.addValue(2.0);
    
    saturnin::Mean a;
    a = m;
    CPPUNIT_ASSERT_EQUAL(m.getMean(), a.getMean());
    CPPUNIT_ASSERT_EQUAL(m.getNbValues(), a.getNbValues() );
    
    saturnin::Mean b(m);
    CPPUNIT_ASSERT_EQUAL(m.getMean(), b.getMean());
    CPPUNIT_ASSERT_EQUAL(m.getNbValues(), b.getNbValues() );
}
