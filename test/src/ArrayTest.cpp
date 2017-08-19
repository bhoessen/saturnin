#include "ArrayTest.h"
#include "../../include/saturnin/Array.h"

CPPUNIT_TEST_SUITE_REGISTRATION(ArrayTest);

void ArrayTest::testCreation() {
    saturnin::Array<int> a;
    CPPUNIT_ASSERT_EQUAL((unsigned int) 0, a.getSize());
    saturnin::Array<int> b(32);
    CPPUNIT_ASSERT_EQUAL((unsigned int) 0, b.getSize());
}

void ArrayTest::testPush() {
    int arrayCap = 4;
    saturnin::Array<int> a(arrayCap);
    for (int i = 0; i < 2 * arrayCap; i++) {
        a.push(i);
        CPPUNIT_ASSERT_EQUAL((unsigned int) (i + 1), a.getSize());
        CPPUNIT_ASSERT_EQUAL(i, a.getLast());
    }
}

void ArrayTest::testPushResize(){
    unsigned int arrayCap = 4;
    saturnin::Array<int> a(arrayCap);
    for(unsigned int i=0; i<arrayCap; i++){
        a.push(i);
    }
    CPPUNIT_ASSERT_EQUAL(arrayCap, a.getCapacity());
    CPPUNIT_ASSERT_EQUAL(arrayCap, a.getSize());
    a.push(a.get(0));
    CPPUNIT_ASSERT_EQUAL(a.get(0), a.getLast());

    saturnin::Array<saturnin::Array<int> > b(arrayCap);
    for(unsigned int i=0; i<arrayCap; i++){
        b.push();
    }
    CPPUNIT_ASSERT_EQUAL(arrayCap, b.getCapacity());
    CPPUNIT_ASSERT_EQUAL(arrayCap, b.getSize());
    b.push(b.get(0));
    CPPUNIT_ASSERT_EQUAL(b.get(0).getSize(), b.getLast().getSize());
}

void ArrayTest::testGet() {
    unsigned int arrayCap = 4;
    saturnin::Array<unsigned int> a(arrayCap);
    for (unsigned int i = 0; i < arrayCap; i++) {
        a.push(i);
        CPPUNIT_ASSERT_EQUAL(i, a.get(i));
    }
}

void ArrayTest::testOperator() {
    unsigned int arrayCap = 4;
    saturnin::Array<unsigned int> a(arrayCap);
    for (unsigned int i = 0; i < arrayCap; i++) {
        a.push(i);
        CPPUNIT_ASSERT_EQUAL(i, a[i]);
        CPPUNIT_ASSERT_EQUAL(i, ((unsigned int*)a)[i]);
    }

    const saturnin::Array<unsigned int>& b = a;
    const unsigned int* const carray = b;
    for (unsigned int i = 0; i < arrayCap; i++) {
        CPPUNIT_ASSERT_EQUAL(i, carray[i]);
    }
}

void ArrayTest::testPop() {
    unsigned int arrayCap = 4;
    saturnin::Array<unsigned int> a(arrayCap);
    for (unsigned int i = 0; i < arrayCap; i++) {
        a.push(i);
    }

    unsigned int prevSize = a.getSize();
    while(prevSize > 0){
        a.pop();
        CPPUNIT_ASSERT_EQUAL(prevSize-1,a.getSize());
        prevSize--;
        for(unsigned int i=0; i<prevSize; i++){
            CPPUNIT_ASSERT_EQUAL(i, a[i]);
        }
    }
}

void ArrayTest::testPopALot(){
    unsigned int arrayCap = 4;
    saturnin::Array<unsigned int> a(arrayCap);
    for (unsigned int i = 0; i < arrayCap; i++) {
        a.push(i);
    }
    
    a.pop(2);
    CPPUNIT_ASSERT_EQUAL((unsigned int)2, a.getSize());
    CPPUNIT_ASSERT_EQUAL((unsigned int)1, a[1U]);
    CPPUNIT_ASSERT_EQUAL((unsigned int)0, a[0U]);
    
    a.pop(4);
    CPPUNIT_ASSERT_EQUAL((unsigned int)0, a.getSize());
}

void ArrayTest::testResize() {
    unsigned int arrayCap = 8;
    saturnin::Array<unsigned int> a(arrayCap / 2);
    for (unsigned int i = 0; i < arrayCap; i++) {
        a.push(i);
        for (unsigned int j = 0; j <= i; j++) {
            CPPUNIT_ASSERT_EQUAL(j, a.get(j));
        }
    }
}

void ArrayTest::testCopy(){
    unsigned int arrayCap = 4;
    saturnin::Array<unsigned int> a(arrayCap);
    for (unsigned int i = 0; i < arrayCap; i++) {
        a.push(i);
    }

    saturnin::Array<unsigned int> b(a);
    CPPUNIT_ASSERT_EQUAL(a.getSize(), b.getSize());
    for (unsigned int i = 0; i < a.getSize(); i++) {
        CPPUNIT_ASSERT_EQUAL(a.get(i), b.get(i));
    }

    saturnin::Array<unsigned int> c;
    c = a;
    CPPUNIT_ASSERT_EQUAL(a.getSize(), c.getSize());
    for (unsigned int i = 0; i < a.getSize(); i++) {
        CPPUNIT_ASSERT_EQUAL(a.get(i), c.get(i));
    }
}

void ArrayTest::testInitializerList(){
    saturnin::Array<unsigned int> a{0U, 1U, 3U};
    CPPUNIT_ASSERT_EQUAL(3U, a.getSize());
    CPPUNIT_ASSERT_EQUAL(0U, a[0U]);
    CPPUNIT_ASSERT_EQUAL(1U, a[1U]);
    CPPUNIT_ASSERT_EQUAL(3U, a[2U]);
}

/**
 * Just an object that will be used to test if arrays can contain properly
 * pointers
 */
class AnObject{
public:
    /**
     * Constructor
     * @param v the value that will be stored in our object
     */
    AnObject(unsigned int v) : a(v){
    }
    
    unsigned int a;
};

void ArrayTest::testPointers(){
    unsigned int arrayCap = 10;
    saturnin::Array<AnObject*> a(arrayCap);
    for(unsigned int i=0; i<arrayCap; i++){
        a.push(new AnObject(i));
    }
    CPPUNIT_ASSERT_EQUAL(arrayCap, a.getSize());
    for(unsigned int i=0; i<arrayCap; i++){
        CPPUNIT_ASSERT_EQUAL(i, a[i]->a);
    }
    saturnin::Array<AnObject*> b(a);
    CPPUNIT_ASSERT(a == b);
    saturnin::Array<AnObject*> d(a);
    CPPUNIT_ASSERT(d == a);
    for(unsigned int i=0; i<arrayCap; i++){
        delete(a.getLast());
        a.pop();
    }
    CPPUNIT_ASSERT_EQUAL(0U, a.getSize());
    
    AnObject tmp(2);
    saturnin::Array<AnObject*> c{NULL, &tmp};
    CPPUNIT_ASSERT_EQUAL(2U, c.getSize());
}

