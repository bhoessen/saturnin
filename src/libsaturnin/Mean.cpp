#include "saturnin/Mean.h"

using namespace saturnin;

Mean::Mean() : sum(0), nbElements(0){    
}
        
Mean::Mean(const Mean& source) : sum(source.sum), nbElements(source.nbElements){    
}
        
Mean& Mean::operator=(const Mean& source){
    sum = source.sum;
    nbElements = source.nbElements;
    return *this;
}
