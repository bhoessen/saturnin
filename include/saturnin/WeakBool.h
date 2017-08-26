/*
Copyright (c) <2012> <B.Hoessen>

This file is part of saturnin.

saturnin is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

saturnin is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with saturnin.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SATURNIN_WBOOL_H
#define	SATURNIN_WBOOL_H

#include "Assert.h"
#include "Saturnin.h"

namespace saturnin {

  /** Definition for the wbool type */
  typedef char wbool;

#define wTrue ((saturnin::wbool)1)
#define wFalse ((saturnin::wbool)0)
#define wUnknown ((saturnin::wbool)2)

  /**
   * This class contains only static methods to help the usage of weak boolean
   * such as knowing if a wbool is assigned
   */
  class SATURNIN_EXPORT WeakBool final {
  public:

    /**
     * Check if a wbool is assigned or not
     * @param b the wbool to check
     * @return true if b is assigned
     */
    static inline bool isAssigned(wbool b) {
      ASSERT(b == wTrue || b == wFalse || b == wUnknown);
      return b != wUnknown;
    }

    /**
     * Retrieve the boolean value of a wbool
     * @param b th wbool to check
     * @return the truth value of the wbool
     */
    static inline bool getBooleanValue(wbool b) {
      ASSERT(b == wTrue || b == wFalse);
      return b == wTrue;
    }

  };

}

#endif	/* SATURNIN_WBOOL_H */

