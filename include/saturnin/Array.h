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
#include <cstdlib>
//Needed for memccpy
#include <cstring>
//Needed for placement new
#include <new>
#include <initializer_list>
#include <utility>

namespace saturnin {
    
#ifdef SATURNIN_COUNT_ACCESS
    extern uint64_t _saturnin_array_access_;
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
        explicit Array(unsigned int c = 64) : cap(c<2?2:c), size(0), data(nullptr) {
            ASSERT(cap != 0);
            auto toAllocate = cap * sizeof(T);
            data = (T*) new char[toAllocate];
#ifdef DEBUG
            memset(data, 0, toAllocate);
#endif /* DEBUG */
        }
        
        /**
         * Create an array containing the different elements of an initializer
         * list.
         * @param list the list containing the different elements to add
         */
        Array(std::initializer_list<T> list) : cap(static_cast<unsigned int>(list.size())), size(0), data(nullptr) {
            ASSERT(cap != 0);
            auto toAllocate = cap * sizeof(T);
            data = (T*) new char[toAllocate];
#ifdef DEBUG
            memset(data, 0, toAllocate);
#endif /* DEBUG */
            unsigned int i = 0;
            for (auto j = list.begin(); j != list.end(); j++) {
                new (data + i) T(std::move(*j));
                i++;
            }
            size = i;
        }

        //Deleted copy constructor. To obtain copy, use the copy() function
        Array(const Array<T>& source) = delete;

        /**
         * Move constructor
         * @param source the source of the move, will be set to an emtpy 2 element array
         */
        Array(Array<T>&& source) : cap(source.cap), size(source.size), data(source.data) {
            source.cap = 0;
            source.size = 0;
            source.data = nullptr; //(T*) new char[2 * sizeof(T)];
#ifdef DEBUG
            //memset(source.data, 0, 2 * sizeof(T));
#endif /* DEBUG */
        }

        //Deleted copy-assignement operator
        Array& operator=(const Array<T>&) = delete;

        /**
         * Move-assignment operator
         * @param source the source of the copy
         * @return the destination of the copy
         */
        Array& operator=(Array<T>&& source) {
            delete[](reinterpret_cast<char*>(data));
            cap = source.cap;
            size = source.size;
            data = source.data;
            source.cap = 0;
            source.size = 0;
            source.data = nullptr; //(T*)std::malloc(2 * sizeof(T));
            return *this;
        }

        /**
         * Destructor
         */
        ~Array() {
			unsigned int sz = size;
			while (sz > 0) {
                sz--;
                data[sz].~T();
            }
            delete[](reinterpret_cast<char*>(data));
            data = nullptr;
        }

        /**
         * Create a copy of this array.
         * The copy will have exactly the same capacity and will hold a copy of the element of this
         * array. The copy is performed using the copy constructor.
         * @return a copy of this array
         */
        Array<T> copy() const {
			Array<T> other(cap);
            for (unsigned int i = 0; i < size; i++) {
				ASSERT(data != nullptr);
                new (other.data + i) T(data[i]);
            }
            other.size = size;
            return std::move(other);
        }

        /**
         * Retrieve the n-th element of the Array
         * @param i the position of the requested element
         * @return the requested element
         */
        inline const T& get(unsigned int i) const {
            ASSERT(i < size);
			ASSERT(data != nullptr);
            SATURNIN_ARRAY_ACCESS();
            return data[i];
        }

        /**
         * Retrieve the last element of the Array
         * @return a const reference to the last element
         */
        inline const T& getLast() const {
            ASSERT(size > 0);
			ASSERT(data != nullptr);
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
				ASSERT(data != nullptr);
				ASSERT(other.data != nullptr);
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
			ASSERT(data != nullptr);
            SATURNIN_ARRAY_ACCESS();
            return data[i];
        }

        /**
        * Retrieve the reference of the n-th element of the Array
        * @param i the position of the requested element
        * @return the reference to the requested element
        */
        inline const T& operator[](unsigned int i) const {
            ASSERT(i < size);
			ASSERT(data != nullptr);
            SATURNIN_ARRAY_ACCESS();
            return data[i];
        }

        /**
         * Retrieve the pointer to the start of the array
         * @return the pointer to the start of the array to be able to access
         *         it directly
         */
        inline operator T*() {
			ASSERT(data != nullptr);
            return data;
        }

        /**
         * Retrieve the pointer to the start of the array
         * @return the pointer to the start of the array to be able to access
         *         it directly
         */
        inline operator const T*() const {
			ASSERT(data != nullptr);
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
			ASSERT(data != nullptr);
            T* dest = &(data[size]);
            new (dest) T(elmnt);
            size++;
        }

        /**
        * Add the given element at the end of this Array
        * @param elmnt the element we want a copy into the Array
        */
        void push(T&& elmnt) {
            if (size >= cap) {
                pushResize(std::move(elmnt));
                return;
            }
            ASSERT(size < cap);
			ASSERT(data != nullptr);
            T* dest = &(data[size]);
            new (dest) T(std::move(elmnt));
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
			ASSERT(data != nullptr);
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
			ASSERT(data != nullptr);
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
				ASSERT(size > 0);
				ASSERT(data != nullptr);
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

        /**
         * Retrieve a pointer to the first element
         * @return the pointer to the first element
         */
        inline T* begin() {
			ASSERT(data != nullptr);
            return data;
        }

        /**
        * Retrieve a pointer to the first element
        * @return the pointer to the first element
        */
        inline const T* begin() const {
			ASSERT(data != nullptr);
            return data;
        }

        /**
        * Retrieve a pointer to the element after the last one
        * @return the pointer to the element after the last one
        */
        inline T* end() {
			ASSERT(data != nullptr);
            return data+size;
        }

        /**
        * Retrieve a pointer to the element after the last one
        * @return the pointer to the element after the last one
        */
        inline const T* end() const {
			ASSERT(data != nullptr);
            return data + size;
        }

    private:

        /** Compute the update capacity */
        constexpr static inline unsigned int updateFactor(unsigned int c) {
            return c < 2 ? 2 : static_cast<unsigned int>(c * 1.65);
        }

        /**
         * If the capacity of the array is not enough, this function will
         * copy the elements in a bigger array
         */
        __attribute_noinline__ void resize() {
            unsigned int newCap = updateFactor(cap);
			auto toAllocate = newCap * sizeof (T);
            ASSERT(toAllocate > (sizeof (T) * cap));
			T* tmp = (T*) new char[toAllocate];
            ASSERT(tmp != nullptr);
#ifdef DEBUG
            memset(tmp + cap, 0, newCap - cap);
#endif /* DEBUG */
            memcpy(tmp, data, cap * sizeof (T));
            delete[](reinterpret_cast<char*>(data));
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
            unsigned int newCap = updateFactor(cap);
			auto toAllocate = newCap * sizeof (T);
            ASSERT(toAllocate > (sizeof (T) * cap));
			T* tmp = (T*) new char[toAllocate];
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
            delete[](reinterpret_cast<char*>(data));
            data = tmp;
            cap = newCap;
        }

		

        /**
        * Push an element to this array during a resize
        * Special care must be done as the lvalue might be to the first
        * element of the vector. Therefore, if we free the memory before using
        * it, we might encounter a problem
        * @param elmnt the lreference to the element we wish to copy
        */
        __attribute_noinline__ void pushResize(T&& elmnt) {
            //create the new array
            unsigned int newCap = updateFactor(cap);
            auto toAllocate = newCap * sizeof (T);
            ASSERT(toAllocate > (sizeof (T) * cap));
			T* tmp = (T*) new char[toAllocate];
            ASSERT(tmp != nullptr);
#ifdef DEBUG
            memset(tmp + cap, 0, newCap - cap);
#endif /* DEBUG */

            //copy the previous elements
            memcpy(tmp, data, cap * sizeof(T));

            //push the new element
            T* dest = &(tmp[size]);
            new (dest) T(std::move(elmnt));
            size++;

            //delete the previous data
            delete[](reinterpret_cast<char*>(data));
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

}

#endif	/* SATURNIN_ARRAY_H */

