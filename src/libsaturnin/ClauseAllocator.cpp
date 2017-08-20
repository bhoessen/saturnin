#include "saturnin/ClauseAllocator.h"

#include "saturnin/Clause.h"

namespace {

    struct BucketizedInit {
        static const int MaxSize = 1;
        static const int BucketSize = 1;
        static const int Shift = 0;
        static const int MaxIndex = 0;
        static int getIndex(unsigned int ) { return 0; }
        static int getMaxSizeInBucket(unsigned int) { return 0; }
    };

    template<unsigned int MaxSz, unsigned int BucketSz, typename Recursive = BucketizedInit>
    class Bucketize {
    public:
        static const int MaxSize = MaxSz;
        static const int BucketSize = BucketSz;
        static const int Shift = (Recursive::MaxSize - 1) / Recursive::BucketSize - Recursive::MaxSize / BucketSize + Recursive::Shift;
        static const int MaxIndex = Shift - 1 + (MaxSize - 1) / BucketSize;

        static int getIndex(unsigned int sz) {
            if (sz<Recursive::MaxSize)
                return Recursive::getIndex(sz);
            return (sz - 1) / BucketSize + Shift - 1;
        }

        static int getMaxSizeInBucket(unsigned int bucketIndex) {
            if (bucketIndex < Recursive::MaxIndex)
                return Recursive::getMaxSizeInBucket(bucketIndex);
            return (bucketIndex + 2 - Shift)*BucketSize;
        }
    };

    using D = Bucketize<17, 1>;
    using V = Bucketize<33, 4, D>;
    using N = Bucketize<129, 8, V>;
    using O = Bucketize<4097, 32, N>;


    inline unsigned int getIndex(unsigned int size) {
        if (size == 2) 
            return 0;
        int index = O::getIndex(size);
        ASSERT(index >= 0);
        return static_cast<unsigned int>(index);
    }

    inline unsigned int getMaxSizeForIndex(unsigned int index) {
        int maxSize = O::getMaxSizeInBucket(index);
        ASSERT(maxSize >= 0);
        return static_cast<unsigned int>(maxSize);
    }

}

using namespace saturnin;

ClauseAllocator::ClauseAllocator(unsigned int maxSize, unsigned int maxLenght) : pools(getIndex(maxSize)), initialPoolSize(maxLenght)
#ifdef PROFILE
, clauseRepartition(36)
#endif /* PROFILE */
{
    unsigned int sz = getIndex(maxSize)+1;
    for (unsigned int i = 0; i < sz; i++) {
        pools.push(new PoolList(getMaxSizeForIndex(i), initialPoolSize));
    }
}

ClauseAllocator::~ClauseAllocator() {
    unsigned int sz = pools.getSize();
    for (unsigned int i = 0; i < sz; i++) {
        delete(pools[i]);
#ifdef DEBUG
        pools[i] = nullptr;
#endif
    }
}

Clause* ClauseAllocator::createClause(const Lit * const lits, unsigned int sz, unsigned int lbd) {
    ASSERT(lits != nullptr);
    ASSERT(sz > 1);
#ifdef PROFILE
    while (clauseRepartition.getSize() <= sz) {
        clauseRepartition.push(0U);
    }
    clauseRepartition[sz]++;
#endif /* PROFILE */
    unsigned int idx = getIndex(sz);
    if(idx>=pools.getSize()){
        for(unsigned int i=pools.getSize(); i<=idx; i++){
            pools.push(new PoolList(getMaxSizeForIndex(i), initialPoolSize));
        }
    }
    Clause* c = pools[idx]->createClause(lits, sz, lbd);
    ASSERT(c->getSize()==sz);
    return c;
}

Clause* ClauseAllocator::retrieveClause(const Lit * const lits, unsigned int sz, unsigned int lbd) {
    ASSERT(lits != nullptr);
    ASSERT(sz > 1);
    unsigned int idx = getIndex(sz);
    if (idx >= pools.getSize()) {
        for (unsigned int i = pools.getSize(); i <= idx; i++) {
            pools.push(new PoolList(getMaxSizeForIndex(i), initialPoolSize));
        }
    }
    Clause* c = pools[idx]->createClause(lits, sz, lbd);
    ASSERT(c->getSize() == sz);
    return c;
}

Clause* ClauseAllocator::createClause(const Array<Lit>& lits, unsigned int lbd) {
    return createClause((const Lit*) lits, lits.getSize(), lbd);
}

void ClauseAllocator::provideClause(Clause*& c) {
    ASSERT(c != nullptr);
    ASSERT(c->getSize() > 1);
    //This is added as we might be releasing a clause that was created by
    //another thread
    unsigned int pos = getIndex(c->getSize());
    if (pos >= pools.getSize()) {
        for (unsigned int i = pools.getSize(); i <= pos; i++) {
            pools.push(new PoolList(getMaxSizeForIndex(i), initialPoolSize));
        }
    }
    pools.get(pos)->releaseClause(c);
    c = nullptr;
}

void ClauseAllocator::releaseClause(Clause*& c) {
#ifdef PROFILE
    ASSERT(clauseRepartition[c->getSize()] > 0);
    clauseRepartition[c->getSize()]--;
#endif /* PROFILE */
    ASSERT(c != nullptr);
    ASSERT(c->getSize() > 1);
    //This is added as we might be releasing a clause that was created by
    //another thread
    unsigned int pos = getIndex(c->getSize());
    if(pos>=pools.getSize()){
        for(unsigned int i=pools.getSize(); i<=pos; i++){
            pools.push(new PoolList(getMaxSizeForIndex(i), initialPoolSize));
        }
    }
    pools.get(pos)->releaseClause(c);
    c = nullptr;
}

void ClauseAllocator::clearup(){
    while(pools.getSize()>0 && pools.getLast()->getSize() == 0){
        delete(pools.getLast());
        pools.pop();
    }
}

