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

#ifndef SATURNIN_H
#define	SATURNIN_H

/**
 * \mainpage Saturnin 
 * \section Introduction Introduction
 * The saturnin binary/library try to propose a c++ interface to solve the
 * Boolean satisfaction problem. In order to do this, saturnin uses a conflict
 * driven clause learning scheme.
 * \section SAT Boolean satisfaction problem
 * The Boolean satisfaction problem (SAT) is defined as follows: let x be a 
 * variable whose value can either be true or false. We define the literals of
 * x as x (positive literal) or ¬x (negative literal).
 * A clause is a disjunction of literals. The problem is to try to
 * find a truth value for each of the variables appearing in a conjunction of
 * clause in order that each clause can be satisfied. If no such values can be
 * found, the solver must prove that no values can be found as the problem is
 * unsatisfiable
 * \section Resolution Resolution
 * The resolution of the clauses C1 and C2 is called resolvent and is defined as
 * follow: let {a1,a2,...,an, c1} be the set of literals appearing in C1 and
 * {b1,b2,...,bn, ¬c1} be the set of literals appearing in C2, the resolvent is
 * defined as
 * a1&nbsp;v&nbsp;a2&nbsp;v&nbsp;...&nbsp;v&nbsp;an&nbsp;v&nbsp;b1&nbsp;v&nbsp;b2&nbsp;v&nbsp;...&nbsp;v&nbsp;bn.
 * \section CDCL Conflict driven clause learning
 * The conflict driven clause learning (CDLC) scheme for solving SAT is defined
 * as follows: we try to define a truth value for every variable through a
 * iterative process. At each step, we select a truth value for a variable and
 * check whether this truth value is possible or not. In order to do this,
 * we use the propagation algorithm.
 * \subsection Propagation Propagation
 * Whenever a truth value is assigned to a variable, we must check that no
 * clause become unsatisfied. A clause is unsatisfied whenever each of its
 * literal is evaluated to false. Therefore, whenever a clause of size n has
 * n-1 literals assigned (whose truth value is defined) to false, the truth
 * value of the last unassigned variable must be such that the literal can
 * be evaluated to false. (see \ref saturnin::Solver::propagate)
 * \subsection Learning Clause learning
 * Sometime, propagations can lead to a conflict: the assignation of a variable
 * leads to an unsatisfied clause. When such event rise, it means that the
 * variable we are propagating appears in at least two clause, once as a
 * positive and once in a negative literal. From those clause, a resolvent can
 * be created. If the generated clause is empty, or if a conflict occur when
 * no choices have been made, the solver proved the unsatisfiability of the
 * input formula. (see \ref saturnin::Solver::analyze)
 * \section Usage
 * To use saturnin as a library, you should use \ref saturnin::Solver. Fill the
 * solver through \ref saturnin::Solver::addClause. Then, you can start it by
 * using \ref saturnin::Solver::solve. If needed, you can stop it asynchronously
 * using the boolean: \ref saturnin::Solver::asyncStop. Setting it to true will
 * stop the solver.
 * When a problem is solved, you can retrieve the value of the variables through
 * \ref saturnin::Solver::getVarValue.
 * If needed, instead of creating the clauses manually, you can  also use
 * \ref saturnin::CNFReader in order to read a file in DIMACS format.
 * \section compileFeatures Compile-time features
 * When saturnin is built, different features can be turned on or off. 
 * <ul>
 * <li>assertion at runtime. Those will check the internal
 * state of the solver, and print out a stack frame whenever such assertion
 * is not met. However, those checks reduce the speed of the solver. Therefore,
 * they are (by default) set only in debugging mode, and not in release mode.</li>
 * <li>database proof. In this mode, a sqlite3 database will be generated and
 * populated with the different clauses and the different resolutions that lead
 * to the creation of that clause. More information can be found at \ref saturnin::DBWrapper</li>
 * <li>Parallelism. A portfolio can be created. At the moment, it uses a simple
 * diversification scheme (based on the initial phase value) and the clause exportation
 * is done after each conflict, but clause import is done at restarts.</li>
 * \section License
 * saturnin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 * 
 * saturnin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with saturnin.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef WIN32
#pragma warning( disable : 4251 )
#pragma warning( disable : 4127 )
#ifdef SATURNIN_LIBRARY_COMPILE
#define SATURNIN_EXPORT __declspec(dllexport)
#else
#define SATURNIN_EXPORT __declspec(dllimport)
#endif /* SATURNIN_LIBRARY_COMPILE */
#else
#define SATURNIN_EXPORT
#endif /* WIN32 */

#endif	/* SATURNIN_H */

