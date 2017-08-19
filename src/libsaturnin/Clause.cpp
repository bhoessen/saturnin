#include "saturnin/Clause.h"
#include <stdlib.h>
#include <stdint.h>

using namespace saturnin;

Clause::Clause(const Lit* const literals, unsigned int sz, unsigned int l) :
        size(sz), compact(), index(0), data(){
    compact.lbd = l;
    compact.toRemove = false;
    compact.attached = true;
    compact.freezeCounter = 0;
    for(unsigned int i = 0; i<sz; i++){
        data[i] = literals[i];
    }
    data[sz] = lit_Undef;
}

bool Clause::operator==(const Clause& other) const{
    if(size != other.size){
        return false;
    }
    for(unsigned int i=0; i<size; i++){
        bool found = false;
        for(unsigned int j=0; !found && j<size; j++){
            found = data[i] == other.data[j];
        }
        if(!found){
            return false;
        }
    }
    return true;
}

Clause::~Clause(){
#ifdef DEBUG
    for(unsigned int i = 0; i<size; i++){
        data[i] = 0;
    }
    size = 0;
    compact.lbd = 0;
    index = 0;
    compact.attached = true;
    compact.freezeCounter = 0;
#endif
}

