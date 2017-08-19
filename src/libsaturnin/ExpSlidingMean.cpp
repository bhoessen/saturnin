#include "saturnin/ExpSlidingMean.h"

using namespace saturnin;

ExpSlidingMean::ExpSlidingMean(unsigned int N, double initialValue) : mean(initialValue), alpha(2.0/(N+1)), beta(1-alpha){    
}

ExpSlidingMean::ExpSlidingMean(const ExpSlidingMean& s) : mean(s.mean), alpha(s.alpha), beta(s.beta){    
}

ExpSlidingMean& ExpSlidingMean::operator=(const ExpSlidingMean& source){
    mean = source.mean;
    alpha = source.alpha;
    beta = source.beta;
    return *this;
}

ExpSlidingMean::~ExpSlidingMean(){ }
