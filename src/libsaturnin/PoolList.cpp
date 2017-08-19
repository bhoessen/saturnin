#include "saturnin/PoolList.h"
#include <stdlib.h>
#include "saturnin/Assert.h"
#include "saturnin/Array.h"
#include "saturnin/Clause.h"

using namespace saturnin;

#define CLAUSE_SIZE(n) (sizeof(Clause)+((n)*sizeof(Lit)))

PoolList::PoolList(unsigned int cL, unsigned int nbClauses) : availableClauses(nbClauses), memory(nullptr), clauseLength(cL), cap(0), size(0) {
    //memory = (mchunks*) malloc(sizeof (mchunks));
    //memory->chunck = (char*) malloc(nbClauses * CLAUSE_SIZE(clauseLength));
    //ASSERT(memory->chunck != nullptr);
#ifdef DEBUG
    //memset(memory->chunck, 0, nbClauses * CLAUSE_SIZE(clauseLength));
#endif /* DEBUG */
    //memory->chunkSize = nbClauses;
    //memory->next = nullptr;
    //char* tmp = memory->chunck;
    //for (unsigned int i = 0; i < nbClauses; i++) {
    //    availableClauses.push((Clause*) tmp);
    //    tmp += CLAUSE_SIZE(clauseLength);
    //}
}

PoolList::~PoolList() {
    if (memory != nullptr) {
        while (memory->next != nullptr) {
            mchunks* tmp = memory;
            memory = memory->next;
            free(tmp->chunck);
            free(tmp);
        }
        free(memory->chunck);
        free(memory);
        memory = nullptr;
    }
}

size_t PoolList::getMemoryFootprint() const {
    mchunks* tmp = memory;
    size_t mem = availableClauses.getMemoryFootprint();
    while (tmp != nullptr) {
        mem += sizeof (mchunks) + tmp->chunkSize * CLAUSE_SIZE(clauseLength);
        tmp = tmp->next;
    }
    return mem;
}

Clause* PoolList::createClause(const Lit * const literals, unsigned int sz, unsigned int lbd) {
    ASSERT(sz <= clauseLength);
    if (availableClauses.getSize() == 0) {
        mchunks* tmp = (mchunks*) malloc(sizeof (mchunks));
        ASSERT(tmp != nullptr);
        if (memory) {
            tmp->chunkSize = (unsigned int)(SATURNIN_POOLLIST_UPDATE_FACTOR * memory->chunkSize);
        } else {
            tmp->chunkSize = availableClauses.getCapacity();
        }
        tmp->chunck = (char*) malloc(tmp->chunkSize * CLAUSE_SIZE(clauseLength));
        ASSERT(tmp->chunck != nullptr);
        tmp->next = memory;
        memory = tmp;
        char* _tmp = memory->chunck;
        for (unsigned int i = 0; i < memory->chunkSize; i++) {
            availableClauses.push((Clause*) _tmp);
            _tmp += CLAUSE_SIZE(clauseLength);
        }
        cap += memory->chunkSize;
    }
    size++;
    Clause* memLocation = availableClauses.get(availableClauses.getSize() - 1);
    availableClauses.pop();
    return new (memLocation) Clause(literals, sz, lbd);
}

void PoolList::releaseClause(Clause*& c) {
    ASSERT(c != nullptr);
    size--;
    c->~Clause();
    availableClauses.push(c);
    c = nullptr;
}
