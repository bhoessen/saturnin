#include "saturnin/Assert.h"

uint64_t _saturnin_nb_assert_ = 0;

#ifdef DEBUG
void saturnin::_saturnin_incNbAssert()
{
    _saturnin_nb_assert_++;
}

uint64_t saturnin::_saturnin_nbAssert()
{
    return _saturnin_nb_assert_;
}
#endif /* DEBUG */