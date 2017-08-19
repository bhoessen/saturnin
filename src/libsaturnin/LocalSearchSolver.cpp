#include "saturnin/LocalSearchSolver.h"

#include "saturnin/Clause.h"

using namespace saturnin;

LocalSearchSolver::LocalSearchSolver(unsigned int aNbVar, unsigned int nbClauses) :
state(wTrue), nbVar(aNbVar), nbSatClauses(0), nbFlip(0),
nbBecomeUnsat(nbVar * 2), nbBecomeSat(nbVar * 2), lastNbFlip(nbVar), tabooLenght(10U),
score(0U), assignation(nbVar), unit(nbVar),
occList(nbVar * 2), allocator(10, 16), clauses(nbClauses) {
    ASSERT(nbVar > 0);
    ensureCapacity(nbVar - 1, 0);
}

LocalSearchSolver::~LocalSearchSolver() {

}

void LocalSearchSolver::ensureCapacity(Var v, unsigned int nbInitialized) {
    if (nbInitialized > v) {
        return;
    }

    for (unsigned int i = nbInitialized; i <= v; i++) {
        nbBecomeSat.push(0);
        nbBecomeSat.push(0);
        nbBecomeUnsat.push(0);
        nbBecomeUnsat.push(0);
        lastNbFlip.push(0);
        assignation.push(wTrue);
        unit.push(wUnknown);
        occList.push();
        occList.push();
    }
    nbVar = v + 1;

}

bool LocalSearchSolver::addClause(const Array<Lit>& clause) {
    return addClause((const Lit*) clause, clause.getSize());
}

bool LocalSearchSolver::addClause(const Lit* clause, unsigned int sz) {
    if (state == wFalse) {
        return false;
    }

    Array<Lit> lits(sz);
    for (unsigned int i = 0; i < sz; i++) {
        Lit l = clause[i];
        if (unit[VariablesManager::getVar(l)] != wUnknown) {
            if (unit[VariablesManager::getVar(l)] == (VariablesManager::getLitSign(l) ? wTrue : wFalse)) {
                return true;
            }
        } else {
            lits.push(l);
        }
    }

    if (lits.getSize() == 1) {
        Lit l = lits[0U];
        Var v = VariablesManager::getVar(l);
        bool sign = VariablesManager::getLitSign(l);
        if (unit[v] != wUnknown && unit[v] != (sign ? wTrue : wFalse)) {
            state = wFalse;
            return false;
        } else {
            unit[v] = sign ? wTrue : wFalse;
            if (unit[v] != assignation[v]) {
                flipVariable(v);
            }
            ASSERT(assignation[v] == unit[v]);
            ASSERT_EQUAL(0U, occList[l].getSize());
            ASSERT_EQUAL(0U, occList[VariablesManager::oppositeLit(l)].getSize());
        }
        return true;
    }

    unsigned int nbTrue = 0;
    Clause* c = allocator.createClause(lits, 2);
    clauses.push(c);
    Lit aTrueLit = lit_Undef;
    for (unsigned int i = 0; i < c->getSize(); i++) {
        Lit l = c->getLit(i);
        ensureCapacity(VariablesManager::getVar(l), nbVar);
        ASSERT(unit[VariablesManager::getVar(l)] == wUnknown);
        occList[l].push(c);
        if ((VariablesManager::getLitSign(l) ? wTrue : wFalse) ==
                assignation[VariablesManager::getVar(l)]) {
            nbTrue++;
            aTrueLit = l;
        }
    }
    c->setIndex(nbTrue);
    if (nbTrue == 0) {
        state = wUnknown;
    } else {
        score++;
    }
    ASSERT(checkTrueLitCounter(c));
    if (nbTrue == 0) {
        //if we switch any of the literals, the clause will become sat
        for (unsigned int i = 0; i < lits.getSize(); i++) {
            nbBecomeSat[VariablesManager::oppositeLit(lits[i])]++;
        }
    } else {
        nbSatClauses++;
        if (nbTrue == 1) {
            //if we switch the only true literal, the clause become unsat
            nbBecomeUnsat[VariablesManager::oppositeLit(aTrueLit)]++;
        }
    }

    return true;

}

bool LocalSearchSolver::checkTrueLitCounter(const Clause* c) const {
    unsigned int nbTrueLits = 0;
    for (unsigned int i = 0; i < c->getSize(); i++) {
        if (VariablesManager::isLitSatisfied(c->getLit(i),
                assignation.get(VariablesManager::getVar(c->getLit(i))))) {
            nbTrueLits++;
        }
    }
    ASSERT_EQUAL(nbTrueLits, c->getIndex());
    return c->getIndex() == nbTrueLits;
}

bool LocalSearchSolver::isModel() const {
    for (unsigned int i = 0; i < clauses.getSize(); i++) {
        ASSERT(checkTrueLitCounter(clauses.get(i)));
        if (clauses.get(i)->getIndex() == 0) {
            return false;
        }
    }
    return true;
}

bool LocalSearchSolver::solve(unsigned int maxNbFlip) {
    if (state != wUnknown) {
        return true;
    }
#ifdef DEBUG
    {
        unsigned int localScore = 0;
        for (unsigned int i = 0; i < clauses.getSize(); i++) {
            ASSERT(checkTrueLitCounter(clauses.get(i)));
            if (clauses.get(i)->getIndex() > 0) {
                localScore++;
            }
        }
        ASSERT_EQUAL(localScore, score);
    }
#endif /* DEBUG */
    bool solved = (state != wUnknown);
    nbFlip += tabooLenght;
    unsigned int localNbFlip = 0;
    while (!solved && localNbFlip < maxNbFlip) {
        Var v = getNextLitToFlip();

        flipVariable(v);
        nbFlip++;
        localNbFlip++;

        solved = score == clauses.getSize();
        ASSERT_EQUAL(isModel(), solved);
#ifdef DEBUG
        unsigned int localScore = 0;
        for (unsigned int i = 0; i < clauses.getSize(); i++) {
            if (clauses.get(i)->getIndex() > 0) {
                localScore++;
            }
        }
        ASSERT_EQUAL(localScore, score);
#endif /* DEBUG */
    }
    nbFlip -= tabooLenght;
    if (solved) {
        state = wTrue;
    }
    return solved;
}

void LocalSearchSolver::flipVariable(Var v) {
    Lit l = VariablesManager::getLit(v, assignation[v] != wTrue);
    Lit nl = VariablesManager::oppositeLit(l);

    lastNbFlip[v] = nbFlip;

    Array<Clause*>& occNl = occList[nl];

    assignation[v] = assignation[v] == wTrue ? wFalse : wTrue;

    for (unsigned int i = 0; i < occNl.getSize(); i++) {
        Clause* c = occNl[i];
        //the clause c should have at least 1 true lit
        ASSERT(c->getIndex() > 0);
        ASSERT(c->getIndex() <= c->getSize());
        c->setIndex(c->getIndex() - 1);
        if (c->getIndex() == 0) {
            ASSERT(score > 0);
            score--;
            ASSERT(nbBecomeUnsat[l] > 0);
            nbBecomeUnsat[l]--;
            for (unsigned int j = 0; j < c->getSize(); j++) {
                Lit cur = c->getLit(j);
                ASSERT(!VariablesManager::isLitSatisfied(cur,
                        assignation[VariablesManager::getVar(cur)]));
                nbBecomeSat[VariablesManager::oppositeLit(cur)]++;
            }
        } else if (c->getIndex() == 1) {
            //we must increase the nbBecomeUnsat counter for the last true lit
            //remaining
            bool found = false;
            for (unsigned int j = 0; !found && j < c->getSize(); j++) {
                Lit cur = c->getLit(j);
                if (VariablesManager::isLitSatisfied(cur,
                        assignation[VariablesManager::getVar(cur)])) {
                    found = true;
                    nbBecomeUnsat[VariablesManager::oppositeLit(cur)]++;
                }
            }
        }

#ifdef DEBUG
        checkTrueLitCounter(c);
#endif /* DEBUG */
    }

    Array<Clause*>& occL = occList[l];
    for (unsigned int i = 0; i < occL.getSize(); i++) {
        Clause* c = occL[i];
        //there must be at least one false literal
        ASSERT(c->getIndex() < c->getSize());
        if (c->getIndex() == 0) {
            nbBecomeUnsat[nl]++;
            ASSERT(score < clauses.getSize());
            score++;
            for (unsigned int j = 0; j < c->getSize(); j++) {
                Lit cur = c->getLit(j);
                ASSERT(nbBecomeSat[VariablesManager::oppositeLit(cur)] > 0);
                nbBecomeSat[VariablesManager::oppositeLit(cur)]--;
            }
        } else if (c->getIndex() == 1) {
            //we must decrease the nbBecomeUnsat counter of the second true lit
            bool found = false;
            for (unsigned int j = 0; !found && j < c->getSize(); j++) {
                Lit cur = c->getLit(j);
                if (cur != l && VariablesManager::isLitSatisfied(cur,
                        assignation[VariablesManager::getVar(cur)])) {
                    found = true;
                    nbBecomeUnsat[VariablesManager::oppositeLit(cur)]--;
                }
            }
        }
        c->setIndex(c->getIndex() + 1);
#ifdef DEBUG
        checkTrueLitCounter(c);
#endif /* DEBUG */
    }

}

long LocalSearchSolver::getFlippingScore(Var v) {
    Lit dest = VariablesManager::getLit(v, assignation[v] != wTrue);
    Lit nDest = VariablesManager::oppositeLit(dest);
    long result = 1024 * (nbBecomeSat[dest] + nbBecomeSat[nDest]);
    result -= (nbBecomeUnsat[dest] + nbBecomeUnsat[nDest]);
    return result;
}

Var LocalSearchSolver::getNextLitToFlip() {
    Var best = 0;
    while (best < nbVar && lastNbFlip[best] + tabooLenght >= nbFlip) {
        best++;
    }
    if (best == nbVar) {
        return 0;
    }
    long bestScore = getFlippingScore(best);
    for (Var tmp = best + 1; tmp < nbVar; tmp++) {
        long tmpScore = getFlippingScore(tmp);
        if (bestScore < tmpScore && lastNbFlip[tmp] + tabooLenght < nbFlip) {
            best = tmp;
            bestScore = tmpScore;
        }
    }
    return best;
}

