#ifdef SATURNIN_PARALLEL

#include "saturnin/ParallelSolver.h"
#include "saturnin/Solver.h"
#include <thread>
#include <stdio.h>
#include <sstream>

using namespace saturnin;

ParallelSolver::ParallelSolver(unsigned int nbSolvers) :
nbThreads(nbSolvers), verbosity(0), solvers(nbSolvers), solutionFound(false), 
winningSolver(nbSolvers+1), exchangedClauses(nbSolvers), maxNbExchanged(128), 
firstNonIncludedClause(nbSolvers), freeNonIncludedClause(nbSolvers), 
exchangedLiterals(nbSolvers), maxNbProvenLit(128), 
firstNonIncludedLiteral(nbSolvers), freeNonIncludedLiteral(nbSolvers) {

    for (unsigned int i = 0; i < nbSolvers; i++) {
        solvers.push(nullptr);
        exchangedClauses.push(Array<Array<Clause*> >(nbSolvers));
        exchangedLiterals.push(Array<Array<Lit> >(nbSolvers));
        freeNonIncludedClause.push(Array<std::atomic<unsigned int> >(nbSolvers));
        freeNonIncludedLiteral.push(Array<std::atomic<unsigned int> >(nbSolvers));
        firstNonIncludedClause.push(Array<std::atomic<unsigned int> >(nbSolvers));
        firstNonIncludedLiteral.push(Array<std::atomic<unsigned int> >(nbSolvers));
        for (unsigned int j = 0; j < nbSolvers; j++) {
            exchangedClauses[i].push(Array<Clause*>(maxNbExchanged));
            exchangedLiterals[i].push(Array<Lit>(maxNbProvenLit));
            for (unsigned int k = 0; k < maxNbExchanged; k++) {
                exchangedClauses[i][j].push(nullptr);
            }
            for (unsigned int k = 0; k < maxNbProvenLit; k++) {
                exchangedLiterals[i][j].push(lit_Undef);
            }
            //two step initialisation as the copy constructor is not
            //defined for atomics
            freeNonIncludedClause[i].push();
            freeNonIncludedClause[i][j] = 0;
            freeNonIncludedLiteral[i].push();
            freeNonIncludedLiteral[i][j] = 0;
            firstNonIncludedClause[i].push();
            firstNonIncludedClause[i][j] = 0;
            firstNonIncludedLiteral[i].push();
            firstNonIncludedLiteral[i][j] = 0;
        }
    }

}

ParallelSolver::~ParallelSolver() {
    for (unsigned int i = 0; i < solvers.getSize(); i++) {
        if (solvers[i] != nullptr) {
            delete(solvers[i]);
            solvers[i] = nullptr;
        }
    }
}

void ParallelSolver::addPresumption(Var v, bool value) {
    for (unsigned int i = 0; i < solvers.getSize(); i++) {
        if(solvers[i] != nullptr) {
            solvers[i]->addPresumption(v, value);
        }
    }
}

Solver& ParallelSolver::getWinningSolver(){
    if(solutionFound){
        return *(solvers[winningSolver]);
    }else{
        if(solvers[0U] == nullptr){
            solvers[0U] = new Solver(16,16);
        }
        return *(solvers[0U]);
    }
}

size_t ParallelSolver::getMemoryFootprint() const {
    size_t sz = 0;
    sz += firstNonIncludedClause.getMemoryFootprint();
    sz += firstNonIncludedLiteral.getMemoryFootprint();
    sz += freeNonIncludedClause.getMemoryFootprint();
    sz += freeNonIncludedLiteral.getMemoryFootprint();
    sz += solvers.getMemoryFootprint();
    sz += exchangedClauses.getMemoryFootprint();
    sz += exchangedLiterals.getMemoryFootprint();
    for (unsigned int i = 0; i < solvers.getSize(); i++) {
        sz += firstNonIncludedClause[i].getMemoryFootprint();
        sz += firstNonIncludedLiteral[i].getMemoryFootprint();
        sz += freeNonIncludedClause[i].getMemoryFootprint();
        sz += freeNonIncludedLiteral[i].getMemoryFootprint();
        if (solvers[i] != nullptr) {
            sz += solvers[i]->getMemoryFootprint() + sizeof (Solver);
        }
        sz += exchangedClauses[i].getMemoryFootprint();
        sz += exchangedLiterals[i].getMemoryFootprint();
        for (unsigned int j = 0; j < solvers.getSize(); j++) {
            sz += exchangedClauses[i][j].getMemoryFootprint();
            sz += exchangedLiterals[i][j].getMemoryFootprint();
        }
    }
    return sz;
}

bool ParallelSolver::initialize(const CNFReader& r, const char* 
#ifdef SATURNIN_DB
dbFileName
#endif /* SATURNIN_DB */
) {
    Array<std::thread> threadPool(nbThreads);
#ifdef SATURNIN_DB
    bool inMemory = dbFileName != nullptr && strncmp(":memory:", dbFileName, 9) == 0;
#endif /* SATURNIN_DB */
    for (unsigned int i = 0; i < nbThreads; i++) {
        threadPool.push();
#ifdef SATURNIN_DB
        std::stringstream dbFileNameStrm;
        if(dbFileName != nullptr && !inMemory && nbThreads > 1){
            dbFileNameStrm << dbFileName << nbThreads;
        }
#endif /* SATURNIN_DB */
        Solver* cur = new Solver(r.getNbVar(), r.getNbClauses()
#ifdef SATURNIN_DB
            , dbFileName == nullptr || inMemory || nbThreads == 1 ? 
                dbFileName : dbFileNameStrm.str().c_str()
#endif /* SATURNIN_DB */
                );
        solvers[i] = cur;
        cur->setParallelSolver(this, i);
        cur->setVerbosity(verbosity);
    }

    for (unsigned int i = 0; i < nbThreads; i++) {
        Solver* cur = solvers[i];
        threadPool[i] = std::thread([cur, &r]() {
            for (unsigned int j = 0; j < r.getNbClauses() && cur->getState() == wUnknown; j++) {
                cur->addClause(r.getClause(j));
            }
        });
    }
    for (unsigned int i = 0; i < nbThreads; i++) {
        threadPool[i].join();
    }
    bool stateFound = solutionFound ? solvers[winningSolver]->getState() != wUnknown : false;
    for(unsigned int i = 0; i<nbThreads && !stateFound; i++){
        if(solvers[i]->getState() != wUnknown){
            solutionFound = true;
            winningSolver = i;
            stateFound = true;
        }
    }
    return stateFound;
}

void ParallelSolver::stop() {
    for(unsigned int i = 0; i<nbThreads; i++){
        if(solvers[i] != nullptr){
            solvers[i]->asyncStop = true;
        }
    }
}


bool ParallelSolver::solve(unsigned int maxNbRestarts) {
    Array<std::thread> threadPool(nbThreads);
    for (unsigned int i = 0; i < nbThreads; i++) {
        Solver* cur = solvers[i];
        threadPool.push();
        
        threadPool[i] = std::thread([cur, maxNbRestarts]() {
            cur->solve(maxNbRestarts);
        });
    }
    for (unsigned int i = 0; i < nbThreads; i++) {
        threadPool[i].join();
    }
    return solutionFound;
}

wbool ParallelSolver::getState() const {
    if (solutionFound) {
        return solvers[winningSolver]->getState();
    }
    return wUnknown;
}

#endif /* SATURNIN_PARALLEL */
