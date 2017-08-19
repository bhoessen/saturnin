#include "saturnin/Simplifier.h"
#include "saturnin/Assert.h"
#include "saturnin/Clause.h"

using namespace saturnin;

Simplifier::Simplifier(unsigned int nbVar, unsigned int nbClauses) : s(nbVar, nbClauses), nbClausesReduced(0U) {

}

Simplifier::~Simplifier() {
}

void Simplifier::revival(unsigned int min) {
    for (unsigned int i = 0; s.state == wUnknown && i < s.clauses.getSize(); i++) {
        Clause* c = s.clauses.get(i);
        s.stopWatchClause(c->getLit(0), c);
        s.stopWatchClause(c->getLit(1), c);
        if (c->getSize() >= min) {
            bool stop = false;
            bool removeClause = false;
            Array<Lit> simplified;
            for (unsigned int j = 0; !stop && j < c->getSize(); j++) {
                Lit l = c->getLit(j);
                Var v = VariablesManager::getVar(l);
                if (s.assign[v] != wUnknown) {
                    if (s.varLevel[v] == 0) {
                        if (s.assign[v] == (VariablesManager::getLitSign(l) ?
                                wTrue : wFalse)) {
                            //c can be removed
                            removeClause = true;
                            stop = true;
                        } else {
                            //l can be removed from c as it has been proven false
                        }
                    } else {
                        if (s.assign[v] == (VariablesManager::getLitSign(l) ?
                                wTrue : wFalse)) {
                            //let -l_1, -l_2, ..., -l_a the literals from c that
                            //lead to the assignation of l, we can replace c by
                            //l_1, l_2, ..., l_a, l
                            simplified.push(l);
                            stop = true;
                        } else {
                            //we may safely remove l from c
                        }
                    }
                } else {
                    s.assignLevel++;
                    s.stackPointer.push(s.stack.getSize());
                    s.enqueue(VariablesManager::oppositeLit(l));
                    Lit* conflict = s.propagate();
                    if (conflict != nullptr) {
                        //simplified subsumes c
                        stop = true;
                        //generate the learnt clause. If it is smaller than
                        //simplified, we can use the learnt instead of simplified
                        Array<Lit> learnt(c->getSize());
                        s.analyze(conflict, learnt);
                        if (learnt.getSize() < simplified.getSize()) {
                            simplified.pop(simplified.getSize());
                            simplified = learnt;
                        }
                    } else {
                        simplified.push(l);
                    }
                }
            }
            if (s.assignLevel > 0) {
                s.backtrack(s.assignLevel);
            }
            ASSERT_EQUAL(0U, s.assignLevel);
            if (!removeClause && simplified.getSize() < c->getSize()) {
                nbClausesReduced++;
                unsigned int clSz = s.clauses.getSize();
                bool added = simplified.getSize() > 0 &&
                        s.addClause((Lit*) simplified,
                        simplified.getSize(), true);
                if (!added) {
                    s.state = wFalse;
                }
                s.clauses[i] = s.clauses.getLast();
                if (s.clauses.getSize() == clSz) {
                    i--;
                }
                s.clauses.pop();
                s.allocator.releaseClause(c);
            } else if (removeClause) {
                s.clauses[i] = s.clauses.getLast();
                s.clauses.pop();
                i--;
                s.allocator.releaseClause(c);
            } else {
                s.addWatchedClause(c);
            }
        }
    }
}
