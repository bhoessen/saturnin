#include "saturnin/Launcher.h"

using namespace saturnin;

int main(int argc, char** argv) {
#ifdef SATURNIN_DB
    printf("c using database\n");
#endif
#ifdef DEBUG
    printf("c debug mode\n");
#endif 
    Launcher saturnin(argc, argv);
    return saturnin.solve();
}
