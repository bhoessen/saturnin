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

#ifndef SATURNIN_VARIABLESMANAGER_H
#define	SATURNIN_VARIABLESMANAGER_H

#include <stdint.h>
#include <limits>

#include "Assert.h"
#include "WeakBool.h"
#include "Saturnin.h"

#include <limits>

namespace saturnin {

    /** Definition for the Lit type */
    typedef unsigned int Lit;

    /** The value of an undefined literal */
    static const Lit lit_Undef = std::numeric_limits<Lit>::max();

    /** Definition for the Var type */
    typedef unsigned int Var;

    /** The value of an undefined variable */
    static const Var var_Undef = std::numeric_limits<Var>::max();

    /**
     * This class contains different static methods to help the usage of
     * the literals and variables in saturnin
     */
    class SATURNIN_EXPORT VariablesManager final {
    public:

        /**
         * Retrieve the variable used in a given literal
         * @param lit the literal we would like it's variable
         * @return the variable used by the literal
         */
        static inline Var getVar(Lit lit) {
            return lit >> 1;
        }

        /**
         * Create a literal from a variable
         * @param var the var we would like a literal from
         * @param sign the sign of the requested literal
         * @return the requested literal
         */
        static inline unsigned int getLit(Var var, bool sign) {
            return (var << 1) + (1 & (!sign));
        }
        
        /**
         * Check if a given literal is satisfied by the value of a weak boolean
         * @param l the literal to check if it is satisfied
         * @param a the assignation value of @a l
         * @return true if the literal is assigned, false otherwise
         */
        static inline bool isLitSatisfied(Lit l, wbool a){
            if(a == wUnknown){
                return false;
            }else{
                return a == (getLitSign(l) ? wTrue : wFalse);
            }
        }

        /**
         * Retrieve the boolean value of a literal
         * @param l the literal we want is boolean value
         * @return the boolean value of a literal (true if x, false if ¬x)
         */
        static inline bool getLitSign(Lit l) {
            return ((l & 1) ^ 1) != 0;
        }

        /**
         * Retrieve the opposite literal of a given literal
         * @param lit the literal which we want it's opposite
         * @return the opposite literal
         */
        static inline unsigned int oppositeLit(Lit lit) {
            return lit^1;
        }

        /**
         * Retrieve the dimacs representation of a literal
         * @param l the literal we want its dimacs representation
         * @return the signed integer dimacs representation
         */
        static inline int toDimacs(Lit l) {
            return ((l / 2) + 1) * (l % 2 == 0 ? 1 : -1);
        }

        /**
         * Create a literal from its dimacs representation
         * For the dimacs representation n represent the n-th variable using
         * the positive literal, whereas -n represent the n-th variable using
         * the negative literal. 0 does not represent any literal
         * @param value the dimacs representation of the literal
         * @return the internal Lit representation of the literal
         */
        static inline Lit fromDimacs(int64_t value) {
            bool sign = value > 0 ? true : false;
            ASSERT(value <= std::numeric_limits<Var>::max());
            ASSERT(value != 0);
            Var v = static_cast<Var>((sign ? value : -value) - 1);
            return VariablesManager::getLit(v, sign);
        }
    };

}

#endif	/* SATURNIN_VARIABLESMANAGER_H */

