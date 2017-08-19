#pragma once
#ifndef SATURNIN_GSATURNIN_H
#define SATURNIN_GSATURNIN_H

#include "saturnin/Launcher.h"

#include "./ValueEvolution.h"

namespace gsaturnin {

    class GUISaturnin {
    public:

        GUISaturnin(int argc, char** argv);

        void start();

        void stop();

        void draw();

        static const int HistoryLenght = 8192/32;

    private:

        void updateData();
        
        saturnin::Launcher launcher;
        ValueEvolution<HistoryLenght, true> propagations;
        ValueEvolution<HistoryLenght> nbClauses;
        ValueEvolution<HistoryLenght, true> conflicts;
        ValueEvolution<HistoryLenght> assignLevel;
        ValueEvolution<HistoryLenght> memory;
#ifdef PROFILE
        ValueEvolution<HistoryLenght> profile_propagateMono;
        ValueEvolution<HistoryLenght> profile_propagateBin;
        ValueEvolution<HistoryLenght> profile_propagateWatched;
        ValueEvolution<HistoryLenght> profile_reduce;
        ValueEvolution<HistoryLenght> profile_analyze;
        ValueEvolution<HistoryLenght> profile_simplify;
#endif /* PROFILE */
    };

}

#endif /* SATURNIN_GSATURNIN_H */
