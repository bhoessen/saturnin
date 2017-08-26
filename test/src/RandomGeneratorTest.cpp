#include "../../include/saturnin/RandomGenerator.h"
#include "RandomGeneratorTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(RandomGeneratorTest);

void RandomGeneratorTest::testComplete() {
    unsigned int nbUsed[256];
    for(unsigned int i = 0; i<256; i++){
        nbUsed[i]=0;
    }
    
    saturnin::RandomGenerator rand;
    for(unsigned int i = 0; i<256; i++){
        nbUsed[rand.getNext()]++;
    }
    
    for(unsigned int i = 0; i<256; i++){
        CPPUNIT_ASSERT_EQUAL(1U, nbUsed[i]);
    }
}

