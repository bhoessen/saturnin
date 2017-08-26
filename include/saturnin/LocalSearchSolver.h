/*
Copyright (c) <2015> <B.Hoessen>

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

#ifndef SATURNIN_LOCALSEARCHSOLVER_H
#define	SATURNIN_LOCALSEARCHSOLVER_H

#include "Array.h"
#include "WeakBool.h"
#include "ClauseAllocator.h"
#include "RandomGenerator.h"
#include "Saturnin.h"


namespace saturnin {
    
    /**
     * This class represent a local search SAT solver
     */
    class SATURNIN_EXPORT LocalSearchSolver final{
    public:
        
        /** 
         * Create a new local search sat solver
         * @param nbVar the initial number of variable of the instance
         * @param nbClause the initial number of clause of the instance
         */
        LocalSearchSolver(unsigned int nbVar, unsigned int nbClause);
        
        /** Destructor */
        ~LocalSearchSolver();
        
        /**
         * Add a clause to this local search solver
         * @param clause the clause to add
         * @return true if we successfully added the clause, false otherwise
         */
        bool addClause(const Array<Lit>& clause);
        
        /**
         * Add a clause to this local search solver
         * @param literals the literals to add
         * @param sz the number of literals in the memory starting at @a literals
         * @return true if we successfully added the clause, false otherwise
         */
        bool addClause(const Lit* literals, unsigned int sz);
        
        /**
         * Try to solve the given instance
         * @param maxNbFlip the maximum number of flipped literals
         * @return true if a solution has been found
         */
        bool solve(unsigned int maxNbFlip);
        
        /**
         * Return the number of variable used in this solver
         * @return the number of variable used in this solver
         */
        unsigned int getNbVar() const {
            return nbVar;
        }
        
        /**
         * Check if the assignation provides a model for the given instance
         * @return true if the given instantiation provides a model for the
         *         SAT instance, false otherwise
         */
        bool isModel() const;
        
        /**
         * Retrieve the assigned value of a variable
         * @param v the variable we want to check its value
         * @return the value of @a v
         */
        wbool getCurrentVarValue(Var v) const {
            return assignation[v];
        }
        
        /**
         * Retrieve the proven value of a variable
         * @param v the variable we want to check its value
         * @return the proven value of @a v: wTrue if proven to be true, wFalse
         *         if proven to be false, wUnknown if not yet proven
         */
        wbool getProvenValue(Var v) const {
            return unit[v];
        }
        
        /**
         * Retrieve the taboo lenght
         * @return the length of the taboo
         */
        unsigned int getTabooLenght() const {
            return tabooLenght;
        }
        
        /**
         * Set the taboo length.
         * The taboo length is the minimum number of flip that can be performed
         * before the same variable is flipped again
         * @param taboo the taboo length
         */
        void setTabooLength(unsigned int taboo){
            tabooLenght = taboo;
        }
        
        /**
         * Retrieve the number of variable we flipped
         * @return the number of variable we flipped
         */
        unsigned int getNbFlip() const {
            return nbFlip;
        }
        
        /**
         * Retrieve the state of the solver
         * @return the weak boolean state of this solver
         */
        wbool getState() const {
            return state;
        }
        
    private:
        
        /**
         * Check that the counter of nb of true literals of this clause is
         * correct
         * @param c the clause to check
         * @return true if the counter is set to the right value
         */
        bool checkTrueLitCounter(const Clause* c) const;
        
        /**
         * Ensure that we have enough capacity in order to use the given
         * variable.
         * @param v the variable that we want to use
         * @param nbInitialized the number of variable that have already been
         *        initialized
         */
        void ensureCapacity(Var v, unsigned int nbInitialized);
        
        /**
         * Retrieve the next candidate for flipping
         * @return the variable to flip
         */
        Var getNextLitToFlip();
        
        /**
         * Flip a literal and update the corresponding data structures
         * @param l the variable to flip
         */
        void flipVariable(Var l);
        
        /**
         * Compute the score of flipping a variable
         * @param v the candidate for flipping
         * @return the computed score
         */
        long getFlippingScore(Var v);
        
        /** The state of this solver */
        wbool state;
        
        /** The number of variables used in the instance we want to solve */
        unsigned int nbVar;
        
        /** The number of clauses that are satisfied */
        unsigned int nbSatClauses;
        
        /** The number of flip already performed */
        unsigned int nbFlip;
        
        /**
         * The array specifying for each variable the number of clause that
         * would become UNSAT if the literal is flipped
         */
        Array<unsigned int> nbBecomeUnsat;
        /**
         * The array specifying for each variable the number of clause that
         * would become SAT if the literal is flipped
         */
        Array<unsigned int> nbBecomeSat;
        /**
         * The array containing the last value of nbFlip when a given variable
         * was flipped
         */
        Array<unsigned int> lastNbFlip;
        /** The taboo lenght */
        unsigned int tabooLenght;
        /** The current score of the assignation */
        unsigned int score;
        /** Current assignation vector */
        Array<wbool> assignation;
        /** The value of the variable if they were added as a unit clause */
        Array<wbool> unit;
        
        /** The occurence list */
        Array<Array<Clause*> > occList;
        
        /** The clause allocator for this solver */
        ClauseAllocator allocator;
        /** The array containing every clause */
        Array<Clause*> clauses;
        
    };
}

#endif	/* LOCALSEARCHSOLVER_H */

