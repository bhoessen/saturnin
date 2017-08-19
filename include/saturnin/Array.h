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

#ifndef SATURNIN_ARRAY_H
#define	SATURNIN_ARRAY_H

#include "Assert.h"
#include "Saturnin.h"
#include <stdlib.h>
//Needed for memccpy
#include <string.h>
//Needed for placement new
#include <new>
#include <initializer_list>

namespace saturnin {
    
#ifndef SATURNIN_ARRAY_UPDATE_FACTOR
#define SATURNIN_ARRAY_UPDATE_FACTOR 1.65
#endif    

#ifdef SATURNIN_COUNT_ACCESS
    extern unsigned long int _saturnin_array_access_;
#define SATURNIN_ARRAY_ACCESS() {_saturnin_array_access_++;}
#define SATURNIN_NB_ARRAY_ACCESS _saturnin_array_access_
#else
#define SATURNIN_ARRAY_ACCESS() {}
#define SATURNIN_NB_ARRAY_ACCESS (0UL)
#endif
    
//for compatibility issue, check that the noinline attribute is defined
#ifndef __attribute_noinline__
#ifdef __MINGW32__
#define __attribute_noinline__ __attribute__ ((noinline))
#else
#define __attribute_noinline__
#endif
#endif

    /**
     * This class aims to give a vector interface to an array. The capacity of
     * the array isn't needed as it can be moved if necessary.
     *
     * This implementation doesn't take care of constructors/destructors of the
     * elements. If anything special needs to be done, it must be done on your
     * side.
     */
    template<typename T>
    class Array final {
    public:
        
        /**
         * Creates a new Array of the given capacity
         * @param c the capacity of the new Array. It must be at least equal to
         * 2
         */
        Array(unsigned int c = 64) : cap(c<2?2:c), size(0), data(nullptr) {
            ASSERT(cap != 0);
            data = (T*) malloc(cap * sizeof (T));
#ifdef DEBUG
            memset(data, 0, cap * sizeof (T));
#endif /* DEBUG */
        }
        
        /**
         * Create an array containing the different elements of an initializer
         * list.
         * @param list the list containing the different elements to add
         */
        __attribute_noinline__ Array(std::initializer_list<T> list) : cap(static_cast<unsigned int>(list.size())), size(0), data(nullptr) {
            ASSERT(cap != 0);
            data = (T*) malloc(cap * sizeof (T));
#ifdef DEBUG
            memset(data, 0, cap * sizeof (T));
#endif /* DEBUG */
            for(auto i = list.begin(); i != list.end(); i++){
                push(*i);
            }
        }

        /**
         * Copy constructor
         * @param source the source of the copy
         */
        Array(const Array<T>& source) : cap(source.cap), size(source.size), data(nullptr) {
            data = (T*) malloc(cap * sizeof (T));
            ASSERT(data != nullptr);
            memcpy(data, source.data, cap * sizeof (T));
        }

        /**
         * Copy-assignment operator
         * @param source the source of the copy
         * @return the destination of the copy
         */
        Array& operator=(const Array<T>& source) {
            if (data != nullptr) {
                free(data);
            }
            cap = source.cap;
            size = source.size;
            data = (T*) malloc(cap * sizeof (T));
            ASSERT(data != nullptr);
            memcpy(data, source.data, cap * sizeof (T));
            return *this;
        }

        /**
         * Destructor
         */
        //__attribute_noinline__ added to avoid error with gcc option Winline
        __attribute_noinline__ ~Array() {
            while (size > 0) {
                size--;
                data[size].~T();
            }
            free(data);
            data = nullptr;
        }

        /**
         * Retrieve the n-th element of the Array
         * @param i the position of the requested element
         * @return the requested element
         */
        inline T get(unsigned int i) const {
            ASSERT(i < size);
            SATURNIN_ARRAY_ACCESS();
            return data[i];
        }

        /**
         * Retrieve the last element of the Array
         * @return a const reference to the last element
         */
        inline T getLast() const {
            ASSERT(size > 0);
            SATURNIN_ARRAY_ACCESS();
            return data[size - 1];
        }
        
        /**
         * Compare two array. They are considered equals if they contains the
         * same elements in the same order. To compare the elements, the '=='
         * operator is used.
         * @param other the other array to compare to
         * @return true if they are equals, false otherwise.
         */
        inline bool operator==(const Array<T>& other){
            if(size != other.size) return false;
            for(unsigned int i = 0; i<size; i++){
                if(!(data[i] == other.data[i])) return false;
            }
            return true;
        }

        /**
         * Retrieve the reference of the n-th element of the Array
         * @param i the position of the requested element
         * @return the reference to the requested element
         */
        inline T& operator[](unsigned int i) {
            ASSERT(i < size);
            SATURNIN_ARRAY_ACCESS();
            return data[i];
        }

        /**
         * Retrieve the pointer to the start of the array
         * @return the pointer to the start of the array to be able to access
         *         it directly
         */
        inline operator T*() {
            return data;
        }

        /**
         * Retrieve the pointer to the start of the array
         * @return the pointer to the start of the array to be able to access
         *         it directly
         */
        inline operator const T*() const {
            return data;
        }

        /**
         * Add a copy of the given element at the end of this Array
         * (The copy is performed through the copy assignment operator)
         * @param elmnt the element we want a copy into the Array
         */
        void push(const T& elmnt) {
            if (size >= cap) {
                pushResize(elmnt);
                return;
            }
            ASSERT(size < cap);
            T* dest = &(data[size]);
            new (dest) T(elmnt);
            size++;
        }

        /**
         * Add a new element in the Array. The default constructor will be used
         */
        void push(){
            if (size >= cap){
                resize();
            }
            ASSERT(size<cap);
            T* dest = &(data[size]);
            new (dest) T();
            size++;
        }

        /**
         * Remove the last element of this array
         * @return a copy of the element that was removed
         */
        inline void pop() {
            ASSERT(size > 0);
            size--;
            data[size].~T();
        }

        /**
         * Remove the n last elements of this array
         * @param n the number of elements to remove in this array
         */
        inline void pop(unsigned int n) {
            int nbToRemove = n > size ? size : n;
            while (nbToRemove > 0) {
                size--;
                data[size].~T();
                nbToRemove--;
            }
        }

        /**
         * Retrieve the size of this Array
         * @return the number of element present in this Array
         */
        inline unsigned int getSize() const {
            return size;
        }

        /**
         * Retrieve the capacity of this Array
         * @return the maximum number of elements this Array can contain
         *         before having to resize itself
         */
        inline unsigned int getCapacity() const{
            return cap;
        }
        
        /**
         * Retrieve the memory footprint of this Array
         * @return the memory allocated in bytes for this array
         */
        inline size_t getMemoryFootprint() const{
            return cap * sizeof(T);
        }

    private:

        /**
         * If the capacity of the array is not enough, this function will
         * copy the elements in a bigger array
         */
        __attribute_noinline__ void resize() {
            unsigned int newCap = (unsigned int) (cap * SATURNIN_ARRAY_UPDATE_FACTOR);
            ASSERT((sizeof (T) * newCap) > (sizeof (T) * cap));
            T* tmp = (T*) malloc(newCap * sizeof (T));
            ASSERT(tmp != nullptr);
#ifdef DEBUG
            memset(tmp + cap, 0, newCap - cap);
#endif /* DEBUG */
            memcpy(tmp, data, cap * sizeof (T));
            free(data);
            data = tmp;
            cap = newCap;
        }

        /**
         * Push an element to this array during a resize
         * Special care must be done as the reference might be to the first
         * element of the vector. Therefore, if we free the memory before using
         * it, we might encounter a problem
         * @param elmnt the reference to the element we wish to copy
         */
        __attribute_noinline__ void pushResize(const T& elmnt) {
            //create the new array
            unsigned int newCap = (unsigned int) (cap * SATURNIN_ARRAY_UPDATE_FACTOR);
            ASSERT((sizeof (T) * newCap) > (sizeof (T) * cap));
            T* tmp = (T*) malloc(newCap * sizeof (T));
            ASSERT(tmp != nullptr);
#ifdef DEBUG
            memset(tmp + cap, 0, newCap - cap);
#endif /* DEBUG */
            
            //copy the previous elements
            memcpy(tmp, data, cap * sizeof (T));

            //push the new element
            T* dest = &(tmp[size]);
            new (dest) T(elmnt);
            size++;

            //delete the previous data
            free(data);
            data = tmp;
            cap = newCap;
        }

        /** The capacity of the array */
        unsigned int cap;
        /** The size of the array */
        unsigned int size;
        /** The pointer to the memory location of the array */
        T* data;

    };

    /**
     * Array implementation for unsigned integers
     */
    template<>
    class SATURNIN_EXPORT Array<unsigned int> final {
    public:

        /**
         * Creates a new Array of the given capacity
         * @param c the capacity of the new Array. The capacity must be at
         * least equal to 2.
         */
        Array(unsigned int c = 64) : cap(c<2?2:c), size(0), data(nullptr) {
            ASSERT(cap != 0);
            data = new unsigned int[cap];
        }
        
        /**
         * Create an array containing the different elements of an initializer
         * list.
         * @param list the list containing the different elements to add
         */
        __attribute_noinline__ Array(std::initializer_list<unsigned int> list) : cap(static_cast<unsigned int>(list.size())), size(0), data(nullptr) {
            ASSERT(cap != 0);
            data = new unsigned int[cap];
            for(auto i = list.begin(); i != list.end(); i++){
                push(*i);
            }
        }

        /**
         * Copy constructor
         * @param source the source of the copy
         */
        Array(const Array<unsigned int>& source) : cap(source.cap), size(source.size), data(nullptr) {
            data = new unsigned int[cap];
            ASSERT(data != nullptr);
            memcpy(data, source.data, cap * sizeof (unsigned int));
        }

        /**
         * Copy-assignment operator
         * @param source the source of the copy
         * @return the destination of the copy
         */
        Array& operator=(const Array<unsigned int>& source) {
            if (data != nullptr) {
                delete[](data);
            }
            cap = source.cap;
            size = source.size;
            data = new unsigned int[cap];
            ASSERT(data != nullptr);
            memcpy(data, source.data, cap * sizeof (unsigned int));
            return *this;
        }

        /**
         * Destructor
         */
        ~Array();

        /**
         * Retrieve the n-th element of the Array
         * @param i the position of the requested element
         * @return the requested element
         */
        inline unsigned int get(unsigned int i) const {
            ASSERT(i < size);
            SATURNIN_ARRAY_ACCESS();
            return data[i];
        }
        
        /**
         * Retrieve the last element of the Array
         * @return a const reference to the last element
         */
        inline unsigned int getLast() const {
            ASSERT(size > 0);
            SATURNIN_ARRAY_ACCESS();
            return data[size - 1];
        }

        /**
         * Compare two array. They are considered equals if they contains the
         * same elements in the same order. To compare the elements, the '=='
         * operator is used.
         * @param other the other array to compare to
         * @return true if they are equals, false otherwise.
         */
        inline bool operator==(const Array<unsigned int>& other){
            if(size != other.size) return false;
            for(unsigned int i = 0; i<size; i++){
                if(data[i] != other.data[i]) return false;
            }
            return true;
        }
        
        /**
         * Retrieve the reference of the n-th element of the Array
         * @param i the position of the requested element
         * @return the reference to the requested element
         */
        inline unsigned int& operator[](unsigned int i) {
            ASSERT(i < size);
            SATURNIN_ARRAY_ACCESS();
            return data[i];
        }

        /**
         * Retrieve the pointer to the start of the array
         * @return the pointer to the start of the array to be able to access
         *         it directly
         */
        inline operator unsigned int*() {
            return data;
        }

        /**
         * Retrieve the pointer to the start of the array
         * @return the pointer to the start of the array to be able to access
         *         it directly
         */
        inline operator const unsigned int*() const {
            return data;
        }

        /**
         * Add a copy of the given element at the end of this Array
         * (The copy is performed through the copy assignment operator)
         * @param elmnt the element we want a copy into the Array
         */
        void push(unsigned int elmnt = 0);

        /**
         * Remove the last element of this array
         * @return a copy of the element that was removed
         */
        inline void pop() {
            ASSERT(size > 0);
            size--;
        }

        /**
         * Remove the n last elements of this array
         * @param n the number of elements to remove in this array
         */
        inline void pop(unsigned int n) {
            int nbToRemove = n > size ? size : n;
            size -= nbToRemove;
        }

        /**
         * Retrieve the size of this Array
         * @return the number of element present in this Array
         */
        inline unsigned int getSize() const {
            return size;
        }
       
        /**
         * Retrieve the capapcity of this Array
         * @return the maximum number of elements this Array can contain
         *         before having to resize itself
         */
        inline unsigned int getCapacity() const{
            return cap;
        }
        
        /**
         * Retrieve the memory footprint of this Array
         * @return the memory allocated in bytes for this array
         */
        inline size_t getMemoryFootprint() const{
            return cap * sizeof(unsigned int);
        }

    private:

        /**
         * If the capacity of the array is not enough, this function will
         * copy the elements in a bigger array
         */
        inline void resize();

        /** The capacity of the array */
        unsigned int cap;
        /** The size of the array */
        unsigned int size;
        /** The pointer to the memory location of the array */
        unsigned int* data;
    };

}

#endif	/* SATURNIN_ARRAY_H */

