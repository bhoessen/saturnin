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


#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <stdlib.h>
#include <stdio.h>
#include <cinttypes>
#include "../../include/saturnin/Assert.h"

int main(int , char** ){

    CppUnit::TextUi::TestRunner runner;
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest( registry.makeTest() );
    printf("%d-bit application\n", sizeof(int) == sizeof(std::intptr_t) ? 32 : 64);
    printf("Database module: %s\n",
#ifdef SATURNIN_DB
    "compiled"  
#else
    "not compiled"
#endif
    );
    printf("Parallel module: %s\n",
#ifdef SATURNIN_PARALLEL
        "compiled"
#else
        "not compiled"
#endif
    );
    printf("Profile information: %s\n",
#ifdef PROFILE
        "compiled"
#else
        "not compiled"
#endif
    );
    bool success = runner.run();
#ifdef DEBUG
    printf("Nb of assert performed: %" PRIu64 "\n",NB_ASSERTED);
#endif /* DEBUG */
    if(success){
        return EXIT_SUCCESS;
    }else{
        return EXIT_FAILURE;
    }
}
