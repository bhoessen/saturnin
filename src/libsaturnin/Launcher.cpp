#include "saturnin/Launcher.h"
#include <stdlib.h>
#include "saturnin/Clause.h"
#include "saturnin/Solver.h"
#include "saturnin/CNFReader.h"
#include "saturnin/StopWatch.h"
#include "../Utils.h"
#include "saturnin/Simplifier.h"
#include <signal.h>
#include <cinttypes>
#if defined (WIN32) || defined (_MSC_VER)
#define NOMINMAX
#include <windows.h>
#include <thread>
#else
#include <unistd.h>
#include <sys/resource.h>
#endif


#ifdef SATURNIN_PARALLEL
#include "saturnin/ParallelSolver.h"
#endif


#ifdef SATURNIN_PARALLEL
saturnin::ParallelSolver* solverToStop = nullptr;
#else 
saturnin::Solver* solverToStop = nullptr;
#endif /* SATURNIN_PARALLEL */

#ifdef PROFILE
uint64_t totalCycles = 0;
#endif /* PROFILE */

#if defined (WIN32) || defined (_MSC_VER)

BOOL WINAPI winSigStopLauncher(DWORD dwCtrlType) {
    switch (dwCtrlType) {
    case CTRL_C_EVENT: printf(" catched CTRL_C_EVENT signal\n");
        break;
    case CTRL_BREAK_EVENT: printf(" catched CTRL_BREAK_EVENT signal\n");
        break;
    case CTRL_CLOSE_EVENT: printf(" catched CTRL_CLOSE_EVENT signal\n");
        break;
    case CTRL_LOGOFF_EVENT: printf(" catched CTRL_LOGOFF_EVENT signal\n");
        break;
    case CTRL_SHUTDOWN_EVENT: printf(" catched CTRL_SHUTDOWN_EVENT signal\n");
        break;
    default: printf(" catched unknown signal\n");
        break;
    }
    if (solverToStop != nullptr) {
        solverToStop->stop();
    }
    return TRUE;
}
#else

/**
* This function is the signal handler that we use to stop the solver
* @param signum the value of the signal
*/
extern "C" void sigStopLauncher(int signum) {
    printf(" catched signal: %d\n", signum);
    if (solverToStop != nullptr) {
        solverToStop->stop();
    }
}

#endif

void saturnin::Launcher::printMemoryValue(FILE * fi, size_t mem) const {
    size_t tmp = mem;
    double f = 1;
    int i = 0;
    const char* prefix = " kmgt";
#ifndef PROFILE
    while (tmp > 1024 && i < 5) {
#else
    while (tmp > 1024 && i < 1) {
#endif
        f *= 1024;
        tmp /= 1024;
        i++;
    }
    fprintf(fi, "%14.3f %cb", mem / f, prefix[i]);
}

void saturnin::Launcher::simplify(saturnin::Solver& s, saturnin::CNFReader& reader) {
    Simplifier simp(reader.getNbVar(), reader.getNbClauses());
    for (unsigned int i = 0; i<reader.getNbClauses(); i++) {
        simp.addClause((const Lit*)reader.getClause(i), reader.getClause(i).getSize());
    }
    simp.revival(reader.getAverageLength() + 1);
    //add the proven literals
    const saturnin::Array<saturnin::Lit>& provenLits = simp.getProvenLit();
    for (unsigned int i = 0; i < provenLits.getSize(); i++) {
        saturnin::Lit l = provenLits.get(i);
        ASSERT(l != saturnin::lit_Undef);
        saturnin::Array<saturnin::Lit> cl(1U);
        cl.push(l);
        s.addClause(cl);
    }
    //Add the binary clauses
    for (unsigned int l = 0; l < reader.getNbVar() * 2; l++) {
        //either the clause is SAT and the clause doesn't need to be added
        //either the clause should be unary and has been already propagated
        if (s.getVarValue(saturnin::VariablesManager::getVar(l)) != wUnknown) continue;
        const saturnin::Array<saturnin::Lit>& binClauses = simp.getBinaryWith(l);
        for (unsigned int j = 0; j < binClauses.getSize(); j++) {
            saturnin::Lit tmp = binClauses.get(j);
            if (tmp < l && s.getVarValue(saturnin::VariablesManager::getVar(tmp)) == wUnknown) {
                saturnin::Array<saturnin::Lit> cl(2U);
                cl.push(l);
                cl.push(tmp);
                s.addClause(cl);
            }
        }
    }

    //add the rest of the clauses
    const saturnin::Array<saturnin::Clause*>& clauses = simp.getInitialClauses();
    for (unsigned int i = 0; i < clauses.getSize(); i++) {
        s.addClause(clauses.get(i)->lits(), clauses.get(i)->getSize());
    }
}

saturnin::Launcher::Launcher(int argc, char ** argv) : clean_exit(false), w(), readMem(0) {
    if (argc < 2) {
        printf("Error: need at least one argument\n");
        printHelp(argv[0]);
        return;
    }

    instanceName = argv[1];
    w.start();

#ifdef SATURNIN_PARALLEL
    unsigned int nbThreads = 1;
#else
    bool optSimplify = false;
#endif /* SATURNIN_PARALLEL */
    int timeLim = -1;
#ifdef SATURNIN_DB
    char* dbFileName = nullptr;
#endif /* SATURNIN_DB */
    for (int i = 2; i < argc; i++) {
        if (strncmp(argv[i], "-h", (size_t)2) == 0) {
            printHelp(argv[0]);
        }
        else if (strncmp(argv[i], "-clean-exit", (size_t)11) == 0) {
            clean_exit = true;
        }
#ifdef SATURNIN_PARALLEL
        else if (strncmp(argv[i], "-t=", (size_t)3) == 0) {
            char* out = nullptr;
            nbThreads = strtol(argv[i] + 3, &out, 10);
            if (out == (argv[i] + 3)) {
                nbThreads = 1;
            }
        }
#else
        else if (strncmp(argv[i], "-simplify", (size_t)9) == 0) {
            optSimplify = true;
        }
#endif /* SATURNIN_PARALLEL */
#ifdef SATURNIN_DB 
        else if (strncmp(argv[i], "-d=", (size_t)3) == 0) {
            dbFileName = argv[i] + 3;
        }
#endif /* SATURNIN_DB */
        else if (strncmp(argv[i], "-time-lim=", (size_t)10) == 0) {
            char* out = nullptr;
            timeLim = strtol(argv[i] + 10, &out, 10);
            if (out == (argv[i] + 10)) {
                timeLim = -1;
            }
        }
    }

    //don't use stack allocation as it wouldn't be destroyed until the end
    //of the program and therefore, use more memory
    CNFReader* reader = new CNFReader(instanceName);

    CNFReader::CNFReaderErrors error = reader->read();

    nbVar = reader->getNbVar();
    nbClauses = reader->getNbClauses();
    avgLenght = reader->getAverageLength();
    maxLenght = reader->getMaxLength();

    if (error != CNFReader::CNFReaderErrors::cnfError_noError) {
        printf("Error during file parsing: %s\n", CNFReader::getErrorString(error));
        return;
    }

#ifdef SATURNIN_PARALLEL
    solver = new ParallelSolver(nbThreads);
    solverToStop = solver;
    solver->initialize(*reader
#ifdef SATURNIN_DB
        , dbFileName
#endif /* SATURNIN_DB */
    );
#else
    solver = new Solver(reader->getNbVar(), reader->getNbClauses()
#ifdef SATURNIN_DB
        , dbFileName
#endif /* SATURNIN_DB */
    );
    solverToStop = solver;
    if (optSimplify) {
        simplify(*solver, *reader);
    }
    else {
        for (unsigned int i = 0; i<reader->getNbClauses(); i++) {
            solver->addClause(reader->getClause(i));
        }
    }
#endif /* SATURNIN_PARALLEL */
    
    const Array<Lit>& pres = reader->getPresumptions();
    for (unsigned int i = 0; i < pres.getSize(); i++) {
        solver->addPresumption(VariablesManager::getVar(pres[i]), VariablesManager::getLitSign(pres[i]));
    }

    readMem = sizeof(CNFReader) + reader->getMemoryFootprint() +
        sizeof(*solver) + solver->getMemoryFootprint();

    printf("c\tSaturnin\nc\t\tFile: %-70s\nc\t\tNb clauses:            %12d\n", instanceName, nbClauses);
    printf("c\t\tNb variables:          %12d\n", nbVar);
    printf("c\t\tMax length:            %12d\n", reader->getMaxLength());
    printf("c\t\tAverage length:        %12d\n", reader->getAverageLength());

    delete(reader);
    reader = nullptr;

    //add the handlers for CTRL-C signals
#if defined (WIN32) || defined (_MSC_VER)
    SetConsoleCtrlHandler(winSigStopLauncher, true);
    if (timeLim > 0) {
      printf("c\t\tTime limit set to:     %12d\n", timeLim);
      auto alreadyElapsed = w.getTimeEllapsed();
      waitingThread = std::unique_ptr<std::thread>(new std::thread([timeLim, alreadyElapsed]() {
          auto toWait = timeLim * 1000 - alreadyElapsed;
          std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(toWait));
          winSigStopLauncher(CTRL_CLOSE_EVENT);
      }));
    }
#else
    signal(SIGINT, sigStopLauncher);
    signal(SIGXCPU, sigStopLauncher);
    signal(SIGALRM, sigStopLauncher);
    rlimit rl;
    getrlimit(RLIMIT_CPU, &rl);
    if (timeLim > 0 && (rl.rlim_max == RLIM_INFINITY || (rlim_t)timeLim < rl.rlim_max)) {
        rl.rlim_cur = timeLim;
        if (setrlimit(RLIMIT_CPU, &rl) == -1) {
            printf("c\t\tWARNING! Could not set resource limit: CPU-time.\n");
        }
        else {
            printf("c\t\tTime limit set to:     %12d\n", timeLim);
        }
    }
#endif

#ifndef SATURNIN_PARALLEL
    unsigned int simplifiedNb = solver->getNbClauses();
    solver->simplify();
    printf("c\t\tNb clauses simplified: %12d\n", simplifiedNb - solver->getNbClauses());
    printf("c\t\tNb lit removed:        %12d\n", solver->getNbLitRemoved());
#endif
}

int saturnin::Launcher::solve()
{
    if(!solver)
        return EXIT_SUCCESS;
#ifdef VERBOSE
    solver->setVerbosity(1);
#else
    solver->setVerbosity(0);
#endif
    SATURNIN_BEGIN_PROFILE;
    unsigned int maxRestarts = std::numeric_limits<Lit>::max();
    int returnValue = EXIT_SUCCESS;
    if (solver->solve(maxRestarts)) {
        SATURNIN_END_PROFILE(totalCycles);
        w.stop();
#ifdef SATURNIN_PARALLEL
        returnValue = printResult(solver->getWinningSolver());
#else
        returnValue = printResult(*solver);
        returnValue = clean_exit;
#endif /* SATURNIN_PARALLEL */
    }
    else {
        w.stop();
        SATURNIN_END_PROFILE(totalCycles);
        if (solver->getState() == wUnknown) {
            printf("UNKNOWN\n");
        }
    }

    size_t mem = sizeof(*solver) + solver->getMemoryFootprint();

    printf("c Read memory (~):   ");
    printMemoryValue(stdout, readMem);
    printf("\n");
    printf("c Memory (~):        ");
    printMemoryValue(stdout, mem);
    printf("\n");
#ifdef SATURNIN_PARALLEL
    printStats(solver->getWinningSolver());
#else
    printStats(*solver);
#endif /* SATURNIN_PARALLEL */
#if defined (WIN32) || defined (_MSC_VER)
    if (waitingThread) {
      waitingThread->join();
    }
#endif

    return returnValue;
}

void saturnin::Launcher::stop() {
    solver->stop();
}

double saturnin::Launcher::getEllapsedTime() const { 
    if (solver->getState() == wUnknown)
        return w.getIntermediate();
    else
        return static_cast<double>(w.getSecondsEllapsed());
}

void saturnin::Launcher::printStats(Solver & s) const {
    printf("c Time:              %14.2f sec\n", w.getTimeEllapsed());
    if (s.getVerbosity() > 0) {
        printf("c Clause distribution:\n");
        const ClauseAllocator& alloc = s.getAllocator();
        for (unsigned int i = 0; i < alloc.getNbPools(); i++) {
            printf("c %4d-%4d: %14d/%-14d (%.3f)\n", (i + 1) * 2, ((i + 1) * 2) + 1,
                alloc.getPool(i).getSize(),
                alloc.getPool(i).getCapacity(),
                (1.0 * alloc.getPool(i).getSize()) / alloc.getPool(i).getCapacity());
        }
    }

    printf("c Nb propagations:   %14" PRIu64 " (%.2f propagations/second)\n",
        s.getNbPropagation(), s.getNbPropagation() / w.getTimeEllapsed());
    printf("c Nb conflicts:      %14" PRIu64 " (%.2f conflicts/second)\n",
        s.getNbConflict(), s.getNbConflict() / w.getTimeEllapsed());
    printf("c Nb restarts:       %14" PRIu64 "\n", s.getNbRestarts());
    printf("c Nb reduce:         %14d\n", s.getNbReduce());
    printf("c Clauses removed:   %14" PRIu64 " (initial: %" PRIu64 ", learnt: %" PRIu64 ")\n",
        s.getNbClausesRemoved() + s.getNbInitialClausesRemoved(),
        s.getNbInitialClausesRemoved(), s.getNbClausesRemoved());
    printf("c Clauses simplified:%14" PRIu64 "\n", s.getNbInitialClausesReduced());
    printf("c Nb pure literal:   %14" PRIu64 "\n", s.getNbPureLiteral());
#ifdef SATURNIN_COUNT_ACCESS
    printf("c Array access:      %14ld\n", SATURNIN_NB_ARRAY_ACCESS);
    printf("c Nb asserts:        %14ld\n", NB_ASSERTED);
#else
    printf("c Array access:             UNKNOWN\n");
#endif

#ifdef PROFILE
    printf("c Profile :\n");
    long double ldtotal = totalCycles / 100.0;
    printf("c   propagate watch: %5.2LF%% (%22" PRIu64 ")\n", s.__profile_propagateWatched / ldtotal, s.__profile_propagateWatched);
    printf("c   propagate bin  : %5.2LF%% (%22" PRIu64 ")\n", s.__profile_propagateBin / ldtotal, s.__profile_propagateBin);
    printf("c   propagate mono : %5.2LF%% (%22" PRIu64 ")\n", s.__profile_propagateMono / ldtotal, s.__profile_propagateMono);
    printf("c   analyze        : %5.2LF%% (%22" PRIu64 ")\n", s.__profile_analyze / ldtotal, s.__profile_analyze);
    printf("c   reduce         : %5.2LF%% (%22" PRIu64 ")\n", s.__profile_reduce / ldtotal, s.__profile_reduce);
    printf("c   simplify       : %5.2LF%% (%22" PRIu64 ")\n", s.__profile_simplify / ldtotal, s.__profile_simplify);
#endif
}

int saturnin::Launcher::printResult(Solver & s) const {
    int returnValue = 0;
#ifdef DEBUG
    if (s.validate() != true) {
        printf("Error: solution found is not correct!\n");
        return EXIT_FAILURE;
    }
#endif
    if (s.getState() == wTrue) {
        printf("s SATISFIABLE\nv");
        unsigned int linelenght = 1;
        unsigned int nlgth = 1;
        unsigned int log = 10;
        for (unsigned int i = 0; i < s.getNbVar(); i++) {
            printf(" %s%d", (s.getVarValue(i) == wTrue ? "" : "-"), i + 1);
            linelenght += nlgth + (s.getVarValue(i) == wTrue ? 1 : 2);

            if (i + 1 == log) {
                log *= 10;
                nlgth++;
            }

            if (linelenght + nlgth + 2 >= 80) {
                printf("\nv ");
                linelenght = 1;
            }

        }
        printf(" 0\n");
        returnValue = 10;
    }
    else {
        printf("s UNSATISFIABLE\n");
        returnValue = 20;
    }
    return clean_exit ? EXIT_SUCCESS : returnValue;
}

void saturnin::Launcher::printHelp(char * programName) const {
    printf("c Usage:\nc \t%s INSTANCE.CNF [-time-lim=X]\n", programName);
    printf("c \tINSTANCE.CNF the input file that contains the cnf version\n");
    printf("c \t             of the instance we want to solve\n");
    printf("c \t -h          print this help text\n");
    printf("c \t -clean-exit do not use exit codes %d for SAT and %d for UNSAT", 10, 20);
#if defined (WIN32) || defined (_MSC_VER)
    printf("c \t-time-lim=X  the number of seconds allowed before stopping\n");
    printf("c \t             the search process\n");
#endif /* WIN32 */
#ifdef SATURNIN_PARALLEL
    printf("c \t-t=X         the number of threads to be used in the search\n");
#endif /* SATURNIN_PARALLEL */
#ifdef SATURNIN_DB
    printf("c \t-d=fileName  the file to be used to store the database\n");
#endif /* SATURNIN_DB */
}
