#include "saturnin/Solver.h"

#include "saturnin/Array.h"             // for Array
#include "saturnin/Assert.h"            // for ASSERT, ASSERT_EQUAL, SKIP
#include "saturnin/Clause.h"            // for Clause, watcher_t, etc
#include "saturnin/ClauseAllocator.h"   // for ClauseAllocator
#include "saturnin/Fifo.h"              // for Fifo
#include "saturnin/Heap.h"              // for Heap
#include "saturnin/Logger.h"            // for Logger
#include "saturnin/Mean.h"              // for Mean
#include "saturnin/RandomGenerator.h"   // for RandomGenerator
#include "saturnin/SlidingMean.h"       // for SlidingMean
#include "saturnin/VariablesManager.h"  // for Lit, VariablesManager, Var, etc
#include "saturnin/WeakBool.h"          // for wbool, wUnknown, wFalse, etc

#ifdef SATURNIN_PARALLEL
#include "saturnin/ParallelSolver.h"    // for ParallelSolver
#endif /* SATURNIN_PARALLEL */

#include <string.h>                     // for memccpy, memmove
#include <limits>                       // for numeric_limits
#include <cstring>                      // for memcpy

#include "../Utils.h"                   // for SATURNIN_BEGIN_PROFILE, etc

using namespace saturnin;


Solver::Solver(unsigned int i, unsigned int nbClauses, const char* 
#ifdef SATURNIN_DB
dbFileName
#endif /* SATURNIN_DB */
) :
asyncStop(false),
#ifdef PROFILE
__profile_propagateMono(0), __profile_propagateBin(0), __profile_propagateWatched(0),
__profile_reduce(0), __profile_analyze(0), __profile_simplify(0),
#endif /* PROFILE */
nbVar(i),
varLevel(nbVar), assign(nbVar), presumption(nbVar), phase(nbVar), previousPhase(nbVar),
phaseHammingDistance(0), lastReduceSeen(nbVar), nbVarSeenBtwReduce(0), minDeviation(100.0),
binReasons(nbVar * 3), nbBin(0), reasons(nbVar), binWatched(nbVar * 2),
watchedClauses(nbVar * 2), monowatch(nbVar * 2), varActivityValue(nbVar),
varActivity(nbVar, VSIDSComp(*this)),
vsidsInc(1.0), vsidsDec(0.95),
clauses(nbClauses), learntClauses(1024),
propagationQueue(128), stack(nbVar), stackPointer(256), assignLevel(0),
nbLitRemoved(0),
nbConfBeforeReduce(500), reduceIncrement(100), reduceLimit(500),
nbReducePerformed(0), nbClauseRemoved(0),
localLbdAverageLength(50), lbdDifferenceFactor(0.8), lbdMean(), stackSize(5000), stackDifferenceFactor(1.4),
restarts(0), nbPropag(0), conflicts(0), state(wUnknown),
uvalue(1), vvalue(1), factor(512),
allocator(10, 16), levelLBDChecked(nbVar), lbdTimeStamp(0), lbdValues(25),
seen(nbVar), map(nbVar), revertMap(nbVar), nextVarMap(0),
pureLitSearch(nbVar * 2), nbPureLitSearch(0), nbPureLitFound(0),
nbInitialClausesReduced(0), nbInitialClausesRemoved(0), verbosity(1)
#ifdef SATURNIN_PARALLEL
, exchangedClauses(10, 16), parallelData(nullptr), threadId(0)
#endif /* SATURNIN_PARALLEL */
#ifdef SATURNIN_DB
, db(dbFileName)
#endif
{
    ASSERT(i != 0);
    ASSERT(nbClauses != 0);
    //make sure we have enough space to holds up to the last variable
    ensureCapacity(nbVar - 1, 0);
    ASSERT_EQUAL(nbVar, assign.getSize());
    stackPointer.push(0);
}

Solver::~Solver() {
    while (clauses.getSize() > 0) {
        Clause* a = clauses.get(clauses.getSize() - 1);
        clauses.pop();
        allocator.releaseClause(a);
    }
    while (learntClauses.getSize() > 0) {
        Clause* a = learntClauses.get(learntClauses.getSize() - 1);
        learntClauses.pop();
        allocator.releaseClause(a);
    }
}

bool Solver::addClause(const Lit * const literals, unsigned int sz, bool mapped) {
    ASSERT(sz != 0);
    ASSERT_EQUAL((unsigned int) 0, assignLevel);
    ASSERT_EQUAL(1U, stackPointer.getSize());
    //if the formula was found SAT and we add a new clause, we may have UNSAT
    //now
    if (state == wTrue) {
        state = wUnknown;
    }
#ifdef SATURNIN_DB
    mapped = mapped || db.isInitialized();
#endif
    ArrayHelper<Lit> _hlp(sz);
    Lit* internalClause = (Lit*) _hlp;
    if (mapped) {
        for (unsigned int i = 0; i < sz; i++) {
            internalClause[i] = literals[i];
        }
    } else {
        for (unsigned int i = 0; i < sz; i++) {
            Var v = VariablesManager::getVar(literals[i]);
            ensureCapacity(v, nbVar);
            if (map[v] == var_Undef) {
                map[v] = nextVarMap;
                revertMap[nextVarMap] = v;
                nextVarMap++;
            }
            internalClause[i] = VariablesManager::getLit(map[v], VariablesManager::getLitSign(literals[i]));
        }
    }
    if (sz == 1) {
        Lit l = internalClause[0];
        Var v = VariablesManager::getVar(l);
        wbool b = VariablesManager::getLitSign(l) ? wTrue : wFalse;
        ensureCapacity(v, nbVar);

        //Check if we didn't prove the opposite literal
        if (assign[v] != wUnknown && assign[v] != b) {
            //we try to add the fact l where we already know ¬l
            state = wFalse;
            return false;
        }
        assign[v] = b;
        phase[v] = b;
        varLevel[v] = 0;
        propagationQueue.push(VariablesManager::oppositeLit(l));
        stack.push(l);
        
#ifdef SATURNIN_DB
        Lit dbC[2];
        dbC[0] = l;
        dbC[1] = std::numeric_limits<Lit>::max();
        long id = db.nbClausesAdded();
        db.addClause(dbC, id);
#endif

        Lit* conflict = propagate();
        if(conflict != nullptr) {
#ifdef SATURNIN_DB
            //compute the reason of the conflict and add the empty clause to
            //the database
            Array<Lit> learnt(2);
            computeLearntClause(conflict, learnt);
            db.addClause(&lit_Undef, db.nbClausesAdded());
#endif /* SATURNIN_DB */
            state = wFalse;
            return false;
        } else {
            return true;
        }
    } else if (sz == 2) {
        Lit a = internalClause[0];
        Var va = VariablesManager::getVar(a);
        ensureCapacity(va, nbVar);
        wbool ba = VariablesManager::getLitSign(a) ? wTrue : wFalse;
        if (assign[va] != wUnknown) {
            //either a has been proven or ¬a
            if (assign[va] == ba) {
                //the binary clause will always be satisfied
                return true;
            } else {
                //only the second literal may be not assign
                return addClause(internalClause + 1, 1, true);
            }
        }
        Lit b = internalClause[1];
        Var vb = VariablesManager::getVar(b);
        ensureCapacity(vb, nbVar);
        wbool bb = VariablesManager::getLitSign(b) ? wTrue : wFalse;
        if (assign[vb] != wUnknown) {
            //either a has been proven or ¬a
            if (assign[vb] == bb) {
                //the binary clause will always be satisfied
                return true;
            } else {
                //only the first literal may be not assign
                return addClause(internalClause, 1, true);
            }
        }

        binWatched[a].push(b);
        binWatched[b].push(a);
        
#ifdef SATURNIN_DB
        Lit clDB[3];
        clDB[0] = a;
        clDB[1] = b;
        clDB[2] = std::numeric_limits<Lit>::max();
        db.addClause(clDB, db.nbClausesAdded());
#endif /* SATURNIN_DB */
        nbBin++;


    } else {
        //simplify according to the current knowledge
        Array<Lit> tmp(sz);
        for (unsigned int i = 0; i < sz; i++) {
            Lit l = internalClause[i];
            Var v = VariablesManager::getVar(l);
            wbool b = VariablesManager::getLitSign(l) ? wTrue : wFalse;
            ensureCapacity(v, nbVar);

            //check if v hasn't been proved to be the other value
            if (assign[v] == wUnknown) {

                //check that we didn't already have added l or ¬l
                Lit nl = VariablesManager::oppositeLit(l);
                bool skip = false;
                for (unsigned int j = 0; !skip && j < tmp.getSize(); j++) {
                    if (tmp[j] == l) {
                        skip = true;
                    } else if (tmp[j] == nl) {
                        //the clause contains l and ¬l. Therefore, this clause
                        //will always be true
                        return true;
                    }
                }
                if (!skip) {
                    tmp.push(l);
                }
            } else {
                //either l or ¬l has been proved
                if (assign[v] == b) {
                    //bingo, one of the literal has been proven true. Therefore
                    //the clause will always be true
                    return true;
                } else {
                    //the clause contains a literals that has been proven false
                    //It is therefore useless to add it
                    SKIP;
                }
            }
        }
        if (tmp.getSize() == 0) {
            //The clause is now empty, therefore the instance is UNSAT
            state = wFalse;
            return false;
        } else if (tmp.getSize() == 1) {
            return addClause((const Lit*) tmp, 1, true);
        } else if (tmp.getSize() == 2) {
            return addClause((const Lit*) tmp, 2, true);
        }

        //TODO: check that the clause doesn't already exist

        //TODO: use a more intelligent sort, and do it before in order
        //      the operation in O(n log) instead of O(n²) (and even O(n)?)
        //      the intelligent sort must be fast for integers that all have
        //      different values
        for (unsigned int k = 0; k < tmp.getSize(); k++) {
            for (unsigned int l = k + 1; l < tmp.getSize(); l++) {
                if (tmp[k] > tmp[l]) {
                    Lit t = tmp[k];
                    tmp[k] = tmp[l];
                    tmp[l] = t;
                }
            }
        }

        Clause* newClause = allocator.createClause(tmp);
        addWatchedClause(newClause);


        //TODO: update the counters of nb clause per literal

        clauses.push(newClause);
#ifdef SATURNIN_DB
        db.addClause((Lit*)newClause, db.nbClausesAdded());
#endif /* SATURNIN_NODB */

    }
    return true;
}

Clause* Solver::addLearntClause(const Array<Lit>& clause) {

    ASSERT(clause.getSize() > 0);
    if (clause.getSize() == 1) {
        enqueue(clause[0]);
        return nullptr;
    } else if (clause.getSize() == 2) {
        Lit a = clause[0];
        Var va = VariablesManager::getVar(a);
        Lit b = clause[1];
        binWatched[a].push(b);
        binWatched[b].push(a);
        nbBin++;
        binReasons[3 * va] = a;
        binReasons[3 * va + 1] = b;
        ASSERT_EQUAL(lit_Undef, binReasons[3 * va + 2]);
        Lit* reason = ((Lit*) binReasons) + 3 * va;
        enqueue(a, reason);
#ifdef SATURNIN_DB
        db.addClause((Lit*)reason, db.nbClausesAdded());
#endif /* SATURNIN_DB */
        return nullptr;
    } else {
        Clause* newClause = allocator.createClause(clause);
        addWatchedClause(newClause);

        ASSERT(newClause->isAttached());

        unsigned int lbd = computeLBD(newClause);
        ASSERT(lbd != 0);
        while (lbd >= lbdValues.getSize()) {
            lbdValues.push(0);
        }
        lbdValues[lbd]++;
        newClause->setLBD(lbd);

        newClause->setIndex(learntClauses.getSize());
        learntClauses.push(newClause);
#ifdef SATURNIN_DB
        db.addClause((Lit*)newClause, db.nbClausesAdded());
#endif /* SATURNIN_DB */
        ASSERT(newClause->isLearnt());

        //TODO: Update the counters

        //TODO: check if we must export the clause

        enqueue(clause[0], newClause->lits());

        return newClause;
    }

}

unsigned int Solver::computeLBD(Clause* clause) {
    lbdTimeStamp++;
    unsigned int lbd = 0;
    for (unsigned int i = 0; i < clause->getSize() && lbd < SATURNIN_MAX_LBD_VALUE; i++) {
        Var v = VariablesManager::getVar(clause->getLit(i));
        //compute the lbd
        unsigned int varLvl = varLevel[v];
        if (levelLBDChecked[varLvl] != lbdTimeStamp) {
            levelLBDChecked[varLvl] = lbdTimeStamp;
            lbd++;
        }
    }
    return lbd;
}

void Solver::removeLearntClause(Clause* c) {

    nbClauseRemoved++;
    ASSERT(c->isLearnt());
    ASSERT(c->getSize() > 1);

    //Remove the clause from the watched structures
    if (c->isAttached()) {
        stopWatchClause(c->getLit(0), c);
        stopWatchClause(c->getLit(1), c);
    }


    //Remove the clause from the learntClauses list by putting it at the end
    //of the list and then 'popping' it
    unsigned int i = c->getIndex();
    ASSERT(learntClauses.getSize() != 0);
    unsigned int last = learntClauses.getSize() - 1;
    ASSERT(i <= last);
    ASSERT(c == learntClauses[i]);
    if (i != last) {
        Clause* tmp = learntClauses[last];
        //put clause c as last element in the learnt clauses vector
        learntClauses[i] = tmp;
        tmp->setIndex(i);
#ifdef DEBUG
        learntClauses[last] = c;
#endif /* DEBUG */
    }
    ASSERT(c == learntClauses[last]);
    learntClauses.pop();

    ASSERT(lbdValues[c->getLBD()] > 0);
    lbdValues[c->getLBD()]--;

    //Remove the clause from memory
    allocator.releaseClause(c);
}

void Solver::addWatchedClause(Clause* c) {
    Lit a = c->getLit(0);
    Lit b = c->getLit(1);
    watchedClauses[a].push(watcher_t(c, b));
    watchedClauses[b].push(watcher_t(c, a));
}

void Solver::stopWatchClause(const Lit l, const Clause* c) {
    watcher_t* i = (watcher_t*) watchedClauses[l];
    watcher_t* end = i + watchedClauses[l].getSize();
    while (i != end) {
        if (i->watched == c) {
            while (i + 1 != end) {
                *i = *(i + 1);
                i++;
            }
            watchedClauses[l].pop();
            return;
        }
        i++;
    }
}

bool Solver::reduceClauseEval(const Clause* c, unsigned int medianLBD, int maxToBeUsefull, int& trueLits) {
    bool containsProvenLit = c->getLBD() > medianLBD;
    if (c->getLBD() >= 3) {
        for (unsigned int j = 0; j < c->getSize() && !containsProvenLit && trueLits <= maxToBeUsefull; j++) {
            Lit l = c->getLit(j);
            Var v = VariablesManager::getVar(l);
            //check if the clause doesn't contain any proven literal
            containsProvenLit = assign[v] != wUnknown &&
                    assign[v] == (VariablesManager::getLitSign(l) ? wTrue : wFalse);

            if (phase[v] == (VariablesManager::getLitSign(l) ? wTrue : wFalse)) {
                trueLits++;
            }
        }
    }
    return containsProvenLit;
}

void Solver::reduce() {

    SATURNIN_BEGIN_PROFILE;
    nbReducePerformed++;
    //some values that are used to show som stats
    double deviation = 0;
    unsigned int nbClausesCurrentlyUsed = 0;

#ifdef DEBUG
    unsigned int ts = 0;
    for (unsigned int i = 0; i < lbdValues.getSize(); i++) {
        ts += lbdValues[i];
    }
    ASSERT_EQUAL(ts, learntClauses.getSize());
#endif /* DEBUG */

    unsigned int sum = 0;
    unsigned int medianLBD = 0;
    while (medianLBD + 1 < lbdValues.getSize() && sum + lbdValues[medianLBD + 1] < learntClauses.getSize() / 2) {
        sum += lbdValues[medianLBD];
        medianLBD++;
    }
    medianLBD+=2;

    ASSERT_EQUAL(assignLevel, 0U);
    ASSERT_EQUAL(0U, propagationQueue.getSize());

    //compute the hamming distance
    phaseHammingDistance = 0;
    for (unsigned int i = 0; i < phase.getSize(); i++) {
        if (phase[i] != previousPhase[i]) {
            phaseHammingDistance++;
        }
    }

    //Copy the current phase
    std::memcpy((wbool*) previousPhase, (wbool*) phase, previousPhase.getSize() * sizeof (wbool));

    //compute the deviation
    deviation = (1.0 * phaseHammingDistance) / nbVarSeenBtwReduce;
    if (deviation > minDeviation) {
        deviation = minDeviation;
    } else if (minDeviation > deviation) {
        minDeviation = deviation;
    }
    deviation = deviation < 0.01 ? 0.1 : deviation;

    //reset the var used to compute the deviation for the next time
    phaseHammingDistance = 0;
    nbVarSeenBtwReduce = 0;

    Array<Clause*> toDesactivate;

    unsigned int i = learntClauses.getSize() - 1;

    while (i < learntClauses.getSize() && !Solver::asyncStop) {

        Clause* c = learntClauses[i];

        if (!c->isAttached()) {
            //we will deal with those later
            i--;
            continue;
        }

        //compute the maximum number of literals that we allow to be the same
        //polarity as the phase for that clause
        //Thanks to the deviation we know an approximation of many changes
        //we could do to the polarity of the variables. If there are too many
        //changes needed to make the clause falsified, we will freeze it
        int maxToBeUsefull = (int) ((c->getSize() * deviation));
        int trueLits = 0;
        bool containsProvenLit = reduceClauseEval(c, medianLBD, maxToBeUsefull, trueLits);

        if (containsProvenLit) {
            //clause is useless
            removeLearntClause(c);
        } else if (trueLits > maxToBeUsefull) {
            //The clause is "too far" from current interpretation
            c->incrementFreezeCounter();
            if (c->getFreezeCounter() > 7U || c->getLBD() > medianLBD) {
                removeLearntClause(c);
            } else /* if (c->isAttached()) => always true */ {
                toDesactivate.push(c);
            }
        } else {
            //This clause could be a usefull clause
            nbClausesCurrentlyUsed++;
        }
        i--;
    }

    unsigned int monowatched = toDesactivate.getSize();
    for (unsigned int j = 0; j < monowatch.getSize(); j++) {
        watcher_t* current = monowatch[j];
        watcher_t* dest = current;
        watcher_t* end = current + monowatch[j].getSize();
        while (current != end) {
            Clause* c = current->watched;
            ASSERT(!c->isAttached());
            int maxToBeUsefull = (int) ((c->getSize() * deviation));
            int trueLits = 0;
            bool containsProvenLit = reduceClauseEval(c, medianLBD, maxToBeUsefull, trueLits);

            if (containsProvenLit) {
                removeLearntClause(c);
                current++;
            } else if (trueLits > maxToBeUsefull) {
                c->incrementFreezeCounter();
                monowatched++;
                *dest = *current;
                dest++;
                current++;
            } else {
                addWatchedClause(c);
                c->setAttached(true);
                current++;
            }
        }
        monowatch[j].pop(static_cast<unsigned int>(end - dest));
    }

    //Deactivate the clause that have to be deactivated
    while (toDesactivate.getSize() > 0) {
        Clause* c = toDesactivate.getLast();
        ASSERT(c->isAttached());
        toDesactivate.pop();
        c->setAttached(false);
        stopWatchClause(c->getLit(0), c);
        stopWatchClause(c->getLit(1), c);
        monowatch[c->getLit(0)].push(watcher_t(c, c->getLit(1)));
    }

    phasedElimination();
    allocator.clearup();

    if (verbosity > 0) {
        ::printf("c %12ld | %10d | %14d\n",
                conflicts, learntClauses.getSize(), monowatched);
    }

    SATURNIN_END_PROFILE(__profile_reduce);
}

void Solver::updateVSIDS(Var v) {

    varActivityValue[v] += vsidsInc;
    if (varActivityValue[v] > 1e100) {
        for (unsigned int i = 0; i < varActivityValue.getSize(); i++) {
            varActivityValue[i] *= 1e-100;
        }
        vsidsInc *= 1e-100;
    }
    if (varActivity.contains(v)) {
        varActivity.updateValue(v);
    }
}

bool Solver::addClause(const Array<Lit>& c) {
    return addClause((const Lit*) c, c.getSize());
}

void Solver::ensureCapacity(Var v, unsigned int nbInitialized) {
    if (nbInitialized > v) {
        return;
    }

    RandomGenerator rand(
#ifdef SATURNIN_PARALLEL
    threadId+
#endif /* SATURNIN_PARALLEL */
    nbInitialized);
    unsigned int nbVarsToAdd = v + 1 - assign.getSize();
    for (unsigned int i = 0; i < nbVarsToAdd; i++) {
        map.push(var_Undef);
        revertMap.push(var_Undef);
        binReasons.push(assign.getSize());
        binReasons.push(lit_Undef);
        binReasons.push(lit_Undef);
        assign.push(wUnknown);
        wbool curPhase = wFalse;
#ifdef SATURNIN_PARALLEL
        if(threadId != 0 && (rand.getNext()+threadId)%2 == 0){
            curPhase = wTrue;
        }
#endif /* SATURNIN_PARALLEL */
        phase.push(curPhase);
        presumption.push(wUnknown);
        previousPhase.push(wFalse);
        varActivityValue.push(0.0);
        varActivity.add(nbInitialized + i);
        varLevel.push(0);
        levelLBDChecked.push(0);
        binWatched.push();
        watchedClauses.push();
        monowatch.push();
        reasons.push((Lit*) nullptr);
        seen.push(false);
        lastReduceSeen.push((unsigned int) - 1);
        pureLitSearch.push(0);
    }
    for (unsigned int i = 0; i < nbVarsToAdd; i++) {
        binWatched.push();
        watchedClauses.push();
        monowatch.push();
        pureLitSearch.push(0);
    }
    nbVar = assign.getSize();
}

Lit* Solver::binaryPropagate(const Lit l) {
    SATURNIN_BEGIN_PROFILE;
    Lit* conflict = nullptr;
    for (unsigned int i = 0; i < binWatched[l].getSize(); i++) {
        Lit other = binWatched[l][i];
        Var vother = VariablesManager::getVar(other);
        if (assign[vother] != wUnknown) {
            if (assign[vother] == (VariablesManager::getLitSign(other) ? wTrue : wFalse)) {
                continue;
            } else {
                //conflict found, we have to return the clause other \/ l
                //however, there is already a clause for vother
                //therefore, we need an extra space
                binConflict[0] = other;
                binConflict[1] = l;
                binConflict[2] = lit_Undef;
                conflict = binConflict;
                i = binWatched[l].getSize();
            }
        } else {
            binReasons[3 * vother] = other;
            binReasons[3 * vother + 1] = l;
            ASSERT_EQUAL(lit_Undef, binReasons[3 * vother + 2]);
            enqueue(other, ((Lit*) binReasons) + 3 * vother);
        }
    }
    SATURNIN_END_PROFILE(__profile_propagateBin);
    return conflict;
}

Lit* Solver::propagate() {
    Lit* conflict = nullptr;
    while (propagationQueue.getSize() > 0 && conflict == nullptr) {
        nbPropag++;
        const Lit l = propagationQueue.getFirst();
        propagationQueue.pop();

        //first, let's propagate binary clauses
        conflict = binaryPropagate(l);

        if (conflict == nullptr) {
            conflict = propagateWatchedBy(l);
        }

        if (conflict == nullptr) {
            conflict = propagateMonoWatched(l);
        }

    }

#ifdef DEBUG

    //verify that the conflicting clause is unsatisfied
    if (conflict != nullptr) {
        for (unsigned int i = 0; conflict[i] != lit_Undef; i++) {
            Lit l = conflict[i];
            Var v = VariablesManager::getVar(l);
            ASSERT(assign[v] != wUnknown);
            ASSERT(assign[v] != (VariablesManager::getLitSign(l) ? wTrue : wFalse));
        }
    }

#endif /* DEBUG */

    if (conflict != nullptr) {
        //TODO: change this with an make empty procedure
        while (propagationQueue.getSize() > 0) {
            propagationQueue.pop();
        }
    }

    return conflict;
}

Lit* Solver::propagateMonoWatched(const Lit l) {
    SATURNIN_BEGIN_PROFILE;
    Lit* conflict = nullptr;
    while (conflict == nullptr && monowatch[l].getSize() > 0) {

        watcher_t nextMonoClause = monowatch[l].getLast();
        monowatch[l].pop();
        ASSERT(nextMonoClause.watched->contains(l));
        ASSERT(nextMonoClause.watched->contains(nextMonoClause.block));

        Var vblock = VariablesManager::getVar(nextMonoClause.block);
        if (assign[vblock] == wUnknown || assign[vblock] == (VariablesManager::getLitSign(nextMonoClause.block) ? wTrue : wFalse)) {
            monowatch[nextMonoClause.block].push(watcher_t(nextMonoClause.watched, l));
            continue;
        }

        Clause* cur = nextMonoClause.watched;
        ASSERT(!cur->isAttached())
        //look for a new watch
        Lit* candidate = cur->lits();
        Var v = VariablesManager::getVar(*candidate);


        //p is the position of l
        unsigned int p = std::numeric_limits<unsigned int>::max();
        bool found = false;
        while (!found && *candidate != lit_Undef) {
            if (*candidate == l) {
                p = static_cast<unsigned int>(candidate - cur->lits());
            }
            found = assign[v] == wUnknown || assign[v] == (VariablesManager::getLitSign(*candidate) ? wTrue : wFalse);
            if (!found) {
                candidate++;
                v = VariablesManager::getVar(*candidate);
            }
        }
        if (!found) {
            //no new watch has been found. Therefore, a conflict appear
            conflict = cur->lits();

            ASSERT(*candidate == lit_Undef);
            ASSERT(p < std::numeric_limits<unsigned int>::max());
            ASSERT_EQUAL(l, cur->getLit(p));

            //remove the clause from the monowatch and put it back to the
            //'normal' data-structures
            cur->swapLiterals(p, 1);
            cur->setAttached(true);
            addWatchedClause(cur);

#ifdef DEBUG
            for (unsigned int k = 0; k < monowatch[l].getSize(); k++) {
                ASSERT(!monowatch[l][k].watched->isAttached());
            }
#endif /* DEBUG */

        } else {
            ASSERT_EQUAL(VariablesManager::getVar(*candidate), v);
            ASSERT(cur->contains(*candidate));
            //we have found another watch
            //change the watching literal by the one set to true
            ASSERT(*candidate != l);
            monowatch[*candidate].push(watcher_t(cur, l));
        }
    }
    if (conflict == nullptr) {
        monowatch[l].pop(monowatch[l].getSize());
    } else {
        ASSERT_EQUAL(l, *(conflict + 1));
    }
    SATURNIN_END_PROFILE(__profile_propagateMono);
    return conflict;
}

Lit* Solver::propagateWatchedBy(const Lit l) {
    SATURNIN_BEGIN_PROFILE;
    Lit* conflict = nullptr;
    //The pointer to the previous clause
    watcher_t* currentWatch = (watcher_t*) watchedClauses[l];
    watcher_t* end = currentWatch + watchedClauses[l].getSize();
    watcher_t* destCopy = currentWatch;

    long int nbskip = 0;
    while (currentWatch != end && conflict == nullptr) {
        bool foundNewWatch = false;

        ASSERT(currentWatch->watched->getLit(0) == l || currentWatch->watched->getLit(1) == l);

        //check if the other literal hasn't been proven true at this
        //level
        Lit block = currentWatch->block;
        Var vBlock = VariablesManager::getVar(block);

        if (assign[vBlock] != wUnknown &&
                assign[vBlock] == (VariablesManager::getLitSign(block) ? wTrue : wFalse)) {
            //ASSERT(varLevel[vBlock] <= assignLevel);
            //go to next clause
            *destCopy = *currentWatch;
            currentWatch++;
            destCopy++;
            continue;
        }

        Clause* cur = currentWatch->watched;

        //make sure that l is at position 1
        //That way, if the other watched literal is set to true, we have
        //the true literal at position 0
        //This fact is used in analyze
        if (cur->getLit(1) != l) {
            cur->swapLiterals(0, 1);
        }
        ASSERT_EQUAL(l, cur->getLit(1));

        //update the blocker
        currentWatch->block = cur->getLit(0);

#ifdef DEBUG
        Clause* next = currentWatch + 1 == end ? nullptr : (currentWatch + 1)->watched;
        ASSERT(next == nullptr || (next->getLit(0) == l || next->getLit(1) == l));
#endif /* DEBUG */

        Lit first = cur->getLit(0);
        Var vFirst = VariablesManager::getVar(first);
        if (assign[vFirst] != wUnknown &&
                assign[vFirst] == (VariablesManager::getLitSign(first) ? wTrue : wFalse)) {
            *destCopy = *currentWatch;
            currentWatch++;
            destCopy++;
            continue;
        }

        //look for a new watch
        unsigned int pos = 2;
        for (; !foundNewWatch && pos < cur->getSize(); pos++) {
            Lit currentLit = cur->getLit(pos);
            Var currentVar = VariablesManager::getVar(currentLit);
            //currentLit is our next watch if the literal is true or
            //if no value has yet been assigned to currentVar
            foundNewWatch = assign[currentVar] == wUnknown ||
                    (//assignLevel >= varLevel[currentVar] &&  <= not needed as otherwise it would be assign[currentVar] == wUnknown
                    assign[currentVar] == (VariablesManager::getLitSign(currentLit) ? wTrue : wFalse));
        }

        if (foundNewWatch) {
            //We found a new watcher

            Lit lTmp = cur->getLit(pos - 1);
            ASSERT(assign[VariablesManager::getVar(lTmp)] == wUnknown ||
                    assign[VariablesManager::getVar(lTmp)] == (VariablesManager::getLitSign(lTmp) ? wTrue : wFalse));
            cur->swapLiterals(1, pos - 1);

            //Remove the current clause from the list of l
            currentWatch++;
            nbskip++;

            //Add the clause to the watched list of the literal of lTmp
            ASSERT(cur->getLit(1) == lTmp);
            ASSERT(lTmp != l);
            watchedClauses[lTmp].push(watcher_t(cur, cur->getLit(0)));
        } else {
            //We didn't found any other watch, we must enqueue another
            //or we may have found a conflict
            Lit secondLit = cur->getLit(0);
            Var secondVar = VariablesManager::getVar(secondLit);
            if (assign[secondVar] != wUnknown &&
                    assign[secondVar] != (VariablesManager::getLitSign(secondLit) ? wTrue : wFalse)) {
                ASSERT(varLevel[secondVar] <= assignLevel);
                //we found a conflict!
                conflict = cur->lits();

                //copy the remaining watches
                memmove(destCopy, currentWatch, (end - currentWatch) * sizeof (watcher_t));

            } else{ 
                ASSERT(assign[secondVar] == wUnknown);
                enqueue(secondLit, cur->lits());

                //we may re-compute the lbd value of the clause
                if (cur->getLBD() > 3) {
                    unsigned int lbd = computeLBD(cur);
                    ASSERT(lbd < cur->getSize());
                    if (lbd < cur->getLBD()) {
                        ASSERT(lbdValues[cur->getLBD()] > 0);
                        lbdValues[cur->getLBD()]--;
                        cur->setLBD(lbd);
                        ASSERT(lbd != 0);
                        while (lbd >= lbdValues.getSize()) {
                            lbdValues.push(0);
                        }
                        lbdValues[lbd]++;
                    }
                }

                *destCopy = *currentWatch;
                destCopy++;
                currentWatch++;
            }
        }
    }
    ASSERT(nbskip == end - destCopy || conflict != nullptr);

    watchedClauses[l].pop(nbskip);
    SATURNIN_END_PROFILE(__profile_propagateWatched);
    return conflict;
}

bool Solver::litRedundant(Lit p, unsigned int abstract_levels, Array<Lit>& analyze_toclear) {
    Array<Lit> analyze_stack;
#ifdef SATURNIN_DB
    Array<Lit*> resolvedClauses;
#endif /* SATURNIN_DB */
    analyze_stack.push(p);
    int top = analyze_toclear.getSize();
    while (analyze_stack.getSize() > 0) {
        ASSERT(reasons[VariablesManager::getVar(analyze_stack.get(analyze_stack.getSize() - 1))] != nullptr);
        Lit* c = reasons.get(VariablesManager::getVar(analyze_stack.get(analyze_stack.getSize() - 1)));
        analyze_stack.pop();
#ifdef SATURNIN_DB
        resolvedClauses.push(c);
#endif /* SATURNIN_DB */
        for (unsigned int i = 1; c[i] != lit_Undef; i++) {
            Lit l = c[i];
            Var v = VariablesManager::getVar(l);
            if (!seen[v] && varLevel.get(v) > 0) {
                if (reasons[v] != nullptr &&
                        (abstractLevel(v) & abstract_levels) != 0) {
                    seen[v] = 1;
                    analyze_stack.push(l);
                    analyze_toclear.push(l);
                } else {
                    for (unsigned int j = top; j < analyze_toclear.getSize(); j++) {
                        seen[VariablesManager::getVar(analyze_toclear[j])] = 0;
                    }
                    analyze_toclear.pop(analyze_toclear.getSize() - top);
                    return false;
                }
            }
        }
    }
    
#ifdef SATURNIN_DB
    for(unsigned int i = 0; i < resolvedClauses.getSize(); i++) {
        db.addResolution(resolvedClauses[i], db.nbClausesAdded());
    }
#endif /* SATURNIN_DB */
    return true;
}

void Solver::computeLearntClause(Lit* conflictingClause, Array<Lit>& learnt){
#ifdef DEBUG
    unsigned int nbVarThisLvl = 0;
    for(unsigned int i = 0; conflictingClause[i]!=lit_Undef; i++){
        ASSERT(assign[VariablesManager::getVar(conflictingClause[i])] != wUnknown);
        ASSERT(getLitValue(conflictingClause[i]) == wFalse);
        if(varLevel[VariablesManager::getVar(conflictingClause[i])] == assignLevel){
            nbVarThisLvl++;
        }
    }
    ASSERT(nbVarThisLvl > 0);
#endif /* DEBUG */
    int nbElementToCheck = 0;
    Lit p = 0;

    // Generate conflict clause:s
    learnt.push(p); // (leave room for the asserting literal)
    ASSERT(stack.getSize() > 0);
    unsigned int index = stack.getSize() - 1;

#ifdef DEBUG
    ASSERT_EQUAL(nbVar, seen.getSize());
    for (unsigned int i = 0; i < nbVar; i++) {
        ASSERT_EQUAL(false, seen[i])
    }
#endif

    //used to simplify the clause
    unsigned int abstract_level = 0;
    Array<Lit> toclear;


    Lit* c = conflictingClause;
    do {

        ASSERT(c != nullptr);
        ASSERT(c == conflictingClause || p == c[0]);
        
#ifdef SATURNIN_DB
        db.addResolution(c, db.nbClausesAdded());
#endif /* SATURNIN_DB */
        
        //The first time we are going through this loop, we have to go through
        //each literal.
        //after that, we know that we have already been through the first literal
        //as it lead to analyzing that new clause. Therefore, we don't need to
        //analyze for it again
        for (unsigned int j = (c == conflictingClause) ? 0 : 1;
                c[j] != lit_Undef; j++) {
            Lit l = c[j];
            Var v = VariablesManager::getVar(l);

            //As the clause c was used as a reason, every var used in the clause
            //should have been assigned
            ASSERT(assign[v] != wUnknown);

            if (!seen[v] && varLevel[v] > 0) {
                updateVSIDS(v);
                seen[v] = true;
                if (varLevel[v] >= assignLevel) {
                    nbElementToCheck++;
                } else {
                    learnt.push(l);

                    //maintain an abstraction of levels involved in conflict
                    //used for the simplification of the clause
                    abstract_level |= abstractLevel(v);
                    //remember that we have to clear the value of seen for this
                    //literal
                    toclear.push(l);
                }
            }
#ifdef SATURNIN_DB
            else if(varLevel[v] == 0){
                //the variable has been proved
                Lit unary[] = {l, lit_Undef};
                db.addResolution(unary, db.nbClausesAdded());
            }
#endif /* SATURNIN_DB*/
        }


        // Select next clause to look at
        while (
#ifdef SATURNIN_DB
                index < stack.getSize() &&
#endif /* SATURNIN_DB */
                !seen[VariablesManager::getVar(stack[index])]) {
            index--;
        }
#ifdef SATURNIN_DB
        if(index < stack.getSize()){
#endif /* SATURNIN_DB */
            p = stack[index];
            index--;

            c = reasons[VariablesManager::getVar(p)];

            //make sure that the literal that we will use to analyze the next
            //clause is at position 0 in that new clause
            ASSERT_EQUAL(VariablesManager::getVar(p),
                    (c == nullptr ? VariablesManager::getVar(p) : VariablesManager::getVar(c[0])));
            toclear.push(p);
#ifdef SATURNIN_DB
        } else {
            c = nullptr;
        }
#endif /* SATURNIN_DB */
        nbElementToCheck--;

    } while (nbElementToCheck > 0);
    learnt[0U] = VariablesManager::oppositeLit(p);

#ifdef DEBUG

    //Check that the first literal is from the highest level, and is the only
    //one from that level
    for (unsigned int i = 1; i < learnt.getSize(); i++) {
        ASSERT(varLevel[VariablesManager::getVar(learnt.get(i))] <
                varLevel[VariablesManager::getVar(learnt.get(0))]);
    }

#endif /* DEBUG */


    //simplify the learnt clause
    unsigned int i = 1;
    unsigned int j = 1;
    for (; i < learnt.getSize(); i++) {
        if (reasons[VariablesManager::getVar(learnt[i])] == nullptr ||
                !litRedundant(learnt[i], abstract_level, toclear)) {
            learnt[j] = learnt[i];
            j++;
        }
    }
    ASSERT(j <= i);
    //remove the useless literals
    learnt.pop(i - j);


    //clear the seen array
    for (unsigned int k = 0; k < toclear.getSize(); k++) {
        seen[VariablesManager::getVar(toclear.get(k))] = false;
    }
}

unsigned int Solver::analyze(Lit* conflictingClause, Array<Lit>& learnt) {
    SATURNIN_BEGIN_PROFILE;
    computeLearntClause(conflictingClause, learnt);

    //Now, we need to look up for the highest level of the different assignment
    //levels of the literals learnt[1] to learnt[size-1]
    //This will provide the level on which we have to backtrack
    unsigned int destinationLevel;

    if (learnt.getSize() == 1) {
        //hurray! we found a fact, let's make a backtrack up to level 0
        //this mean that we do a restart, without leaving the search function
        destinationLevel = 0;
    } else {
        unsigned int pos = 1;
        unsigned int maxV = varLevel[VariablesManager::getVar(learnt[pos])];
        for (unsigned int k = 2; k < learnt.getSize(); k++) {
            if (varLevel[VariablesManager::getVar(learnt[k])] > maxV) {
                maxV = varLevel[VariablesManager::getVar(learnt[k])];
                pos = k;
            }
        }
        destinationLevel = maxV;
        //make sure that the literal of level maxV is at position 1
        //that way, it will be one of the watched literals
        Lit ltmp = learnt[pos];
        learnt[pos] = learnt[1U];
        learnt[1U] = ltmp;
    }

#ifdef DEBUG
    //check that the created clause is well entirely false at the current
    //assignation level
    for (unsigned int k = 0; k < learnt.getSize(); k++) {
        ASSERT(assign[VariablesManager::getVar(learnt[k])] != wUnknown);
        ASSERT(assign[VariablesManager::getVar(learnt[k])] !=
                (VariablesManager::getLitSign(learnt[k]) ? wTrue : wFalse));
    }

    for (unsigned int k = 0; k < nbVar; k++) {
        ASSERT_EQUAL(false, seen[k])
    }

#endif

    SATURNIN_END_PROFILE(__profile_analyze);

    ASSERT(assignLevel >= destinationLevel);
    return assignLevel - destinationLevel;

}

wbool Solver::simplify() {
    SATURNIN_BEGIN_PROFILE;
    ASSERT_EQUAL((unsigned int) 0, assignLevel);
    bool stopSimplification = state != wUnknown;
    unsigned int nbRemoved = 0U;
    unsigned int nbReduced = 0U;
    while (!stopSimplification && !asyncStop) {
        stopSimplification = true;
        unsigned int i = 0;
        while (i < clauses.getSize() && !asyncStop) {
            Clause* c = clauses[i];
            bool removed = false;
            unsigned int j = 0;
            bool simpl = false;
            while (!removed && !simpl && j < c->getSize()) {
                Lit aLit = c->getLit(j);
                if (assign[VariablesManager::getVar(aLit)] != wUnknown) {
                    wbool v = getLitValue(aLit);
                    ASSERT(v != wUnknown);
                    if (v == wTrue) {
                        //The clause contains a literal proven true, we may remove
                        //the clause from our learnt clause database
                        ASSERT(c == clauses[i]);
                        clauses[i] = clauses[clauses.getSize() - 1];
                        clauses.pop();
                        stopWatchClause(c->getLit(0), c);
                        stopWatchClause(c->getLit(1), c);
                        allocator.releaseClause(c);
                        removed = true;
                        nbRemoved++;
                    } else {
                        //The clause contains a literal proven false, we may
                        //simplify the clause
                        ASSERT(c == clauses[i]);
                        stopWatchClause(c->getLit(0), c);
                        stopWatchClause(c->getLit(1), c);
                        if (!addClause(c->lits(), c->getSize(), true)) {
                            state = wFalse;
                        }
                        clauses[i] = clauses[clauses.getSize() - 1];
                        clauses.pop();
                        allocator.releaseClause(c);
                        simpl = true;
                        nbReduced++;
                        j++;
                    }
                } else {
                    j++;
                }
            }
            if (!removed) {
                i++;
            }
        }
    }
    nbInitialClausesRemoved += nbRemoved;
    nbInitialClausesReduced += nbReduced;

    if (state == wUnknown && getNbClauses() == 0) {
        state = wTrue;
    }
    SATURNIN_END_PROFILE(__profile_simplify);
    return state;
}

wbool Solver::phasedElimination() {

    ASSERT_EQUAL(0U, assignLevel);
    ASSERT_EQUAL(2 * nbVar, pureLitSearch.getSize());
    ASSERT_EQUAL(0U, propagationQueue.getSize());

    if (state == wUnknown) {
        nbPureLitSearch++;

        //check which literals are used by the clauses of the initial problem
        for (unsigned int i = 0; i < clauses.getSize(); i++) {
            Clause* c = clauses[i];
            for (unsigned j = 0; j < c->getSize(); j++) {
                Lit l = c->getLit(j);
                pureLitSearch[l] = nbPureLitSearch;
            }
        }
        //check which literals are used by binary clauses
        for (unsigned int i = 0; i < binWatched.getSize(); i++) {
            //we don't need to go through the elements of binWatched[i] since
            //for each element x in binWatched[i], binWatched[x] contains
            //i
            if (binWatched[i].getSize() != 0) {
                pureLitSearch[i] = nbPureLitSearch;
            }
        }
        //check which literals are used by the learnt clauses
        for (unsigned int i = 0; i < learntClauses.getSize(); i++) {
            Clause* c = learntClauses[i];
            for (unsigned j = 0; j < c->getSize(); j++) {
                Lit l = c->getLit(j);
                pureLitSearch[l] = nbPureLitSearch;
            }
        }

        unsigned int nbFound = 0;

        for (unsigned int i = 0; i < nbVar; i++) {
            if (assign[i] == wUnknown) {
                int flagT = pureLitSearch[VariablesManager::getLit(i, true)];
                int flagF = pureLitSearch[VariablesManager::getLit(i, false)];
                if (flagT != nbPureLitSearch && flagF == nbPureLitSearch) {
                    nbFound++;
                    //l is not used but ¬l is
                    enqueue(VariablesManager::getLit(i, false));
                } else if (flagF != nbPureLitSearch && flagT == nbPureLitSearch) {
                    //¬l is not used but l is
                    nbFound++;
                    enqueue(VariablesManager::getLit(i, true));
                }
            }
        }

        nbPureLitFound += nbFound;

#ifdef DEBUG
        Lit* b =
#endif /* DEBUG */
                propagate();
#ifdef DEBUG
        ASSERT(b == nullptr);
#endif


    }


    return state;

}

bool Solver::solve(unsigned int maxNbRestarts) {
    state = simplify();
    state = phasedElimination();
    while (state == wUnknown && restarts <= maxNbRestarts && !asyncStop) {
        restarts++;
        ASSERT_EQUAL((unsigned int) 0, assignLevel);
        state = search(vvalue * factor);
        //compute the next uvalue and vvalue
        if ((uvalue & -uvalue) == vvalue) {
            uvalue = uvalue + 1;
            vvalue = 1;
        } else {
            vvalue = vvalue * 2;
        }
        if (state == wUnknown && restarts <= maxNbRestarts && assignLevel > 0) {
            backtrack(assignLevel);
            ASSERT_EQUAL(0U, assignLevel);
            if (propagationQueue.getSize() > 0) {
                propagationQueue.pop();
            }
#ifdef SATURNIN_PARALLEL
            //import the clauses from other solvers
            importClauses();
            importProvenLiterals();
#endif /* SATURNIN_PARALLEL */
            state = simplify();
            ASSERT_EQUAL(0U, propagationQueue.getSize());
        }
    }
#ifdef SATURNIN_PARALLEL
    if(state != wUnknown && parallelData != nullptr){
        unsigned int noSolver = parallelData->nbThreads +1;
        bool first = parallelData->winningSolver.compare_exchange_strong(noSolver, threadId);
        if(first){
            parallelData->solutionFound = true;
            //stop the other solvers
            for(unsigned int i = 0; i<parallelData->nbThreads; i++){
                parallelData->solvers[i]->asyncStop = true;
            }
        }
    }
#endif /* SATURNIN_PARALLEL */

    return state != wUnknown;
}

wbool Solver::search(int) {
    int currentNbConflict = 0;
    bool restart = false;
    wbool answer = state;
    SlidingMean lbdSlide(localLbdAverageLength);
    while (!restart && answer == wUnknown && !asyncStop) {

        Lit* conflictingClause = propagate();
        if (conflictingClause == nullptr) {
            if (nbConfBeforeReduce == 0 && assignLevel == 0) {
                reduce();
                reduceLimit += reduceIncrement;
                nbConfBeforeReduce = reduceLimit;
            }

            //start a new assignation level
            assignLevel++;
            stackPointer.push(stack.getSize());
            ASSERT_EQUAL(assignLevel + 1, stackPointer.getSize());

            //look for next variable
            if (varActivity.getSize() > 0) {
                Var v = varActivity.removeMin();
                while (assign[v] != wUnknown && varActivity.getSize() > 0) {
                    v = varActivity.removeMin();
                }
                if (varActivity.getSize() == 0 && assign[v] != wUnknown) {
                    //we couldn't find a variable not assigned
                    //we did it, a solution was found!
                    answer = wTrue;
                } else {
                    Lit l = 0;
                    if(presumption[v] == wUnknown){
                        l = VariablesManager::getLit(v, phase[v] == wTrue);
                    }else{
                        l = VariablesManager::getLit(v, presumption[v] == wTrue);
                    }

                    enqueue(l);
                }
            } else {
                //we did it, a solution was found!
                answer = wTrue;
            }
        } else {
            currentNbConflict++;
            conflicts++;

            //see Gilles Audemard, Laurent Simon
            //"Refining Restarts Strategies For SAT and UNSAT"
            //in 18th International Conference on Principles and Practice of Constraint Programming (CP'12)
            //Springer, pp. 118-126, october 2012.
            stackSize.addValue(stack.getSize());
            if (conflicts > 10000 && lbdSlide.isFull() && stack.getSize() > stackDifferenceFactor * stackSize.getMean()) {
                lbdSlide.reset();
            }

            //if we already are on level 0, it means we have proven UNSAT
            if (assignLevel == 0) {
                answer = wFalse;
            } else {
                Clause* learnt = treatConflict(conflictingClause);
                if(state == wFalse) answer = wFalse;
                lbdSlide.addValue(learnt != nullptr ? learnt->getLBD() : 1);
            }
            
            //Check if a restart is needed
            restart = lbdSlide.isFull() &&
                    lbdSlide.getMean() * lbdDifferenceFactor > lbdMean.getMean();

            //Check if a reduce needs to be performed
            if (nbConfBeforeReduce > 0) {
                nbConfBeforeReduce--;
            }
        }
    }

    if (verbosity > 0) {
        double m = 0;
        if (lbdSlide.getNbValues() > 0) {
            m = lbdSlide.getMean();
        }
        fprintf(Logger::getStdOutLogger().getOutput(),
#ifdef SATURNIN_PARALLEL
                "thread: %4d, nbConflicts: %8ld, prop: %8ld, avg lbd: %5.1f\n",
                threadId, conflicts, nbPropag, m);
#else
                "nbConflicts: %8ld, prop: %8ld, avg lbd: %5.1f\n",
                conflicts, nbPropag, m);
#endif /* SATURNIN_PARALLEL */ 
    }
    return answer;
}

void Solver::enqueue(Lit l, Lit* r) {

    Var v = VariablesManager::getVar(l);

    ASSERT(wUnknown == assign[v]);
    ASSERT(nullptr == reasons[v]);
    wbool sign = VariablesManager::getLitSign(l) ? wTrue : wFalse;
    assign[v] = sign;

#ifdef DEBUG
    //make sure that every literal but the first is assigned to false
    for (Lit* i = r + 1; r != nullptr && *i != lit_Undef; i++) {
        Var vTmp = VariablesManager::getVar(*i);
        ASSERT(assign[vTmp] != wUnknown);
        ASSERT(assign[vTmp] == (VariablesManager::getLitSign(*i) ? wFalse : wTrue));
    }
#endif /* DEBUG */

    if (lastReduceSeen[v] != nbReducePerformed) {
        nbVarSeenBtwReduce++;
        lastReduceSeen[v] = nbReducePerformed;
    }

    phase[v] = sign;
    varLevel[v] = assignLevel;
    reasons[v] = r;

    propagationQueue.push(VariablesManager::oppositeLit(l));
    stack.push(l);

}

bool Solver::backtrack(unsigned int nbLvl) {
    //if we backtrack of 0 level, we stay at the same height and will loop
    ASSERT(nbLvl != 0);
    nbLvl -= 1;
    if (assignLevel < nbLvl) {
        //bingo, inconsistent at level 0 -> UNSAT
        return false;
    }
    stackPointer.pop(nbLvl);
    ASSERT(stackPointer.getSize() > 0);
    unsigned int goal = stackPointer.getLast();
    ASSERT(stack.getSize() > goal);
    //revert the decisions/propagations that were made
    while (stack.getSize() != goal) {
        Lit l = stack.getLast();
        Var v = VariablesManager::getVar(l);
        stack.pop();
        assign[v] = wUnknown;
        if (!varActivity.contains(v)) {
            varActivity.add(v);
        }
        reasons[v] = nullptr;

    }
    assignLevel -= nbLvl + 1;

    //Now that we have successfully remove every thing we made up to level + 1
    //remove the last element of stack pointer to be as we were when we left
    //the level we want to reach
    stackPointer.pop();

    //The only assignation level that could be empty is level 0 if no
    //literal has been proven
    ASSERT(stackPointer.getLast() < stack.getSize() ||
            (stackPointer.getLast() == 0 && stack.getSize() == 0));
    ASSERT_EQUAL(assignLevel + 1, stackPointer.getSize());

    return true;
}

void Solver::printClause(Clause* c, bool color, FILE * out) const {
    printClause(c->lits(), color, out);
}

void Solver::printClause(Lit* c, bool color, FILE* out) const {
    for (unsigned int i = 0; c[i] != lit_Undef; i++) {
        printLit(c[i], color, out);
        if (c[i + 1] != lit_Undef) {
            fprintf(out, " ");
        }
    }
}

void Solver::printClauses(bool color, const char* const sep, FILE * out) const {
    for (unsigned int i = 0; i < clauses.getSize(); i++) {
        printClause(clauses[i], color, out);
        if (i + 1 < clauses.getSize()) {
            fprintf(out, "%s", sep);
        }
    }
}

bool Solver::validate() {
    if (state == wFalse) {
#ifdef SATURNIN_DB
        db.sanatize();
        return db.checkResolutions();
#else
        return true;
#endif /* SATURNIN_DB */
    }
    return checkClauseSetSatifiability(clauses, assign);
}

bool Solver::checkClauseSetSatifiability(const Array<Clause*>& set,
        const Array<wbool>& interpretation) const {
    for (unsigned int i = 0; i < set.getSize(); i++) {
        if (!set.get(i)->isSatisfied(interpretation)) {
            return false;
        }
    }
    return true;
}

Clause* Solver::treatConflict(Lit* conflictingClause){
    Array<Lit> res;
    unsigned int btlevel = analyze(conflictingClause, res);
    Clause* learnt = nullptr;
    if (!backtrack(btlevel)) {
        //we found a contradiction at level 0, therefore the problem is
        //UNSAT
        state = wFalse;
        return nullptr;
    } else {
        //a new clause was learnt
        learnt = addLearntClause(res);
#ifdef SATURNIN_PARALLEL
        exportClause(res, learnt == nullptr ? 1 : learnt->getLBD());
#endif /* SATURNIN_PARALLEL */
        if (learnt != nullptr) {
            unsigned int lbd = learnt->getLBD();
            lbdMean.addValue(lbd);
        } else if (res.getSize() == 2) {
            lbdMean.addValue(1);
        }
    }
    vsidsInc *= (1 / vsidsDec);
    return learnt;
}

#ifdef SATURNIN_PARALLEL
void Solver::exportClause(Array<Lit>& c, unsigned int lbd){
    if(parallelData == nullptr) return;
    if(c.getSize() == 1){
        for(unsigned int i = 0; i<parallelData->solvers.getSize(); i++){
            if(i == threadId) continue;
            unsigned int nextPos = (parallelData->freeNonIncludedLiteral[i][threadId]);
            parallelData->exchangedLiterals[i][threadId][nextPos] = c[0U];
            parallelData->freeNonIncludedLiteral[i][threadId] = (nextPos+1)%parallelData->maxNbProvenLit;
        }
    }else{
        for(unsigned int i = 0; i<parallelData->solvers.getSize(); i++){
            if(i==threadId) continue;
            Clause* toExport = exchangedClauses.retrieveClause((Lit*)c, c.getSize(), lbd);
            unsigned int nextPos = (parallelData->freeNonIncludedClause[i][threadId]);
            parallelData->exchangedClauses[i][threadId][nextPos] = toExport;
            parallelData->freeNonIncludedClause[i][threadId] = (nextPos+1)%parallelData->maxNbExchanged;
        }
    }
}

bool Solver::importClause(Clause* c){
    ASSERT(parallelData != nullptr);
    ASSERT_EQUAL(0U, assignLevel);
    
    unsigned int sz = c->getSize();
#ifdef DEBUG
    unsigned int initialSize = sz;
#endif /* DEBUG */
    Lit* literals = c->lits();
    //inside this loop, we do multiple things:
    //first, we remove the literals that were proven to be false
    //second, we count the number of potential watches
    //third, we look for the variable with the highest level
    for(unsigned int i = 0, j = 0; i<sz; i++){
        if(assign[VariablesManager::getVar(c->getLit(i))] != wUnknown && 
                varLevel[VariablesManager::getVar(c->getLit(i))] == 0){
            if(getLitValue(c->getLit(i)) == wTrue){
                return true;
            }
        }else {
            literals[j] = literals[i];
            j++;
        }
    }
    ASSERT_EQUAL(initialSize, c->getSize());
    
    if(sz == 0){
        return false;
    }else if (sz == 1){
        //propagate
        enqueue(c->getLit(0));
        Lit* conflict = propagate();
        if(conflict != nullptr) {
#ifdef SATURNIN_DB
            //compute the reason of the conflict and add the empty clause to
            //the database
            Array<Lit> learnt(2);
            computeLearntClause(conflict, learnt);
            db.addClause(&lit_Undef, db.nbClausesAdded());
#endif /* SATURNIN_DB */
            state = wFalse;
        }
    } else {
        if(c->getSize() > sz){
            c->setLit(sz, lit_Undef);
            ASSERT_EQUAL(initialSize, c->getSize());
        }
        if (sz == 2) {
            Lit a = c->getLit(0);
            Lit b = c->getLit(1);
            binWatched[a].push(b);
            binWatched[b].push(a);
            nbBin++;
        } else {
            Clause* newClause = allocator.createClause(c->lits(), sz);
            ASSERT(newClause->isAttached());
            unsigned int lbd = computeLBD(newClause);
            ASSERT(lbd != 0);
            while (lbd >= lbdValues.getSize()) {
                lbdValues.push(0);
            }
            lbdValues[lbd]++;
            newClause->setLBD(lbd);
            newClause->setIndex(learntClauses.getSize());
            learntClauses.push(newClause);
            addWatchedClause(newClause);
        }
    }
    
    ASSERT(c->getSize() <= initialSize);
    
    //make sure we can re-use the allocated memory if we can
    this->exchangedClauses.provideClause(c);
    
    return state != wFalse;
}

bool Solver::importClauses() {
    if (parallelData == nullptr) return true;

    bool isUnsat = false;
    for (unsigned int j = 0; j < parallelData->solvers.getSize() && !isUnsat; j++) {
        //no need to import our own things
        if (j == threadId) continue;

        unsigned int last = parallelData->freeNonIncludedClause[threadId][j];
        if (parallelData->firstNonIncludedClause[threadId][j] != last) {
            if (parallelData->firstNonIncludedClause[threadId][j] > last) {
                while (parallelData->firstNonIncludedClause[threadId][j] < parallelData->maxNbExchanged && !isUnsat) {
                    //import the clause
                    unsigned int pos = parallelData->firstNonIncludedClause[threadId][j];
                    Clause* c = parallelData->exchangedClauses[threadId][j][pos];
                    ASSERT(c != nullptr);
                    parallelData->firstNonIncludedClause[threadId][j]++;
                    isUnsat = !importClause(c);
                }
                parallelData->firstNonIncludedClause[threadId][j] = 0;
            }
            while (parallelData->firstNonIncludedClause[threadId][j] < last && !isUnsat) {
                //import the clause 
                unsigned int pos = parallelData->firstNonIncludedClause[threadId][j];
                Clause* c = parallelData->exchangedClauses[threadId][j][pos];
                ASSERT(c != nullptr);
                parallelData->firstNonIncludedClause[threadId][j]++;
                isUnsat = !importClause(c);
            }
        }
    }

    return !isUnsat;
}

bool Solver::importProvenLiterals() {
    if (parallelData == nullptr) return true;
    
    ASSERT_EQUAL(0U, assignLevel);

    bool isUnsat = false;
    for (unsigned int j = 0; j < parallelData->solvers.getSize() && !isUnsat; j++) {
        //no need to import our own things
        if (j == threadId) continue;

        unsigned int last = parallelData->freeNonIncludedLiteral[threadId][j];
        if (parallelData->firstNonIncludedLiteral[threadId][j] != last) {
            if (parallelData->firstNonIncludedLiteral[threadId][j] > last) {
                while (parallelData->firstNonIncludedLiteral[threadId][j] < parallelData->maxNbProvenLit && !isUnsat) {
                    //import the literal
                    unsigned int pos = parallelData->firstNonIncludedLiteral[threadId][j];
                    Lit l = parallelData->exchangedLiterals[threadId][j][pos];
                    ASSERT_EQUAL(0U, assignLevel);
                    ASSERT(l != lit_Undef);
                    parallelData->firstNonIncludedLiteral[threadId][j]++;
                    isUnsat = !addClause(&l, 1, true);
                }
                parallelData->firstNonIncludedLiteral[threadId][j] = 0;
            }
            while (parallelData->firstNonIncludedLiteral[threadId][j] < last && !isUnsat) {
                //import the literal
                unsigned int pos = parallelData->firstNonIncludedLiteral[threadId][j];
                Lit l = parallelData->exchangedLiterals[threadId][j][pos];
                ASSERT_EQUAL(0U, assignLevel);
                ASSERT(l != lit_Undef);
                parallelData->firstNonIncludedLiteral[threadId][j]++;
                isUnsat = !addClause(&l, 1, true);
            }
        }
    }
    return !isUnsat;
}
#endif /* SATURNIN_PARALLEL */

size_t Solver::getMemoryFootprint() const {
    size_t mem = varLevel.getMemoryFootprint() + assign.getMemoryFootprint();
    mem += phase.getMemoryFootprint() + previousPhase.getMemoryFootprint();
    mem += presumption.getMemoryFootprint();
    mem += reasons.getMemoryFootprint() + binReasons.getMemoryFootprint();
    mem += binWatched.getMemoryFootprint();
#ifdef SATURNIN_DB
    mem += db.getMemoryFootprint();
#endif /* SATURNIN_DB */
    for (unsigned int i = 0; i < binWatched.getSize(); i++) {
        mem += binWatched.get(i).getMemoryFootprint();
    }
    for (unsigned int i = 0; i < watchedClauses.getSize(); i++) {
        mem += watchedClauses.get(i).getMemoryFootprint();
    }
    mem += monowatch.getMemoryFootprint();
    for (unsigned int i = 0; i < watchedClauses.getSize(); i++) {
        mem += monowatch.get(i).getMemoryFootprint();
    }
    mem += watchedClauses.getMemoryFootprint() + varActivityValue.getMemoryFootprint();
    mem += clauses.getMemoryFootprint() + learntClauses.getMemoryFootprint();
    mem += propagationQueue.getMemoryFootprint() + stack.getMemoryFootprint();
    mem += stackPointer.getMemoryFootprint() + varActivity.getMemoryFootprint();
    mem += allocator.getMemoryFootprint();
    mem += levelLBDChecked.getMemoryFootprint() + seen.getMemoryFootprint();
    mem += pureLitSearch.getMemoryFootprint();
    mem += lbdValues.getMemoryFootprint() + map.getMemoryFootprint() + revertMap.getMemoryFootprint();
    return mem;
}

