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

#ifndef SATURNIN_SOLVER_H
#define	SATURNIN_SOLVER_H

#include <stddef.h>                     // for size_t
#include <stdio.h>                      // for fprintf, FILE, stdout
#include <limits>                       // for numeric_limits
#include "saturnin/Array.h"             // for Array
#include "saturnin/Clause.h"            // for Clause, watcher_t
#include "saturnin/ClauseAllocator.h"   // for ClauseAllocator
#include "saturnin/Fifo.h"              // for Fifo
#include "saturnin/Heap.h"              // for Heap
#include "saturnin/Mean.h"              // for Mean
#include "saturnin/SlidingMean.h"       // for SlidingMean
#include "saturnin/VariablesManager.h"  // for Lit, Var, VariablesManager
#include "saturnin/WeakBool.h"          // for wbool, wFalse, wTrue, etc
#include "saturnin/Assert.h"            // for ASSERT
#include "saturnin/Saturnin.h"          // for SATURNIN_EXPORT
#ifdef SATURNIN_DB
#include "saturnin/DBWrapper.h"         // for DBWrapper
#endif /* SATURNIN_DB */

namespace saturnin {

#ifndef VARIABLE_TRUE_COLOR
#define VARIABLE_TRUE_COLOR "\x1B[0;32m"
#endif
#ifndef VARIABLE_FALSE_COLOR
#define VARIABLE_FALSE_COLOR "\x1B[0;31m"
#endif
#ifndef VARIABLE_NO_COLOR
#define VARIABLE_NO_COLOR "\x1B[0;30m"
#endif

#ifdef SATURNIN_PARALLEL
    /** Forward declaration of the ParallelSolver class */
    class ParallelSolver;
#endif /* SATURNIN_PARALLEL */
    
    /**
     * This class represent a solver by itself to solve a given instance. For
     * each new instance, a new solver needs to be instantiated
     */
    class SATURNIN_EXPORT Solver final {
    public:

        /**
         * Create a new Solver
         * @param nbVar the number of variable in the instance we would like
         *              to solve
         * @param nbClauses the number of clauses present in the instance we want
         *                  to solve
         * @param dbFileName the name of the database file that will be used to
         *                   gather the information during the computation. If
         *                   set to nullptr, no database operation will be used
         */
        Solver(unsigned int nbVar, unsigned int nbClauses, const char* dbFileName = nullptr);

        /**
         * Destructor
         */
        ~Solver();

        /**
         * Retrieve the weak boolean value of a given variable
         * @param v the variable we would like to know it's value
         * @return the wbool representing the weak boolean value
         */
        inline wbool getVarValue(Var v) const {
            if(map[v]==std::numeric_limits<unsigned int>::max()){
                //the variable v hasn't been used in the problem
                //therefore, we may give it any value we want
                return wUnknown;
            }
            return assign[map[v]];
        }

        
        /**
         * Retrieve the number of variable used by this Solver
         * @return the number of variable used by this Solver
         */
        inline unsigned int getNbVar() const {
            return nbVar;
        }
        
        /**
         * Retrieve the number of binary clauses
         * @return how many binary clauses are present in this solver
         */
        inline unsigned int getNbBinaryClauses() const {
            return nbBin;
        }
        
        /**
         * Return the reference to the array containing all initial clauses
         * except the binary ones
         * @return the list of initial clauses of size 3+
         */
        const Array<Clause*>& getInitialClauses() const {
            return clauses;
        }
        
        /**
         * Return the list of literal that are present in a binary clause with
         * a given literal @a l
         * @param l a literal we want every binary clause using @a l
         * @return the list of literals that appear in a binary clause with @a l
         */
        const Array<Lit>& getBinaryWith(Lit l) const {
            return binWatched.get(l);
        }

        /**
         * Retrieve the initial number of clauses that was described by the
         * instance
         * @return the number of clauses defined in the instance
         */
        inline unsigned int getNbClauses() const {
            return clauses.getSize() + nbBin;
        }

        /**
         * Retrieve the number of learnt clauses currently used by the solver
         * @return the current number of learnt clause the solver could
         */
        inline unsigned int getNbLearntClauses() const {
            return learntClauses.getSize();
        }

        /**
         * Retrieve the total number of restart performed until now
         * @return the total number of restart performed by the solver
         */
        inline uint64_t getNbRestarts() const {
            return restarts;
        }

        /**
         * Retrieve the total number of propagation performed until now
         * @return the total number of propagation performed by the solver
         */
        inline uint64_t getNbPropagation() const {
            return nbPropag;
        }

        /**
         * Retrieve the total number of conflict occured until now
         * @return the total number of conflict occured by the solver
         */
        inline uint64_t getNbConflict() const {
            return conflicts;
        }

        inline uint64_t getAssignationLevel() const {
            return assignLevel;
        }

        /**
         * Retrieve the state of the solver
         * @return wTrue if a solution has been found, wFalse if UNSAT has been
         *         proven, wUnknown otherwise
         */
        inline wbool getState() const {
            return state;
        }

        /**
         * Add a new Clause to the solver
         * @param literals the literals that will form the clause
         * @param sz the size of the array @a literals
         * @param mapped boolean telling whether the literals in @a literals
         *               have already been mapped
         * @return true if we were able to add the clause, false if we reached
         *              an inconsistent state
         */
        bool addClause(const Lit * const literals, unsigned int sz, bool mapped = false);

        /**
         * Add a presumption
         * @param v the variable we presume its value
         * @param value the truth value that is supposed to be assigned
         * @param mapped if true, we use the internal mapping
         */
        void addPresumption(Var v, bool value, bool mapped = false){
            if(mapped
#ifdef SATURNIN_PARALLEL
                    && parallelData != nullptr
#endif /* SATURNIN_PARALLEL */
                    ){
                v = map[v];
            }
            presumption[v] = value ? wTrue : wFalse;
        }

        /**
         * Add a new Clause to the solver
         * @param c the Array containing the literals to add
         * @return true if we were able to add the clause, false if we reached
         *              an inconsistent state
         */
        bool addClause(const Array<Lit>& c);

        /**
         * Try to solve the instance
         * @param nbRestarts the maximum number of restart allowed
         * @return true if a solution has been found
         */
        bool solve(unsigned int nbRestarts);

        /**
         * Try to simplify the initial instance
         * @return false if UNSAT has been proven
         */
        wbool simplify();

        /**
         * Try to find variables that appear on only 1 phase
         * @return wFalse if UNSAT has been proven
         */
        wbool phasedElimination();

        /**
         * Print a variable on a given output stream
         * @param v the variable to print
         * @param colored if true, the variable will be printed in green if the
         *        variable has been assigned to true, in red otherwise. If colored
         *        is false, no color will be used
         * @param out the output stream in which we will print the variable
         */
        inline void printVar(Var v, bool colored = true, FILE* out = stdout) const {
            colored = colored && assign.getSize() > v;
            if (colored) {
                if (assign[v] == wTrue) {
                    fprintf(out, "%s", VARIABLE_TRUE_COLOR);
                } else if (assign[v] == wFalse) {
                    fprintf(out, "%s%s", VARIABLE_FALSE_COLOR, "-");
                } else {
                    fprintf(out, "%s%s", VARIABLE_NO_COLOR, "?");
                }
            } else {
                if (assign[v] == wFalse) {
                    fprintf(out, "-");
                } else if (assign[v] == wUnknown) {
                    fprintf(out, "?");
                }
            }
            fprintf(out, "%d", v);
            if (colored) {
                fprintf(out, "%s", VARIABLE_NO_COLOR);
            }
        }

        /**
         * Print a literal on a given output stream
         * @param l the literal to print
         * @param colored if true, the literal will be printed in green if the
         *        literal has been assigned to true, in red otherwise. If colored
         *        is false, no color will be used
         * @param out the output stream in which we will print the variable
         */
        inline void printLit(Lit l, bool colored = true, FILE* out = stdout) const {
            Var v = VariablesManager::getVar(l);
            colored = colored && assign.getSize() > v;
            if (colored) {
                if (VariablesManager::getLitSign(l) ?
                    assign[v] == wTrue : assign[v] == wFalse) {
                    fprintf(out, "%s", VARIABLE_TRUE_COLOR);
                } else if (VariablesManager::getLitSign(l) ?
                    assign[v] == wFalse : assign[v] == wTrue) {
                    fprintf(out, "%s", VARIABLE_FALSE_COLOR);
                } else {
                    fprintf(out, "%s", VARIABLE_NO_COLOR);
                }
            }
            if (!VariablesManager::getLitSign(l)) {
                fprintf(out, "-");
            }
            fprintf(out, "%d", v +1);
            if (colored) {
                fprintf(out, "%s", VARIABLE_NO_COLOR);
            }
        }

        /**
         * Print the given clause according to the state of this solver
         * @param c the clause to print
         * @param color if true, a colored representation will be printed
         * @param out the output file in which we will print the clause @a c
         */
        void printClause(Clause* c, bool color = true, FILE* out = stdout) const;

        /**
         * Print the given clause according to the state of this solver
         * @param c the clause to print as an array of literals, ending with a sentinel value
         * @param color if true, a colored representation will be printed
         * @param out the output file in which we will print the clause @a c
         */
        void printClause(Lit* c, bool color = true, FILE* out = stdout) const;

        /**
         * Print the clause database that were given as the instance to solve
         * @param color if set to true, colors will be used to print the clauses
         * @param sep the separator to use between the different clauses
         * @param out the output file on which we will print the clauses
         */
        void printClauses(bool color = true, const char* const sep = "\n", FILE* out = stdout) const;

        /**
         * Check that the answer found is correct.
         * If UNSAT was found, we can't validate and assume it is correct
         * @return true if the answer is correct, false otherwise
         */
        bool validate();
        
        /**
         * Set the verbosity of the solver. The higher the value, the more will
         * be printed on the standard logger
         * @param verb the verbosity value
         */
        inline void setVerbosity(unsigned int verb){
            verbosity = verb;
        }
        
        /**
         * Retrieve the verbosity of the solver. The higher the value, the more
         * will be printed on the standard logger
         * @return the verbosity of the solver
         */
        inline unsigned int getVerbosity() const{
            return verbosity;
        }
        
        /**
         * Retrieve the number of times we reduced the size of the database
         * containing the learnt clauses
         * @return the number of reduce performed
         */
        inline unsigned int getNbReduce() const{
            return nbReducePerformed;
        }
        
        /**
         * Retrieve the number of clauses that were removed
         * @return the number of clauses we judged useless during the search
         */
        inline uint64_t getNbClausesRemoved() const{
            return nbClauseRemoved;
        }

        /**
         * Retrieve the allocator used by this solver
         * @return the allocator used by this solver
         */
        const ClauseAllocator& getAllocator() const{
            return allocator;
        }

        /**
         * Retrieve the lbd distribution
         */
        const Array<unsigned int> &getLBDDistribution() const {
            return lbdValues;
        }

        /**
         * Retrieve the number of literal removed from the initial clauses
         * @return the number of literals that were removed from the initial
         *         clauses due to the fact that they were proven false
         */
        inline unsigned int getNbLitRemoved() const{
            return nbLitRemoved;
        }

        /**
         * Retrieve the number of clauses from the initial problem that were
         * removed
         * @return the number of initial clauses that were removed
         */
        inline uint64_t getNbInitialClausesRemoved() const{
            return nbInitialClausesRemoved;
        }

        /**
         * Retrieve the number of clauses from the initial problem that were
         * reduced
         * @return the number of initial clauses that were reduced
         */
        inline uint64_t getNbInitialClausesReduced() const{
            return nbInitialClausesReduced;
        }

        /**
         * Retrieve the number of pure literals found
         * @return the number of pure literals found
         */
        inline uint64_t getNbPureLiteral() const{
            return nbPureLitFound;
        }
        
#ifdef SATURNIN_PARALLEL        
        /**
         * Set the parallel solver using this solver
         * @param master the parallel solver that created this solver
         * @param id the id of the thread that will hold this Solver
         */
        inline void setParallelSolver(ParallelSolver* master, unsigned int id) {
            parallelData = master;
            threadId = id;
        }
#endif /* SATURNIN_PARALLEL */
        
        /**
         * Retrieve the memory footprint of this solver.
         * It will compute the memory footprint of every object used by this
         * solver
         * @return the memory footprint in bytes
         */
        size_t getMemoryFootprint() const;
        
        /**
         * Asynchronous command to stop the solver
         */
        inline void stop() {
            asyncStop = true;
        }
        
        /** The variable to stop the solver */
        bool asyncStop;

#ifdef PROFILE

        /** number of cycles spent in propagateMonoWatched*/
        uint64_t __profile_propagateMono;
        /** number of cycles spent in binaryPropagate*/
        uint64_t __profile_propagateBin;
        /** number of cycles spent in propagateWatchedBy*/
        uint64_t __profile_propagateWatched;
        /** number of cycles spent in reduce*/
        uint64_t __profile_reduce;
        /** number of cycles spent in analyze*/
        uint64_t __profile_analyze;
        uint64_t __profile_simplify;

#endif /* PROFILE */


    private:
        
        /**
         * Retrieve the weak boolean value of a given literal
         * @param l the literal we would like to know it's value
         * @return the wbool representing the weak boolean value
         */
        inline wbool getLitValue(Lit l) const {
            ASSERT(assign[VariablesManager::getVar(l)] != wUnknown);
            wbool w = assign[VariablesManager::getVar(l)];
            if(VariablesManager::getLitSign(l) == false){
                w = w == wTrue ? wFalse : wTrue;
            }
            return w;
        }

        /**
         * Treat a conflicting clause
         * @param c the conflicting clause. The literal array should end with
         *          a lit_Undef
         * @return the learnt clause if it's size is greater than 2
         */
        Clause* treatConflict(Lit* c);
        
        /**
         * Ensure that we have enough capacity in order to use the given
         * variable.
         * @param v the variable that we want to use
         * @param nbInitialized the number of variable that have already been
         *        initialized
         */
        void ensureCapacity(Var v, unsigned int nbInitialized);

        /**
         * Search for a solution
         * @param maxConflict the maximum number of conflict allowed
         * @return a weak boolean representing the state of the search when
         *         finished: wTrue: the problem is SAT, wFalse: the problem is
         *         UNSAT, wUnknown: we couldn't determine whether the problem is
         *         SAT or UNSAT
         */
        wbool search(int maxConflict);
        
        /**
         * Evaluate a clause to know whether we have to keep it active
         * @param c the clause to evaluate
         * @param medianLBD the median LBD of the solver
         * @param maxToBeUsefull
         * @param trueLits the number of potential true literals in the clause
         * @return true if @a c contains literals proven true
         */
        bool reduceClauseEval(const Clause* c, unsigned int medianLBD, int maxToBeUsefull, int& trueLits);

        /**
         * Perform a reduction of the set of learnt clauses
         */
        void reduce();

        /**
         * Propagate every literal that is in the waiting queue
         * @return the pointer to the clause leading to a conflict
         */
        Lit* propagate();

        /**
         * Propagate binary clauses
         * @param l the literal to propagate
         * @return the pointer to the clause leading to a conflict, if any,
         *         nullptr otherwise
         */
        Lit* binaryPropagate(const Lit l);

        /**
         * Propagate clauses of size > 2 that are using the 2 literal watched
         * scheme
         * @param l one of the two watchers, the literal we are propagating on
         * @return the pointer to the clause leading to a conflict, if any,
         *         nullptr otherwise
         */
        Lit* propagateWatchedBy(const Lit l);

        /**
         * Propagate clauses of size > 2 that are using only 1 watched literal
         * Those clauses are the 'frozen' ones
         * @param l the literal to propagate
         * @return the pointer to the clause leading to a conflict, if any,
         *         nullptr otherwise
         */
        Lit* propagateMonoWatched(const Lit l);

        /**
         * Analyze the reason of a conflict and fill the array with a clause
         * representing the conflict
         * @param conflict the clause that couldn't have been satisfied
         * @param res the array that will contain the clause representing the
         *            conflict. Its first literal will be the branching literal
         * @return the level at which the backtrack needs to be performed
         */
        unsigned int analyze(Lit* conflict, Array<Lit>& res);
        
        /**
         * Compute the learnt clause by analyzing the reason of a conflict
         * @param conflict the clause that couldn't have been satisfied
         * @param res the array that will contain the clause representing the
         *            conflict. Its first literal will be the branching literal
         */
        void computeLearntClause(Lit* conflict, Array<Lit>& res);

        /**
         * Compute an abstract level for a given variable according its level
         * The abstract level of a variable is computed as follows: let L be the
         * level of variable @a v. L modulo 31 = N. The abstract level of @a v
         * will be an integer where the n-th bit is set to true
         * @param v the variable
         * @return the abstract level of this variable
         */
        inline unsigned int abstractLevel(Var v) const {
            return 1 << (varLevel.get(v) & 31);
        }

        /**
         * TODO
         * @param p
         * @param abstract_levels
         * @param analyze_toclear
         * @return
         */
        bool litRedundant(Lit p, unsigned int abstract_levels, Array<Lit>& analyze_toclear);

        /**
         * Enqueue the literal l for propagation
         * @param l the literal that was proven/decided to be true at the
         *          current decision level
         * @param reason the clause that led to enqueue the literal l. If
         *               nullptr, it means that either l was proven or l is
         *               is based on a decision variable
         */
        void enqueue(Lit l, Lit* reason = nullptr);

        /**
         * Backtrack a given number of levels. The reason why a backtrack would
         * fail is that we try to revert something that was proven and therefore
         * the problem must be UNSAT
         * @param nbLvl the number of levels to backtrack
         * @return true if we could backtrack, false if we couldn't. 
         */
        bool backtrack(unsigned int nbLvl);

        /**
         * Add the given clause to the set of learnt clauses.
         * If the learnt clause is one literal, it will be added to the
         * propagation.
         * @param clause the Array containing the literals that represent the
         *        literals of the clause to add
         * @return the pointer to the clause representation of  @a clause
         */
        Clause* addLearntClause(const Array<Lit>& clause);
        
        /**
         * Remove a learnt clause from the learnt database
         * @param c the clause that we will remove
         */
        void removeLearntClause(Clause* c);
        
        /**
         * Remove a given clause from the watched list for the given literal
         * @param l one of the watched literals of clause @a c
         * @param c the clause that we want to remove from the watched list
         */
        void stopWatchClause(const Lit l, const Clause* c);
        
        /**
         * Add the given clause to the watched list
         * @param c the clause we will add to the list of watched clause
         */
        void addWatchedClause(Clause* c);

        /**
         * Update the the variable state independent decaying sum (VSIDS) of a
         * given variables
         * @param v the variable to update its vsids value
         */
        void updateVSIDS(Var v);

        /**
         * Check the validity of a set of clause according to an given state
         * @param set the array containing the set of clauses that we want to 
         *            check the satisfiability
         * @param interpretation the interpretation for the variables
         * @return true if the set of clause is satisfied, false otherwise
         */
        bool checkClauseSetSatifiability(const Array<Clause*>& set,
                const Array<wbool>& interpretation) const;

        /**
         * Compute the literal block distance of a clause
         * @param c the clause to compute its literal block distance
         * @return the value of the lbd of the clause @a c
         */
        unsigned int computeLBD(Clause* c);
        
#ifdef SATURNIN_PARALLEL
        /**
         * Export a given clause to the other solvers
         * @param c the clause to export
         * @param lbd the lbd of the clause to export
         */
        void exportClause(Array<Lit>& c, unsigned int lbd);
        
        /**
         * Import a given clause into this solver
         * @param c the clause to import
         * @return true if we imported the clause correctly or false if we
         *         reached an inconsistent state
         */
        bool importClause(Clause* c);
        
        /** 
         * Import every clauses that were exported for this solver
         * @return true if we imported the clauses correctly or false if we
         *         reached an inconsistent state
         */
        bool importClauses();
        
        /** 
         * Import every literal that were exported for this solver
         * @return true if we imported the literals correctly or false if we
         *         reached an inconsistent state
         */
        bool importProvenLiterals();

#endif /* SATURNIN_PARALLEL */
        
        /**
         * This class is used for the heap containing the variables and ordered
         * according to their activity value.
         */
        class VSIDSComp {
        public:

            /**
             * Create a new VSIDSComp
             * @param s the solver that will provide the value of the activity
             *          of the variables
             */
            VSIDSComp(Solver& s) : solver(s) {
            }

            /**
             * Compare two elements
             * @param a the first element to compare
             * @param b the second element to compare
             * @return a negative number if @a a lower than @a b, a positive number
             *         if @a is greater than @b or zero if @a equals @b
             */
            inline double compare(const Var a, const Var b) const {
                return solver.varActivityValue.get(b) - solver.varActivityValue.get(a);
            }

            /**
             * Retrieve the value associated with a given key
             * @param key the key we want its value
             * @return the value related to @a key
             */
            inline double operator[](Var key) const {
                return solver.varActivityValue.get(key);
            }

        private:
            /** The solver using this VSIDSComp */
            const Solver& solver;
        };

        /** The number of variable in the instance we are trying to solve */
        unsigned int nbVar;
        /** The level at which each variable has been assigned */
        Array<unsigned int> varLevel;
        /** The array containing the assignation of the variables */
        Array<wbool> assign;
        /** The presumption array: we presume the value of a given variable */
        Array<wbool> presumption;
        /** The array containing the phase of the variables */
        Array<wbool> phase;
        /** 
         * The previous phase of the variables, used to compute the hamming
         * distance between the current phase and this one. 
         */
        Array<wbool> previousPhase;
        /**
         * The hamming distance between previousPhase and phase. The hamming
         * distance is the number of variable having a phase different.
         */
        unsigned int phaseHammingDistance;
        /**
         * This array will provide for each variable the id of the reduce
         * when we last enqueued it
         * Used to compute how many different variables we enqueue between two
         * reduce
         */
        Array<unsigned int> lastReduceSeen;
        /** 
         * The current number of different variable we have seen since last 
         * reduce 
         */
        unsigned int nbVarSeenBtwReduce;
        /** The current minimal deviation found */
        double minDeviation;
        /**
         * The array containing the binary clauses for reasons
         * It is arranged the following way:
         * |3*n  | a literal l whose variable is n
         * |3*n+1| the other literal that led to assigning l
         * |3*n+2| the largest unsigned int, to let the algorithms know that
         *             the clause has ended
         */
        Array<Lit> binReasons;
        /** The conflicting clause */
        Lit binConflict[3];
        /** The number of binary clauses */
        unsigned int nbBin;
        /** The array containing the reason to the choice of sign of a variable*/
        Array<Lit*> reasons;
        /** This array represent the occurrence list for binary clauses */
        Array<Array<Lit> > binWatched;
        /** The first non-binary clause watched by each literal */
        Array<Array<watcher_t> > watchedClauses;
        /** This array contains the clauses that are watched by only 1 literal */
        Array<Array<watcher_t> > monowatch;
        /** The value of the variable activity for each variable */
        Array<double> varActivityValue;
        /** The heap containing the activity of the variables */
        Heap<Var, VSIDSComp> varActivity;
        /** 
         * The value that will be added to the activity of a variable whenever
         * we need to increment its activity
         */
        double vsidsInc;
        /**
         * After each conflict, the value of vsidsInc must be changed. For this,
         * we are using the following value
         */
        double vsidsDec;
        /** The array containing all given clauses */
        Array<Clause*> clauses;
        /** The array containing every learnt clauses */
        Array<Clause*> learntClauses;
        /**
         * Propagation queue. This queue will contains the fact that were
         * discovered to be false with the current assignations.
         */
        Fifo<Lit> propagationQueue;
        /** 
         * The queue representing the different literals that were choosen to
         * be true, and those who where propagate. Last choosen/propagated will
         * be on top
         */
        Array<Lit> stack;
        /**
         * The position of the stack pointer at the beginning of each
         * assignation level. Helpfull for backtracking
         */
        Array<Lit> stackPointer;
        /** The current assignation level */
        unsigned int assignLevel;

        /** The number of literals removed from the initial problem */
        unsigned int nbLitRemoved;
        /**
         * The number of conflict left before we will perform a reduction of the
         * clause database
         */
        unsigned int nbConfBeforeReduce;
        /** The increment value of the number of conflict between two reduce */
        unsigned int reduceIncrement;
        /**
         * The current number of conflict that we must reach to perform a reduce
         */
        unsigned int reduceLimit;
        /** The number of reduce that were performed */
        unsigned int nbReducePerformed;
        /** The number of clauses that were removed */
        uint64_t nbClauseRemoved;
        
        /** 
         * The number of values taken for the average lbd over some last
         * conflicts
         */
        unsigned int localLbdAverageLength;
        /**
         * The difference factor allowed between the average lbd since last
         * restart and the average lbd since the last localLbdAverageLength conflicts
         */
        double lbdDifferenceFactor;
        /** Compute the average lbd over the life of this solver */
        Mean lbdMean;

        /** The sliding mean containing the last stack size before conflicts */
        SlidingMean stackSize;
        /** The difference factor used to contain restarts */
        double stackDifferenceFactor;

        /** The current total number of restart performed */
        uint64_t restarts;
        /** The current total number of propagations made */
        uint64_t nbPropag;
        /** The current total number of conflict found */
        uint64_t conflicts;

        /**
         * The current known state of the solver. If wTrue: the problem is SAT,
         * wFalse: the problem is UNSAT, wUnknown: we don't know
         */
        wbool state;

        /**
         * The u-value of the reluctant doubling suite
         * For more information about the reluctant doubling suite (aka luby)
         * please refer to "The art of computer programming, Volume 4B" by
         * Donald E. Knuth
         */
        int uvalue;
        /** The v-value of the reluctant doubling suite */
        int vvalue;
        /** The factor applied to the reluctant suite */
        int factor;

        /** The allocator used for the clauses generated by this solver */
        ClauseAllocator allocator;

        /** 
         * This array will map for each assignation level a value to indicate
         * 'when' was the last time we encountered that level.
         * 
         * It will be used to compute the value of literal block distance
         */
        Array<uint64_t> levelLBDChecked;
        
        /**
         * Counter used for the computation of the lbd. It needs to be updated
         * each time we compute an lbd, in order to not have side effects
         */
        uint64_t lbdTimeStamp;

        /**
         * For each lbd value, the number of learnt clauses having that lbd
         * value
         */
        Array<unsigned int> lbdValues;
        
        /**
         * This array is used in the analyze method to mark which variables have
         * already been seen
         */
        Array<bool> seen;

        /**
         * The mapping between the external variable names to the internal
         * variables names
         */
        Array<Var> map;

        /** The revert mapping */
        Array<Var> revertMap;

        /** The variable value for the next variable that has not been mapped yet */
        unsigned int nextVarMap;

        /**
         * This array is used whenever we are looking up for pure literals:
         * variables that appear using only one phase: either l or ¬l
         */
        Array<int> pureLitSearch;

        /** The number of search we did for pure literals */
        int nbPureLitSearch;

        /** The number of pure literals found */
        uint64_t nbPureLitFound;

        /** The number of initial clauses that were reduced */
        uint64_t nbInitialClausesReduced;

        /** The number of initial clauses that were removed */
        uint64_t nbInitialClausesRemoved;

        /**
         * This variable controls the verbosity of the solver. The higher the
         * value is, the more information will be printed on the standard logger
         */
        unsigned int verbosity;
        
#ifdef SATURNIN_PARALLEL
        /** The clause allocator used to export clauses */
        ClauseAllocator exchangedClauses;
        
        /** The parallel solver data */
        ParallelSolver* parallelData;
        
        /** 
         * The id of the thread that will hold this Solver if we are using a
         * parallel solver
         */
        unsigned int threadId;
#endif /* SATURNIN_PARALLEL */
        
#ifdef SATURNIN_DB
        /** The database wrapper used to retain the generated proof */
        DBWrapper db;
#endif

        friend class Simplifier;

        //! @cond Doxygen_Suppress
        //private and non declared as it may not be used
        /** MAY NOT BE USED (private and not implemented) */
        Solver(const Solver&) = delete;
        /** MAY NOT BE USED (private and not implemented) */
        Solver& operator=(const Solver&) = delete;
        //! @endcond

    };

}

#endif	/* SATURNIN_SOLVER_H */

