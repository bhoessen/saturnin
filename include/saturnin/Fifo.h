/*
Copyright (c) <2013> <B.Hoessen>

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

#ifndef SATURNIN_FIFO_H
#define	SATURNIN_FIFO_H

#include <stdlib.h>
//Needed for memccpy
#include <string.h>
//Needed for placement new
#include <new>
#include "Assert.h"
#include "Saturnin.h"

#ifndef SATURNIN_FIFO_UPDATE_FACTOR
#define SATURNIN_FIFO_UPDATE_FACTOR 1.65
#endif

namespace saturnin {

    /**
     * This class represent a fifo: a data structure used to store element and
     * retrieve them with the same order they were given
     */
    template<typename T>
    class Fifo final {
    public:

        /**
         * Creates a new Fifo
         * @param initialSize the initial capacity of the fifo
         */
        Fifo(unsigned int initialSize = 32) : data(nullptr), capacity(initialSize),
        size(0), first(0), end(0) {
            data = (T*) malloc(capacity * sizeof (T));
        }

        /**
         * Copy constructor
         * @param other the source of the copy
         */
        Fifo(const Fifo& other) : data(nullptr), capacity(other.capacity),
        size(other.size), first(other.first), end(other.end) {
            data = (T*) malloc(capacity * sizeof (T));
            for (unsigned int i = 0; i < size; i++) {
                unsigned int idx = (first + i) % capacity;
                T* dest = &(data[idx]);
                new (dest) T(other.data[idx]);
            }
        }

        /**
         * Copy assignment operator
         * @param other the source of the copy
         * @return the destination of the copy
         */
        Fifo& operator=(const Fifo& other) {
            capacity = other.capacity;
            size = other.size;
            first = other.first;
            end = other.end;
            data = (T*) malloc(capacity * sizeof (T));
            for (unsigned int i = 0; i < size; i++) {
                unsigned int idx = (first + i) % capacity;
                T* dest = &(data[idx]);
                new (dest) T(other.data[idx]);
            }
            return *this;
        }

        /**
         * Destructor
         */
        ~Fifo() {
            free(data);
        }

        /**
         * Add an element to the fifo
         * @param elem
         */
        void push(const T& elem) {
            if (size == capacity) {
                resize();
            }
            ASSERT(size < capacity);
            //            data[size].T(elmnt);
            T* dest = &(data[end]);
            new (dest) T(elem);
            size++;
            end = (end + 1) % capacity;
        }

        /**
         * Retrieve the first element of the fifo
         * @return the first element of the fifo
         */
        T& getFirst() {
            ASSERT(size > 0);
            return data[first];
        }

        /**
         * Retrieve the first element of the fifo
         * @return the first element of the fifo
         */
        const T& getFirst() const {
            ASSERT(size > 0);
            return data[first];
        }

        /**
         * Remove the first element from the fifo
         */
        void pop() {
            ASSERT(size > 0);
            data[first].~T();
            first = (first + 1) % capacity;
            size--;
        }

        /**
         * Retrieve the number of elements in this fifo
         * @return the size of this fifo
         */
        unsigned int getSize() const {
            return size;
        }

        /**
         * Retrieve the current capacity of this fifo
         * @return the current capacity of the fifo
         */
        unsigned int getCapacity() const {
            return capacity;
        }

        /**
         * Retrieve the memory footprint of this fifo
         * @return the size allocated by this object
         */
        size_t getMemoryFootprint() const {
            return capacity * sizeof (T);
        }

    private:

        /** Increase the size of the data array */
        void resize() {
            unsigned int newCap = (unsigned int) (capacity * SATURNIN_FIFO_UPDATE_FACTOR);
            ASSERT((sizeof (T) * newCap) > (sizeof (T) * capacity));
            T* tmp = (T*) malloc(newCap * sizeof (T));
            ASSERT(tmp != nullptr);
#ifdef DEBUG
            memset(tmp, 0, newCap * sizeof (T));
#endif /* DEBUG */
            ASSERT_EQUAL(first, end);
            memcpy(tmp, data + first, (capacity - first) * sizeof (T));
            if (first > 0) {
                memcpy(tmp + (capacity - first), data, end * sizeof (T));
            }
#ifdef DEBUG
            unsigned int k = first;
            for (unsigned int i = 0; i < size; i++) {
                ASSERT_EQUAL(data[k], tmp[i]);
                k = (k + 1) % capacity;
            }
#endif /* DEBUG */
            free(data);
            data = tmp;
            first = 0;
            end = size;
            capacity = newCap;
        }

        /** The actual array containing the data */
        T* data;
        /** The capacity of the data array */
        unsigned int capacity;
        /** The number of elements present in this fifo */
        unsigned int size;
        /** The index of the first element in the data */
        unsigned int first;
        /** The index of the last unused element */
        unsigned int end;
    };

}

#endif	/* SATURNIN_FIFO_H */

