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

#ifndef SATURNIN_SIMPLIFIER_H
#define	SATURNIN_SIMPLIFIER_H

#include "Solver.h"
#include "Saturnin.h"

namespace saturnin{
    
    /**
     * This class is used to simplify a SAT instance
     */
    class SATURNIN_EXPORT Simplifier final {
    public:
        
        /**
         * Create a new simplifier
         * @param nbVar the initial number of variables
         * @param nbClauses the initial number of clauses
         */
        Simplifier(unsigned int nbVar, unsigned int nbClauses);
        
        
        /**
         * Destructor
         */
        ~Simplifier();
        
        /**
         * Add a new clause to the simplifier
         * @param lits the literals composing the clause to add
         * @param sz the number of literals contained int the arrar @a lits
         * @return true if we added the clause successfully, false otherwise
         */
        bool addClause(const Lit* const lits, unsigned int sz){
            return s.addClause(lits, sz, true);
        }
        
        /**
         * Retrieve the number of variable present in this simplifier
         * @return the number of variable
         */
        unsigned int getNbVar() const {
            return s.getNbVar();
        }
        
        /**
         * Retrieve the weak boolean value of a given variable
         * @param v the variable we would like to know it's value
         * @return the wbool representing the weak boolean value
         */
        inline wbool getVarValue(Var v) const {
            ASSERT(s.map.getSize() <= v || s.map[v] == lit_Undef);
            return s.assign[v];
        }
        
        /**
         * Retrieve the number of binary clauses
         * @return how many binary clauses are present in this solver
         */
        inline unsigned int getNbBinaryClauses() const {
            return s.getNbBinaryClauses();
        }
        
        /**
         * Return the list of literal that are present in a binary clause with
         * a given literal @a l
         * @param l a literal we want every binary clause using @a l
         * @return the list of literals that appear in a binary clause with @a l
         */
        const Array<Lit>& getBinaryWith(Lit l) const {
            return s.getBinaryWith(l);
        }
        
        /**
         * Return the reference to the array containing all initial clauses
         * except the binary ones
         * @return the list of initial clauses of size 3+
         */
        const Array<Clause*>& getInitialClauses() const {
            return s.getInitialClauses();
        }
        
        /**
         * Return the list of proven literals
         * @return the array containing every proven literal
         */
        const Array<Lit>& getProvenLit() const {
            return s.stack;
        }
        
        /**
         * Apply the revival algorithm on the clauses of a minimum length.
         * For more information, feel free to consult the following publication:
         * Piette C, Hamadi Y, Saïs L, Vivifying Propositional Clausal Formulae
         * In ECAI 2008, pp 525-529
         * @param minimum the minimum length of the clauses to be taken into
         *        account for the simplification
         */
        void revival(unsigned int minimum = 15);
        
        /**
         * Retrieve the number of clauses that were reduced by the revival
         * algorithm
         * @return the number of clauses reduced
         */
        unsigned int getNbClausesReduced() const {
            return nbClausesReduced;
        }
        
    private:
        
        /** The solver used to simplify an instance */
        Solver s;
        /** The number of clauses that were reduced */
        unsigned int nbClausesReduced;
        
    };
    
}

#endif	/* SATURNIN_SIMPLIFIER_H */

