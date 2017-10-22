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

#ifndef SATURNIN_CLAUSE_H
#define	SATURNIN_CLAUSE_H

#include "VariablesManager.h"
#include "WeakBool.h"
#include "Assert.h"
#include "PoolList.h"
#include "Logger.h"
#include "Saturnin.h"

#include <stdint.h>

#define SATURNIN_MAX_LBD_VALUE ((1<<16)-1)

namespace saturnin {

    /**
     * This class represent a clause. In order to be memory efficient, the
     * array containing the literals (data) is defined as a 0-length array at
     * the end of the class. This way, if we want to have a clause of length 3,
     * we may allocate the whole clause as one chunk of memory, and the end of
     * the memory may be used for the literals.
     * However, this implementation has a drawback. We can't use the 'normal'
     * way of instantiate the clause (Clause* c = new Clause(...)) as it won't
     * reserve enough space for the literals. This mean that we have to use a
     * dedicated allocator.
     */
    class SATURNIN_EXPORT Clause final {
    public:

        /**
         * Retrieve the n-th literal of the clause
         * @param pos the position of the literal we want
         * @return the requested literal
         */
        inline Lit getLit(unsigned int pos) const {
            ASSERT(pos < size);
            return data[pos];
        }

        /**
         * Set the value of a given literal
         * @param pos the position of the literal we want to change
         * @param l the new value for the literal
         */
        inline void setLit(unsigned int pos, Lit l) {
            ASSERT(pos < size);
            data[pos] = l;
        }

        /**
         * Retrieve the n-th literal of the clause
         * @param pos the position of the literal we want
         * @return the requested literal
         */
        inline Lit& operator[](unsigned int pos) {
            ASSERT(pos < size);
            return data[pos];
        }

        /**
         * Retrieve the literal block distance of the clause.
         * This measure will only be available if the clause was learnt.
         *
         * For more information, please look at <b>Gilles Audemard</b>,
         * <b>Laurent Simon</b>, <i>"Predicting Learnt Clauses Quality in Modern
         * SAT Solver"</i>. In Twenty-first International Joint Conference on
         * Artificial Intelligence (IJCAI'09), pp. 399-404, july 2009.
         * @return the lbd of the clause
         */
        inline unsigned int getLBD() const {
            return compact.lbd;
        }

        /**
         * Specify the literal block distance value of the clause
         * @param v the new value of the lbd
         */
        inline void setLBD(unsigned int v) {
            //TODO: THIS NEEDS TO BE MODIFIED AS LBD SHOULD BE LOWER THAN SIZE
            ASSERT(v <= size);
#ifdef SATURNIN_PARALLEL
            if (v >= (1 << 8) - 1) {
                v = (1 << 8) - 1;
            }
#else
            ASSERT(v <= (1 << 16) - 1)
#endif /* SATURNIN_PARALLEL */
            compact.lbd = v;
        }

        /**
         * Check if the clause was learnt or is part of the original instance.
         *
         * <b> To determine this, we use the implicit invariant through the code
         * that when a clause is learnt (=generated), we always specify its
         * literal block distance </b>
         * @return true if the clause was learnt (=generated), false otherwise
         */
        inline bool isLearnt() const {
            return compact.lbd > 0;
        }

        /**
         * Allows to obtain direct access to literal array
         * @return a pointer to the array containing the literals
         */
        inline operator Lit*() {
            return data;
        }

        /**
         * Allows to obtain direct access to literal array
         * @return a pointer to the array containing the literals
         */
        inline Lit* lits() {
            return data;
        }

        /**
         * Allows to obtain direct access to literal array
         * @return a pointer to the array containing the literals
         */
        inline operator const Lit*() const {
            return data;
        }

        /**
         * Allows to obtain direct access to literal array
         * @return a pointer to the array containing the literals
         */
        inline const Lit* lits() const {
            return data;
        }

        /**
         * Retrieve the size of this clause
         * @return the size of the clause
         */
        inline unsigned int getSize() const {
            return size;
        }

        /**
         * Remove the element at a given position
         * @param pos the position of the element we remove
         */
        inline void removeLit(unsigned int pos){
            data[pos] = data[size-1];
            size--;
        }


        
        /**
         * Set the index for this clause
         * @param idx the index to set
         */
        inline void setIndex(unsigned int idx){
            index = idx;
        }
        
        /**
         * Retrieve the index linked to this clause
         * @return the index for this clause
         */
        inline unsigned int getIndex() const{
            return index;
        }

        /**
         * Swap the literal at the given positions
         * @param posA the position of the first literal
         * @param posB the position of the second literal
         */
        inline void swapLiterals(unsigned int posA, unsigned int posB) {
            ASSERT(posA < size);
            ASSERT(posB < size);
            Lit tmp = data[posA];
            data[posA] = data[posB];
            data[posB] = tmp;
        }

        /**
         * Check that the clause contains a given literal
         * @param l the literal to check
         * @return true if the literal is present, false otherwise
         */
        inline bool contains(Lit l) const{
            for(unsigned int i=0; i<size; i++){
                if(data[i] == l) return true;
            }
            return false;
        }

        /**
         * Check if the given clause is satisfied by the given interpretation
         * @param interpretation the wbool array that will provide the value
         *        of the n-th variable at position n
         * @return true if the clause is satisfied
         */
        inline bool isSatisfied(const Array<wbool>& interpretation) const {
            bool satisfied = false;
            for (unsigned int i = 0; i < size && !satisfied; i++) {
                Lit l = data[i];
                wbool assigned = interpretation[VariablesManager::getVar(l)];
                satisfied = assigned != wUnknown &&
                        assigned == (VariablesManager::getLitSign(l) ? wTrue : wFalse);
            }
            return satisfied;
        }
        
        /**
         * Check if this clause is attached to a solver
         * @return true if the clause is attached to a solver
         */
        inline bool isAttached() const{
            return compact.attached != 0;
        }
        
        /**
         * Change the value of the attachment of this clause
         * True meaning that the clause is currently attached, false means
         * otherwise
         * @param at the new value of the attachment of this clause
         */
        inline void setAttached(bool at){
            compact.attached = at;
        }
        
        /**
         * Retrieve the value of the freeze counter
         * The freeze counter is used to represent the number of times this
         * clause has been frozen
         * @return the value of the freeze counter
         */
        unsigned int getFreezeCounter() const{
            return compact.freezeCounter;
        }
        
        /**
         * Increment the frozen counter
         */
        inline void incrementFreezeCounter(){
            compact.freezeCounter++;
        }
        
        /**
         * Reset the freeze counter to its initial value
         */
        inline void resetFreezeCounter(){
            compact.freezeCounter = 0;
        }

        /**
         * Check if two clauses are equals.
         * Two clauses are equals if they have the same size and if
         * they both contain the same set of literals.
         * The complexity of this method is O(n²)
         * @param other the other clause to compare to
         * @return true if this clause and @a other are equals
         */
        bool operator==(const Clause& other) const;

        /**
         * Destructor
         */
        ~Clause();

        //Deleted copy constructor
        Clause(const Clause&) = delete;
        //Deleted copy-assignment operator
        Clause& operator=(const Clause&) = delete;

        /** Compact structure for various values of the clause */
        struct compact_t {
            /** The literal block distance of the clause */
            unsigned lbd : 16;
            /** Allows to known if this clause have to be removed from the watches*/
            unsigned toRemove : 4;
            /** Allows to known if this clause is currently attached to the solver*/
            unsigned attached : 4;
            /** Represent the number of times a clause was frozen */
            unsigned freezeCounter : 8;
        };

    private:

        friend class PoolList;

        /**
         * Creates a new Clause. Should only be called from PoolList
         * @param literals the array containing the literals
         * @param sz the number of literals in this clause, and in @a literals
         * @param lbd the literal block distance of the clause
         */
        Clause(const Lit * const literals, unsigned int sz, unsigned int lbd = 0);
       
        /** The length of the clause */
        unsigned int size;

        /** The variable containing some data about the close in a compact way*/
        compact_t compact;
        
        
        /** An index that can be used by the user of the clause */
        unsigned int index;

        /**
         * The array that will contain the literals
         * The size has been set to 1 to have some data and be able to use
         * the pedantic option of the compilers
         */
        Lit data[1];

    };


    /** The structure used in lazy occurrence lists */
    struct watcher_t {
        /**
         * Constructor
         * @param w the clause to watch
         * @param l the blocking literal
         */
        watcher_t(Clause* w, Lit l) : watched(w), block(l) {
            ASSERT(watched != nullptr);
            ASSERT(watched->contains(block));
        }

        /**
         * Move constructor
         * @param other the source of the copy
         */
        watcher_t(watcher_t&& other) : watched(other.watched), block(other.block) {
            ASSERT(watched != nullptr);
            ASSERT(watched->contains(block));
        }

        /**
         * Copy constructor
         * @param other the source of the copy
         */
        watcher_t(const watcher_t & other) : watched(other.watched), block(other.block) {
            ASSERT(watched != nullptr);
            ASSERT(watched->contains(block));
        }

        /**
         * Copy assignment operator
         * @param other the source of the copy
         * @return the destination of the copy
         */
        watcher_t& operator=(const watcher_t & other) {
            watched = other.watched;
            block = other.block;
            ASSERT(watched != nullptr);
            ASSERT(watched->contains(block));
            return *this;
        }
        /** The clause being watched */
        Clause* watched;
        /** The blocking literal */
        Lit block;
    };
    
}


#endif	/* SATURNIN_CLAUSE_H */

