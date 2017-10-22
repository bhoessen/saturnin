#include "HeapTest.h"
#include "../../include/saturnin/Heap.h"


CPPUNIT_TEST_SUITE_REGISTRATION(HeapTest);

class HeapTestComp {
public:

    HeapTestComp(const saturnin::Array<double>& v) : values(v) {
    }

    HeapTestComp(const HeapTestComp& other) : values(other.values) {
    }

    inline double compare(saturnin::Var a, saturnin::Var b) const {
        return values.get(a) - values.get(b);
    }

    /**
     * Retrieve the value associated with a given key
     * @param key the key we want its value
     * @return the value related to @a key
     */
    inline double operator[](saturnin::Var key) const {
        return values.get(key);
    }
private:

    const saturnin::Array<double>& values;
};

void HeapTest::testCreation() {
    saturnin::Array<double> values;
    saturnin::Heap<double, HeapTestComp> h(64, HeapTestComp(values));
    CPPUNIT_ASSERT_EQUAL((unsigned int) 0, h.getSize());
}

void HeapTest::testInsertion() {
    {
        //increasing order
        saturnin::Array<double> values;
        saturnin::Heap<double, HeapTestComp> h(64, HeapTestComp(values));
        for (unsigned int i = 0; i < 10; i++) {
            values.push(i * 1.0);
            h.add(i);
            CPPUNIT_ASSERT_EQUAL(i + 1, h.getSize());
            for (unsigned int j = 0; j <= i; j++) {
                CPPUNIT_ASSERT_EQUAL(j * 1.0, h[j]);
            }
#ifdef DEBUG
            CPPUNIT_ASSERT(h.check());
#endif 
        }
    }

    {
        //decreasing order
        saturnin::Array<double> values;
        saturnin::Heap<double, HeapTestComp> h2(64, HeapTestComp(values));
        for (unsigned int i = 0; i < 10; i++) {
            values.push(10 - (i * 1.0));
            h2.add(i);
            CPPUNIT_ASSERT_EQUAL(i + 1, h2.getSize());
            for (unsigned int j = 0; j <= i; j++) {
                CPPUNIT_ASSERT_EQUAL(10 - (j * 1.0), h2[j]);
            }
#ifdef DEBUG
            CPPUNIT_ASSERT(h2.check());
#endif
        }
    }

}

void HeapTest::testOperator() {
    saturnin::Array<double> values;
    saturnin::Heap<double, HeapTestComp> h(64, HeapTestComp(values));
    for (unsigned int i = 0; i < 10; i++) {
        values.push(i * 1.0);
        h.add(i);
        CPPUNIT_ASSERT_EQUAL(i + 1, h.getSize());
    }
    for (unsigned int i = 0; i < 10; i++) {
        CPPUNIT_ASSERT_EQUAL(i * 1.0, h[i]);
    }
}

void HeapTest::testGetMin() {
    unsigned int heapSize = 12;
    saturnin::Array<double> values(heapSize);
    saturnin::Heap<double, HeapTestComp> h(heapSize, HeapTestComp(values));
    for (unsigned int i = 0; i < heapSize; i++) {
        double newValue = (heapSize - i)*1.0;
        values.push(newValue);
        h.add(i);
        saturnin::Var m = h.getMin();
        CPPUNIT_ASSERT_EQUAL(i, m);
    }

    for (unsigned int i = 0; i < heapSize; i++) {
        CPPUNIT_ASSERT_EQUAL((heapSize - i)*1.0, h[i]);
    }

    double min = h[h.getMin()];
    unsigned int size = h.getSize();
    while (size > 1) {
        h.removeMin();
        double newMin = h[h.getMin()];
        CPPUNIT_ASSERT(min < newMin);
        CPPUNIT_ASSERT_EQUAL(size - 1, h.getSize());
        min = newMin;
        size = h.getSize();
    }
    h.removeMin();
    CPPUNIT_ASSERT_EQUAL((unsigned int) 0, h.getSize());
}

void HeapTest::testUpdate() {
    unsigned int heapSize = 12;
    saturnin::Array<double> values(heapSize);
    saturnin::Heap<double, HeapTestComp> h(heapSize, HeapTestComp(values));
    //fill the heap
    for (unsigned int i = 0; i < heapSize; i++) {
        double newValue = (heapSize - i)*1.0;
        values.push(newValue);
        h.add(i);
        CPPUNIT_ASSERT_EQUAL(i + 1, h.getSize());
        CPPUNIT_ASSERT_EQUAL(i, h.getMin());
    }

#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    //update the var with the greatest value (heapSize) to -1
    saturnin::Var v = (unsigned int) 0;
    CPPUNIT_ASSERT_EQUAL(values[v], h[v]);
    values[v] = -1.0;
    h.updateValue(v);
    CPPUNIT_ASSERT_EQUAL(v, h.getMin());

#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[v] = (heapSize)*1.0;
    h.updateValue(v);
    CPPUNIT_ASSERT_EQUAL(heapSize - 1, h.getMin());

    for (unsigned int i = 0; i < values.getSize(); i++) {
        values[i] = 0.5 * values[i];
    }

    for (unsigned int i = 0; i < heapSize; i++) {
        CPPUNIT_ASSERT_EQUAL((heapSize - i)*0.5, h[i]);
    }

}

void HeapTest::testPresent() {
    unsigned int heapSize = 3;
    saturnin::Array<double> values(heapSize);
    values.push(1.0);
    values.push(1.0);
    values.push(1.0);
    saturnin::Heap<double, HeapTestComp> h(heapSize, HeapTestComp(values));
    h.add(0);
    h.add(1);
    h.add(2);
    CPPUNIT_ASSERT_EQUAL((unsigned int) 0, h.removeMin());
    CPPUNIT_ASSERT_EQUAL((unsigned int) 2, h.removeMin());
    CPPUNIT_ASSERT(h.contains((unsigned int) 1));
}

void HeapTest::testContainLong() {
    {
    saturnin::Array<double> values(20);
    for (unsigned int i = 0; i < 20; i++) {
        values.push(0);
    }
    saturnin::Heap<double, HeapTestComp> h(20, HeapTestComp(values));
    values[0U] = 0;
    h.add((saturnin::Var) 0);
    values[1U] = 0;
    h.add((saturnin::Var) 1);
    values[2U] = 0;
    h.add((saturnin::Var) 2);
    values[3U] = 0;
    h.add((saturnin::Var) 3);
    values[4U] = 0;
    h.add((saturnin::Var) 4);
    values[5U] = 0;
    h.add((saturnin::Var) 5);
    values[6U] = 0;
    h.add((saturnin::Var) 6);
    values[7U] = 0;
    h.add((saturnin::Var) 7);
    values[8U] = 0;
    h.add((saturnin::Var) 8);
    values[9U] = 0;
    h.add((saturnin::Var) 9);
    values[10U] = 0;
    h.add((saturnin::Var) 10);
    values[11U] = 0;
    h.add((saturnin::Var) 11);
    values[12U] = 0;
    h.add((saturnin::Var) 12);
    values[13U] = 0;
    h.add((saturnin::Var) 13);
    values[14U] = 0;
    h.add((saturnin::Var) 14);
    values[15U] = 0;
    h.add((saturnin::Var) 15);
    values[16U] = 0;
    h.add((saturnin::Var) 16);
    values[17U] = 0;
    h.add((saturnin::Var) 17);
    values[18U] = 0;
    h.add((saturnin::Var) 18);
    values[19U] = 0;
    h.add((saturnin::Var) 19);

    CPPUNIT_ASSERT_EQUAL(20U, h.getSize());
    
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[0U] = 1;
    h.updateValue((saturnin::Var) 0);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    
    values[1U] = 1;
    h.updateValue((saturnin::Var) 1);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    
    values[2U] = 1;
    h.updateValue((saturnin::Var) 2);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    
    values[3U] = 1;
    h.updateValue((saturnin::Var) 3);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    
    values[4U] = 1;
    h.updateValue((saturnin::Var) 4);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    
    values[5U] = 1;
    h.updateValue((saturnin::Var) 5);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    
    values[6U] = 1;
    h.updateValue((saturnin::Var) 6);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    
    values[7U] = 1;
    h.updateValue((saturnin::Var) 7);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    
    values[8U] = 1;
    h.updateValue((saturnin::Var) 8);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    
    values[9U] = 1;
    h.updateValue((saturnin::Var) 9);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    
    values[10U] = 1;
    h.updateValue((saturnin::Var) 10);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    values[11U] = 1;
    h.updateValue((saturnin::Var) 11);
    values[0U] = 2;
    h.updateValue((saturnin::Var) 0);
    values[3U] = 2;
    h.updateValue((saturnin::Var) 3);
    values[0U] = 3;
    h.updateValue((saturnin::Var) 0);
    values[6U] = 2;
    h.updateValue((saturnin::Var) 6);
    values[0U] = 4;
    h.updateValue((saturnin::Var) 0);
    values[9U] = 2;
    h.updateValue((saturnin::Var) 9);
    values[3U] = 3;
    h.updateValue((saturnin::Var) 3);
    values[6U] = 3;
    h.updateValue((saturnin::Var) 6);
    values[3U] = 4;
    h.updateValue((saturnin::Var) 3);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    values[9U] = 3;
    h.updateValue((saturnin::Var) 9);
    values[6U] = 4;
    h.updateValue((saturnin::Var) 6);
    values[9U] = 4;
    h.updateValue((saturnin::Var) 9);
    values[1U] = 2;
    h.updateValue((saturnin::Var) 1);
    values[4U] = 2;
    h.updateValue((saturnin::Var) 4);
    values[1U] = 3;
    h.updateValue((saturnin::Var) 1);
    values[7U] = 2;
    h.updateValue((saturnin::Var) 7);
    values[1U] = 4;
    h.updateValue((saturnin::Var) 1);
    values[10U] = 2;
    h.updateValue((saturnin::Var) 10);
    values[4U] = 3;
    h.updateValue((saturnin::Var) 4);
    values[7U] = 3;
    h.updateValue((saturnin::Var) 7);
    values[4U] = 4;
    h.updateValue((saturnin::Var) 4);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    values[10U] = 3;
    h.updateValue((saturnin::Var) 10);
    values[7U] = 4;
    h.updateValue((saturnin::Var) 7);
    values[10U] = 4;
    h.updateValue((saturnin::Var) 10);
    values[2U] = 2;
    h.updateValue((saturnin::Var) 2);
    values[5U] = 2;
    h.updateValue((saturnin::Var) 5);
    values[2U] = 3;
    h.updateValue((saturnin::Var) 2);
    values[8U] = 2;
    h.updateValue((saturnin::Var) 8);
    values[2U] = 4;
    h.updateValue((saturnin::Var) 2);
    values[11U] = 2;
    h.updateValue((saturnin::Var) 11);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    values[5U] = 3;
    h.updateValue((saturnin::Var) 5);
    values[8U] = 3;
    h.updateValue((saturnin::Var) 8);
    values[5U] = 4;
    h.updateValue((saturnin::Var) 5);
    values[11U] = 3;
    h.updateValue((saturnin::Var) 11);
    values[8U] = 4;
    h.updateValue((saturnin::Var) 8);
    values[11U] = 4;
    h.updateValue((saturnin::Var) 11);

#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    }
    
    {
    saturnin::Array<double> values(20);
    for (unsigned int i = 0; i < 20; i++) {
        values.push(0);
    }
    saturnin::Heap<double, HeapTestComp> h(20, HeapTestComp(values));
    values[0U] = 0;
    h.add(0);
    values[1U] = 0;
    h.add(1);
    values[2U] = 0;
    h.add(2);
    values[3U] = 0;
    h.add(3);
    values[4U] = 0;
    h.add(4);
    values[5U] = 0;
    h.add(5);
    values[6U] = 0;
    h.add(6);
    values[7U] = 0;
    h.add(7);
    values[8U] = 0;
    h.add(8);
    values[9U] = 0;
    h.add(9);
    values[10U] = 0;
    h.add(10);
    values[11U] = 0;
    h.add(11);
    values[12U] = 0;
    h.add(12);
    values[13U] = 0;
    h.add(13);
    values[14U] = 0;
    h.add(14);
    values[15U] = 0;
    h.add(15);
    values[16U] = 0;
    h.add(16);
    values[17U] = 0;
    h.add(17);
    values[18U] = 0;
    h.add(18);
    values[19U] = 0;
    h.add(19);
    values[0U] = 1;
    h.updateValue(0);
    values[1U] = 1;
    h.updateValue(1);
    values[2U] = 1;
    h.updateValue(2);
    values[3U] = 1;
    h.updateValue(3);
    values[4U] = 1;
    h.updateValue(4);
    values[5U] = 1;
    h.updateValue(5);
    values[6U] = 1;
    h.updateValue(6);
    values[7U] = 1;
    h.updateValue(7);
    values[8U] = 1;
    h.updateValue(8);
    values[9U] = 1;
    h.updateValue(9);
    values[10U] = 1;
    h.updateValue(10);
    values[11U] = 1;
    h.updateValue(11);
    values[12U] = 1;
    h.updateValue(12);
    values[13U] = 1;
    h.updateValue(13);
    values[14U] = 1;
    h.updateValue(14);
    values[15U] = 1;
    h.updateValue(15);
    values[16U] = 1;
    h.updateValue(16);
    values[17U] = 1;
    h.updateValue(17);
    values[18U] = 1;
    h.updateValue(18);
    values[19U] = 1;
    h.updateValue(19);
    values[0U] = 2;
    h.updateValue(0);
    values[4U] = 2;
    h.updateValue(4);
    values[0U] = 3;
    h.updateValue(0);
    values[8U] = 2;
    h.updateValue(8);
    values[0U] = 4;
    h.updateValue(0);
    values[12U] = 2;
    h.updateValue(12);
    values[0U] = 5;
    h.updateValue(0);
    values[16U] = 2;
    h.updateValue(16);
    values[4U] = 3;
    h.updateValue(4);
    values[8U] = 3;
    h.updateValue(8);
    values[4U] = 4;
    h.updateValue(4);
    values[12U] = 3;
    h.updateValue(12);
    values[4U] = 5;
    h.updateValue(4);
    values[16U] = 3;
    h.updateValue(16);
    values[8U] = 4;
    h.updateValue(8);
    values[12U] = 4;
    h.updateValue(12);
    values[8U] = 5;
    h.updateValue(8);
    values[16U] = 4;
    h.updateValue(16);
    values[12U] = 5;
    h.updateValue(12);
    values[16U] = 5;
    h.updateValue(16);
    values[1U] = 2;
    h.updateValue(1);
    values[5U] = 2;
    h.updateValue(5);
    values[1U] = 3;
    h.updateValue(1);
    values[9U] = 2;
    h.updateValue(9);
    values[1U] = 4;
    h.updateValue(1);
    values[13U] = 2;
    h.updateValue(13);
    values[1U] = 5;
    h.updateValue(1);
    values[17U] = 2;
    h.updateValue(17);
    values[5U] = 3;
    h.updateValue(5);
    values[9U] = 3;
    h.updateValue(9);
    values[5U] = 4;
    h.updateValue(5);
    values[13U] = 3;
    h.updateValue(13);
    values[5U] = 5;
    h.updateValue(5);
    values[17U] = 3;
    h.updateValue(17);
    values[9U] = 4;
    h.updateValue(9);
    values[13U] = 4;
    h.updateValue(13);
    values[9U] = 5;
    h.updateValue(9);
    values[17U] = 4;
    h.updateValue(17);
    values[13U] = 5;
    h.updateValue(13);
    values[17U] = 5;
    h.updateValue(17);
    values[2U] = 2;
    h.updateValue(2);
    values[6U] = 2;
    h.updateValue(6);
    values[2U] = 3;
    h.updateValue(2);
    values[10U] = 2;
    h.updateValue(10);
    values[2U] = 4;
    h.updateValue(2);
    values[14U] = 2;
    h.updateValue(14);
    values[2U] = 5;
    h.updateValue(2);
    values[18U] = 2;
    h.updateValue(18);
    values[6U] = 3;
    h.updateValue(6);
    values[10U] = 3;
    h.updateValue(10);
    values[6U] = 4;
    h.updateValue(6);
    values[14U] = 3;
    h.updateValue(14);
    values[6U] = 5;
    h.updateValue(6);
    values[18U] = 3;
    h.updateValue(18);
    values[10U] = 4;
    h.updateValue(10);
    values[14U] = 4;
    h.updateValue(14);
    values[10U] = 5;
    h.updateValue(10);
    values[18U] = 4;
    h.updateValue(18);
    values[14U] = 5;
    h.updateValue(14);
    values[18U] = 5;
    h.updateValue(18);
    values[3U] = 2;
    h.updateValue(3);
    values[7U] = 2;
    h.updateValue(7);
    values[3U] = 3;
    h.updateValue(3);
    values[11U] = 2;
    h.updateValue(11);
    values[3U] = 4;
    h.updateValue(3);
    values[15U] = 2;
    h.updateValue(15);
    values[3U] = 5;
    h.updateValue(3);
    values[19U] = 2;
    h.updateValue(19);
    values[7U] = 3;
    h.updateValue(7);
    values[11U] = 3;
    h.updateValue(11);
    values[7U] = 4;
    h.updateValue(7);
    values[15U] = 3;
    h.updateValue(15);
    values[7U] = 5;
    h.updateValue(7);
    values[19U] = 3;
    h.updateValue(19);
    values[11U] = 4;
    h.updateValue(11);
    values[15U] = 4;
    h.updateValue(15);
    values[11U] = 5;
    h.updateValue(11);
    values[19U] = 4;
    h.updateValue(19);
    values[15U] = 5;
    h.updateValue(15);
    values[19U] = 5;
    h.updateValue(19);
    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[2U] = 5;
    h.add(2);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    
    values[3U] = 6;
    h.updateValue(3);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */
    
    
    }
    {
    saturnin::Array<double> values(20);
    for (unsigned int i = 0; i < 20; i++) {
        values.push(0);
    }
    saturnin::Heap<double, HeapTestComp> h(20, HeapTestComp(values));
    values[0U] = 0;
    h.add(0);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[1U] = 0;
    h.add(1);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[2U] = 0;
    h.add(2);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[3U] = 0;
    h.add(3);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[4U] = 0;
    h.add(4);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[5U] = 0;
    h.add(5);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[6U] = 0;
    h.add(6);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[7U] = 0;
    h.add(7);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[8U] = 0;
    h.add(8);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[9U] = 0;
    h.add(9);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[10U] = 0;
    h.add(10);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[11U] = 0;
    h.add(11);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[12U] = 0;
    h.add(12);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[13U] = 0;
    h.add(13);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[14U] = 0;
    h.add(14);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[15U] = 0;
    h.add(15);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[16U] = 0;
    h.add(16);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[17U] = 0;
    h.add(17);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[18U] = 0;
    h.add(18);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[19U] = 0;
    h.add(19);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[0U] = 1;
    h.updateValue(0);
    values[1U] = 1;
    h.updateValue(1);
    values[2U] = 1;
    h.updateValue(2);
    values[3U] = 1;
    h.updateValue(3);
    values[4U] = 1;
    h.updateValue(4);
    values[5U] = 1;
    h.updateValue(5);
    values[6U] = 1;
    h.updateValue(6);
    values[7U] = 1;
    h.updateValue(7);
    values[8U] = 1;
    h.updateValue(8);
    values[9U] = 1;
    h.updateValue(9);
    values[10U] = 1;
    h.updateValue(10);
    values[11U] = 1;
    h.updateValue(11);
    values[12U] = 1;
    h.updateValue(12);
    values[13U] = 1;
    h.updateValue(13);
    values[14U] = 1;
    h.updateValue(14);
    values[15U] = 1;
    h.updateValue(15);
    values[16U] = 1;
    h.updateValue(16);
    values[17U] = 1;
    h.updateValue(17);
    values[18U] = 1;
    h.updateValue(18);
    values[19U] = 1;
    h.updateValue(19);
    values[0U] = 2;
    h.updateValue(0);
    values[4U] = 2;
    h.updateValue(4);
    values[0U] = 3;
    h.updateValue(0);
    values[8U] = 2;
    h.updateValue(8);
    values[0U] = 4;
    h.updateValue(0);
    values[12U] = 2;
    h.updateValue(12);
    values[0U] = 5;
    h.updateValue(0);
    values[16U] = 2;
    h.updateValue(16);
    values[4U] = 3;
    h.updateValue(4);
    values[8U] = 3;
    h.updateValue(8);
    values[4U] = 4;
    h.updateValue(4);
    values[12U] = 3;
    h.updateValue(12);
    values[4U] = 5;
    h.updateValue(4);
    values[16U] = 3;
    h.updateValue(16);
    values[8U] = 4;
    h.updateValue(8);
    values[12U] = 4;
    h.updateValue(12);
    values[8U] = 5;
    h.updateValue(8);
    values[16U] = 4;
    h.updateValue(16);
    values[12U] = 5;
    h.updateValue(12);
    values[16U] = 5;
    h.updateValue(16);
    values[1U] = 2;
    h.updateValue(1);
    values[5U] = 2;
    h.updateValue(5);
    values[1U] = 3;
    h.updateValue(1);
    values[9U] = 2;
    h.updateValue(9);
    values[1U] = 4;
    h.updateValue(1);
    values[13U] = 2;
    h.updateValue(13);
    values[1U] = 5;
    h.updateValue(1);
    values[17U] = 2;
    h.updateValue(17);
    values[5U] = 3;
    h.updateValue(5);
    values[9U] = 3;
    h.updateValue(9);
    values[5U] = 4;
    h.updateValue(5);
    values[13U] = 3;
    h.updateValue(13);
    values[5U] = 5;
    h.updateValue(5);
    values[17U] = 3;
    h.updateValue(17);
    values[9U] = 4;
    h.updateValue(9);
    values[13U] = 4;
    h.updateValue(13);
    values[9U] = 5;
    h.updateValue(9);
    values[17U] = 4;
    h.updateValue(17);
    values[13U] = 5;
    h.updateValue(13);
    values[17U] = 5;
    h.updateValue(17);
    values[2U] = 2;
    h.updateValue(2);
    values[6U] = 2;
    h.updateValue(6);
    values[2U] = 3;
    h.updateValue(2);
    values[10U] = 2;
    h.updateValue(10);
    values[2U] = 4;
    h.updateValue(2);
    values[14U] = 2;
    h.updateValue(14);
    values[2U] = 5;
    h.updateValue(2);
    values[18U] = 2;
    h.updateValue(18);
    values[6U] = 3;
    h.updateValue(6);
    values[10U] = 3;
    h.updateValue(10);
    values[6U] = 4;
    h.updateValue(6);
    values[14U] = 3;
    h.updateValue(14);
    values[6U] = 5;
    h.updateValue(6);
    values[18U] = 3;
    h.updateValue(18);
    values[10U] = 4;
    h.updateValue(10);
    values[14U] = 4;
    h.updateValue(14);
    values[10U] = 5;
    h.updateValue(10);
    values[18U] = 4;
    h.updateValue(18);
    values[14U] = 5;
    h.updateValue(14);
    values[18U] = 5;
    h.updateValue(18);
    values[3U] = 2;
    h.updateValue(3);
    values[7U] = 2;
    h.updateValue(7);
    values[3U] = 3;
    h.updateValue(3);
    values[11U] = 2;
    h.updateValue(11);
    values[3U] = 4;
    h.updateValue(3);
    values[15U] = 2;
    h.updateValue(15);
    values[3U] = 5;
    h.updateValue(3);
    values[19U] = 2;
    h.updateValue(19);
    values[7U] = 3;
    h.updateValue(7);
    values[11U] = 3;
    h.updateValue(11);
    values[7U] = 4;
    h.updateValue(7);
    values[15U] = 3;
    h.updateValue(15);
    values[7U] = 5;
    h.updateValue(7);
    values[19U] = 3;
    h.updateValue(19);
    values[11U] = 4;
    h.updateValue(11);
    values[15U] = 4;
    h.updateValue(15);
    values[11U] = 5;
    h.updateValue(11);
    values[19U] = 4;
    h.updateValue(19);
    values[15U] = 5;
    h.updateValue(15);
    values[19U] = 5;
    h.updateValue(19);
    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[2U] = 5;
    h.add(2);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[12U] = 5;
    h.add(12);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[0U] = 5;
    h.add(0);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[3U] = 6;
    h.updateValue(3);
    values[9U] = 6;
    h.updateValue(9);
    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[0U] = 5;
    h.add(0);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[2U] = 6.05263;
    h.updateValue(2);
    values[9U] = 7.05263;
    h.updateValue(9);
    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[15U] = 5;
    h.add(15);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[14U] = 6.10803;
    h.updateValue(14);
    values[9U] = 8.16066;
    h.updateValue(9);
    values[12U] = 5;
    h.add(12);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[0U] = 5;
    h.add(0);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[5U] = 8.16066;
    h.add(5);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[13U] = 5;
    h.add(13);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[17U] = 5;
    h.add(17);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[16U] = 5;
    h.add(16);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[18U] = 5;
    h.add(18);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[17U] = 6.16635;
    h.updateValue(17);
    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[16U] = 5;
    h.add(16);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[18U] = 6.22774;
    h.updateValue(18);
    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[15U] = 5;
    h.add(15);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[13U] = 5;
    h.add(13);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[14U] = 7.40039;
    h.updateValue(14);
    values[0U] = 6.29236;
    h.updateValue(0);
    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[13U] = 5;
    h.add(13);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[15U] = 6.36037;
    h.updateValue(15);
    values[0U] = 7.65273;
    h.updateValue(0);
    values[10U] = 7.65273;
    h.add(10);
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    values[13U] = 6.43197;
    h.updateValue(13);
    values[0U] = 9.0847;
    h.updateValue(0);
    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    h.removeMin();
#ifdef DEBUG
    h.check();
#endif /* DEBUG */

    }
}
