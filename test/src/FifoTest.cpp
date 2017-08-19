#include "FifoTest.h"
#include "../../include/saturnin/Fifo.h"
CPPUNIT_TEST_SUITE_REGISTRATION(FifoTest);

void FifoTest::testCreation() {
    saturnin::Fifo<int> f(12);
    CPPUNIT_ASSERT_EQUAL(0U, f.getSize());
    CPPUNIT_ASSERT_EQUAL(12U, f.getCapacity());

}

void FifoTest::testPushPop() {
    saturnin::Fifo<int> f(10);
    for (int i = 0; i < 5; i++) {
        f.push(i);
        CPPUNIT_ASSERT_EQUAL((unsigned int) i + 1, f.getSize());
    }

    for (int i = 0; i < 5; i++) {
        CPPUNIT_ASSERT_EQUAL(i, f.getFirst());
        f.pop();
        CPPUNIT_ASSERT_EQUAL(4U - (unsigned int) i, f.getSize());
    }

}

void FifoTest::testPushAround() {
    unsigned int cap = 10;
    saturnin::Fifo<unsigned int> f(cap);
    for (unsigned int i = 0; i < 5U; i++) {
        f.push(i);
        f.pop();
        CPPUNIT_ASSERT_EQUAL(0U, f.getSize());
    }

    for (unsigned int i = 0; i < cap; i++) {
        f.push(i);
        CPPUNIT_ASSERT_EQUAL(i + 1, f.getSize());
    }

    for (unsigned int i = 0; i < cap; i++) {
        CPPUNIT_ASSERT_EQUAL(i, f.getFirst());
        f.pop();
        CPPUNIT_ASSERT_EQUAL(cap - 1 - (unsigned int) i, f.getSize());
    }

    CPPUNIT_ASSERT_EQUAL(cap, f.getCapacity());
}

void FifoTest::testIncreaseCapacity() {

    {
        unsigned int cap = 10;
        saturnin::Fifo<unsigned int> f(cap);
        for (unsigned int i = 0; i < cap + 2; i++) {
            f.push(i);
            CPPUNIT_ASSERT_EQUAL(i + 1, f.getSize());
        }
        CPPUNIT_ASSERT(f.getCapacity() > cap);

        for (unsigned int i = 0; i < cap + 2; i++) {
            unsigned int j = f.getFirst();
            CPPUNIT_ASSERT_EQUAL(i, j);
            f.pop();
            CPPUNIT_ASSERT_EQUAL(cap + 1 - i, f.getSize());
        }
        CPPUNIT_ASSERT(f.getCapacity() > cap);
    }

    {
        //same test, but this time the first element won't be at position 0
        unsigned int cap = 10;
        saturnin::Fifo<unsigned int> f(cap);
        
        f.push(0);
        f.pop();
        f.push(0);
        f.pop();
        

        for (unsigned int i = 0; i < cap + 2; i++) {
            f.push(i);
            CPPUNIT_ASSERT_EQUAL(i + 1, f.getSize());
        }
        CPPUNIT_ASSERT(f.getCapacity() > cap);

        for (unsigned int i = 0; i < cap + 2; i++) {
            unsigned int j = f.getFirst();
            CPPUNIT_ASSERT_EQUAL(i, j);
            f.pop();
            CPPUNIT_ASSERT_EQUAL(cap + 1 - i, f.getSize());
        }
        CPPUNIT_ASSERT(f.getCapacity() > cap);
    }

}

void FifoTest::testCopy(){
    saturnin::Fifo<unsigned int> a(12);
    a.push(0);
    a.push(1);
    a.pop();
    a.push(2);
    a.push(3);
    saturnin::Fifo<unsigned int> b(a);
    saturnin::Fifo<unsigned int> c = a;
    CPPUNIT_ASSERT_EQUAL(a.getSize(), b.getSize());
    CPPUNIT_ASSERT_EQUAL(a.getSize(), c.getSize());
    CPPUNIT_ASSERT_EQUAL(a.getCapacity(), b.getCapacity());
    CPPUNIT_ASSERT_EQUAL(a.getCapacity(), c.getCapacity());
    while(a.getSize() > 0){
        CPPUNIT_ASSERT_EQUAL(a.getFirst(), b.getFirst());
        CPPUNIT_ASSERT_EQUAL(a.getFirst(), c.getFirst());
        a.pop();
        b.pop();
        c.pop();
    }
}
