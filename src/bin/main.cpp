#include "saturnin/Launcher.h"

using namespace saturnin;

int main(int argc, char** argv) {
#ifdef SATURNIN_DB
    printf("using database\n");
#endif
#ifdef DEBUG
    printf("debug mode\n");
#endif 
    Launcher saturnin(argc, argv);
    return saturnin.solve();
}
