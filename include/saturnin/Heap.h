/*
Copyright (c) <2012> <B.Hoessen>

This file is part of saturnin.

saturnin is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

saturnin is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with saturnin.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SATURNIN_HEAP_H
#define	SATURNIN_HEAP_H

#include "Array.h"
#include "VariablesManager.h"

namespace saturnin {

    /**
     * This is a default class that can be used for the heap comparator
     */
    template<typename K, typename V = K>
    class HeapHelper final {
    public:

        HeapHelper() = default;

        /**
         * Compare two elements
         * @param a the first element to compare
         * @param b the second element to compare
         * @return a negative number if @a a lower than @a b, a positive number
         *         if @a is greater than @b or zero if @a equals @b
         */
        inline int compare(const K& a, const K& b) const {
            if (a < b) {
                return -1;
            } else if (a > b) {
                return 1;
            } else {
                return 0;
            }
        }

        /**
         * Retrieve the value associated with a given key
         * @param key the key we want its value
         * @return the value related to @a key
         */
        inline V operator[](const K& key) const {
            return key;
        }

    };
    

    /**
     * This class represent a Heap. A data structure that allows to add
     * different integer according to a key, and an easy access to the minimum
     * key present in this heap.
     * In order to work, if n integer must be present in the heap must range
     * from 0 to n-1 and be inserted in order
     */
    template<typename T, class Comp = HeapHelper<Var, T> >
    class Heap final {
    public:

        /**
         * Create a new Heap
         * @param initialCapacity the initial capacity of the data array
         * @param c the comparator to use
         */
        Heap(unsigned int initialCapacity, Comp c) :
            data(initialCapacity), positions(initialCapacity), comparator(c) {
        }

        /**
         * Copy constructor
         * @param source the source of the copy
         */
        Heap(const Heap& source) :
            data(source.data), positions(source.positions), comparator(source.comparator) {
        }
        
        /**
         * Destructor
         */
        __attribute_noinline__ ~Heap(){
        }
        
        /**
         * Copy-assignment operator
         * @param source the source of the copy
         * @return the destination of the copy
         */
        Heap& operator=(const Heap& source) {
            data = source.data;
            positions = source.positions;
            return *this;
        }

        /**
         * Check if a given variable is already present in this heap
         * @param v the variable we would like to know if present or not
         * @return true if @a v is present in the heap, false otherwise
         */
        bool contains(const Var v) const {
            return positions[v] < data.getSize() && data.get(positions[v]) == v;
        }

        /**
         * Add the given variable to the heap
         * @param v the variable to add to the heap
         */
        void add(const Var v) {
            if (v >= positions.getSize()) {
                while (v >= positions.getSize()) {
                    positions.push((unsigned int) - 1);
                }
            }
            ASSERT(!contains(v));
            data.push(v);
            positions[v] = data.getSize() - 1;
            percolateUp(v);
            ASSERT_EQUAL(v, data.get(positions[v]));
        }

        /**
         * Retrieve the var with the minimum value
         * @return the variable having the minimal value in the heap
         */
        Var getMin() const {
            ASSERT(data.getSize()>(unsigned int) 0);
            ASSERT_EQUAL((unsigned int) 0, positions[data.get(0)]);
            return data.get((unsigned int) 0);
        }

        /**
         * Remove the minimum value from the heap
         * @return the var having the minimum value in the heap
         */
        Var removeMin() {
            ASSERT(data.getSize()>(unsigned int) 0);
            Var v = data.get((unsigned int) 0);
            ASSERT_EQUAL((unsigned int) 0, positions[v]);
            unsigned int size = data.getSize();
            if (size > 1) {
                Var other = data.get(size - 1);
                ASSERT_EQUAL(size - 1, positions[other]);
                swap(positions[v], positions[other]);
                swap(data[0U], data[size - 1]);
                ASSERT_EQUAL((unsigned int) 0, positions[other]);
                data.pop();
                percolateDown(other);
            } else {
                data.pop();
            }
            return v;
        }

        /**
         * Retrieve the value associated with a given variable
         * @param v the variable we want its value
         * @return the value related to the variable
         */
        T operator[](Var v) const {
            ASSERT_EQUAL(v, data.get(positions.get(v)));
            return comparator[v];
        }

        /**
         * Update the value of a given variable
         * @param v the variable we want to update its value
         */
        void updateValue(Var v) {
            unsigned int pos = positions[v];
            ASSERT(pos < data.getSize());
            ASSERT_EQUAL(v, data[pos]);
            if (pos != 0 && comparator.compare(v, data.get(parent(pos))) < 0) {
                percolateUp(v);
            } else if (pos == 0 || comparator.compare(v, data.get(parent(pos))) > 0) {
                percolateDown(v);
            }
        }

        /**
         * Return the number of elements present in this heap
         * @return the number of element in the heap
         */
        unsigned int getSize() const {
            return data.getSize();
        }

#ifdef DEBUG

        /**
         * Check that the heap is well formed
         * @param pos the starting position to check the heap
         * @return true if the heap is well formed, false otherwise
         */
        bool check(unsigned int pos = 0)const {
            ASSERT(pos < data.getSize());
            ASSERT_EQUAL(pos, positions.get(data.get(pos)));
            if (left(pos) < data.getSize()) {
                ASSERT(comparator.compare(data.get(pos), data.get(left(pos))) <= 0);
                ASSERT(check(left(pos)));
            }
            if (right(pos) < data.getSize()) {
                ASSERT(comparator.compare(data.get(pos), data.get(right(pos))) <= 0);
                ASSERT(check(right(pos)));
            }
            return true;
        }
#endif

        /**
         * Retrieve the memory footprint of this heap
         * @return the memory footprint of this heap
         */
        size_t getMemoryFootprint() const{
            return data.getMemoryFootprint() + positions.getMemoryFootprint();
        }

    private:

        /**
         * Percolate the position of a variable as the new value of the variable
         * is lower than its previous value
         * @param v the variable to percolate
         */
        void percolateUp(Var v) {
            unsigned int pos = positions[v];
            ASSERT(pos < data.getSize());
            ASSERT_EQUAL(v, data[pos]);
            bool stop = false;
            while (!stop && pos > 0) {
                unsigned int par = parent(pos);
                if (comparator.compare(data[pos], data[par]) < 0) {
                    swap(data[par], data[pos]);
                    //update the position of the parent (which is now at pos)
                    positions[data[pos]] = pos;
                    pos = par;
                } else {
                    stop = true;
                }
            }
            positions[v] = pos;
            data[pos] = v;
        }

        /**
         * Percolate the position of a variable as the new value of the variable
         * is greater than its previous value
         * @param v the variable to percolate
         */
        void percolateDown(Var v) {
            unsigned int pos = positions[v];
            ASSERT(pos < data.getSize());
            ASSERT_EQUAL(v, data[pos]);
            unsigned int size = data.getSize();
            bool stop = false;
            while (!stop && left(pos) < size) {
                unsigned int son = right(pos) < size &&
                        comparator.compare(data[right(pos)],
                            data[left(pos)]) < 0 ? right(pos) : left(pos);
                if (comparator.compare(data[pos], data[son]) > 0) {
                    swap(data[pos], data[son]);
                    //update the position of the son (which is now at pos)
                    positions[data[pos]] = pos;
                    pos = son;
                } else {
                    stop = true;
                }
            }
            data[pos] = v;
            positions[v] = pos;
        }

        /**
         * Swap two elements using the copy operator
         * @param a the first element to swap
         * @param b the second element to swap
         */
        template<typename D>
        void swap(D& a, D& b) {
            D c(a);
            a = b;
            b = c;
        }

        /**
         * Retrieve the position of the left son of an element at a given
         * position
         * @param pos the position of the element we want its left son
         * @return the position of the left son of the element at position @a pos
         */
        inline unsigned int left(unsigned int pos) const {
            return (pos << 1) + 1;
        }

        /**
         * Retrieve the position of the right son of an element at a given
         * position
         * @param pos the position of the element we want its right son
         * @return the position of the right son of the element at position @a pos
         */
        inline unsigned int right(unsigned int pos) const {
            return (pos << 1) + 2;
        }

        /**
         * Retrieve the position of the parent of an element at a given position
         * @param pos the position of the element we want its parent
         * @return the position of the parent of the element at position @a pos
         */
        inline unsigned int parent(unsigned int pos) const {
            return (pos - 1) >> 1;
        }


        /** The structure containing the key and the value */
        Array<Var> data;

        /**
         * The map containing the position of the different integer in the data
         * array. This allows for easy modification of their value
         */
        Array<unsigned int> positions;

        /** The comparator used */
        Comp comparator;

    };

}

#endif	/* SATURNIN_HEAP_H */

