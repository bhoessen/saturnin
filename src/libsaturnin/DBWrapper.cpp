#include "saturnin/DBWrapper.h"
#include "saturnin/Logger.h"
#include "saturnin/VariablesManager.h"
#ifdef SATURNIN_DB
#include <sqlite3.h>
#include <algorithm>
#include <iosfwd>
#include <sstream>
#include <map>
#include <algorithm>

#include "../Utils.h"
#include "DBWrapperUtils.h"

using namespace saturnin;

extern "C" {

    int executeTreatment(void* exp, int nbColumns, char** columnsValue, char** columnsName) {
        QueryTreatment* t = (QueryTreatment*) exp;
        return t->execute(nbColumns, columnsValue, columnsName);
    }
}

class ClauseCounter : public QueryTreatment {
public:

    ClauseCounter() : count(0), set(false) {
    }

#ifdef DEBUG

    virtual int execute(int nbColumns, char** columnsValue, char**) {
#else

    virtual int execute(int, char** columnsValue, char**) {
#endif /* DEBUG */
        ASSERT_EQUAL(1, nbColumns);
        ASSERT(!set);
        char* end = nullptr;
        count = strtol(columnsValue[0], &end, 10);
        ASSERT(end > columnsValue[0]);
        return SQLITE_OK;
    }

    long getCount() {
        return count;
    }

private:
    long count;
    bool set;
};

DBWrapper::DBWrapper(const char* aDBName, bool verbose) :
internalDBEngine(nullptr), nbClauses(0), produceLogs(verbose) {
    if (aDBName == nullptr) return;

    int retVal = sqlite3_open_v2(aDBName, &internalDBEngine,
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE , nullptr);
    if (SQLITE_OK != retVal || internalDBEngine == nullptr) {
        if (internalDBEngine != nullptr) {
            sqlite3_close(internalDBEngine);
        }
        internalDBEngine = nullptr;
        if (produceLogs)
            fprintf(Logger::getDBLogger().getOutput(),
#if SQLITE_VERSION_NUMBER > 3007016
                "c [dblog] Unable to open/create database '%s': %s\n",
                aDBName, sqlite3_errstr(retVal));
#else
                "c [dblog] Unable to open/create database '%s'\n",
                aDBName);
#endif
        return;
    }

    /* Create the tables needed */

    executeQuery("CREATE TABLE clause(id INTEGER, literals TEXT, lbd INTEGER, height INTEGER);");
    executeQuery("CREATE TABLE resolution(clause TEXT, clauseID INTEGER, resolvent INTEGER);");

    //count the number of clauses already present in the database
    ClauseCounter c;
    executeQuery("SELECT COUNT(id) FROM clause;", &c);
    nbClauses = c.getCount();
}

DBWrapper::~DBWrapper() {
    if (internalDBEngine == nullptr) return;
    int returnCode = sqlite3_close(internalDBEngine);
    if (returnCode != SQLITE_OK) {
        fprintf(Logger::getDBLogger().getOutput(),
               "c [dblog] unable to close the DB: %s (error code: %d)\n",
#if SQLITE_VERSION_NUMBER > 3007016
                sqlite3_errstr(returnCode), returnCode);
#else
                sqlite3_errmsg(internalDBEngine), returnCode);
#endif
    }
}

size_t DBWrapper::getMemoryFootprint() const {
    if (internalDBEngine == nullptr) return 0;

    int mem = 0;
    int max = 0;
    sqlite3_status(SQLITE_STATUS_MEMORY_USED, &mem, &max, false);
    return mem;
}

bool DBWrapper::addResolution(const Lit * const lits, long resolvent) {
    if (internalDBEngine == nullptr) return true;
#ifdef DEBUG
    if (produceLogs) fprintf(Logger::getDBLogger().getOutput(), "c [dblog] %s\n", __PRETTY_FUNCTION__);
#endif /* DEBUG */

    std::stringstream aInsertStrm;
    aInsertStrm << "INSERT INTO resolution(clause, resolvent) VALUES('";
    aInsertStrm << VariablesManager::toDimacs(*lits);
    for (const Lit* i = lits + 1; *i != lit_Undef; i++) {
        aInsertStrm << " " << VariablesManager::toDimacs(*i);
    }
    aInsertStrm << "', " << resolvent << ");";
    return executeQuery(aInsertStrm.str().c_str());
}

bool DBWrapper::addClause(const Lit * const lits, long id) {
    if (internalDBEngine == nullptr) return true;
#ifdef DEBUG
    if (produceLogs) fprintf(Logger::getDBLogger().getOutput(), "c [dblog] %s\n", __PRETTY_FUNCTION__);
#endif /* DEBUG */

    std::stringstream aInsertStrm;
    aInsertStrm << "INSERT INTO clause(literals, id) VALUES('";
    if (lits != nullptr && *lits != lit_Undef) {
        aInsertStrm << VariablesManager::toDimacs(*lits);
        for (const Lit* i = lits + 1; *i != lit_Undef; i++) {
            aInsertStrm << " " << VariablesManager::toDimacs(*i);
        }
    }
    aInsertStrm << "', " << id << ");";
    if (executeQuery(aInsertStrm.str().c_str())) {
        nbClauses++;
        return true;
    } else {
        return false;
    }
}

std::string DBWrapper::sortLiterals(const std::string& clause) const {
    Array<int> lits(clause.size());
    unsigned int sz = clause.size();
    const char* chars = clause.c_str();
    const char* end = nullptr;

    while (sz > 0) {
        int nbRead = 0;
        int64_t read = strntol(chars, &end, sz, nbRead);
        if (nbRead > 0 && read >= std::numeric_limits<int>::min() &&
                read <= std::numeric_limits<int>::max()) {
            lits.push((int) read);
        }
        sz = end - chars;
        chars = end;
    }

    std::sort((int*) lits, ((int*) lits) + lits.getSize());

    std::stringstream out;
    if (lits.getSize() > 0) {
        out << lits[0U];
        for (unsigned int i = 1; i < lits.getSize(); i++) {
            out << " " << lits[i];
        }
    }
    return out.str();
}

void DBWrapper::sanatize() {
    if (internalDBEngine == nullptr) return;
#ifdef DEBUG
    if (produceLogs) fprintf(Logger::getDBLogger().getOutput(), "c [dblog] %s\n", __PRETTY_FUNCTION__);
#endif /* DEBUG */
    {
        //make sure that the literals in the clauses are sorted
        PreparedStatement stmt(internalDBEngine, "SELECT literals, id FROM clause;", produceLogs);
        sqlite3_stmt* selectClausesStmt = stmt;
        int errorCode;
        for (errorCode = sqlite3_step(selectClausesStmt);
                errorCode == SQLITE_ROW;
                errorCode = sqlite3_step(selectClausesStmt)) {
            ASSERT_EQUAL(2, sqlite3_column_count(selectClausesStmt));
            ASSERT_EQUAL(SQLITE_TEXT, sqlite3_column_type(selectClausesStmt, 0));
            ASSERT_EQUAL(SQLITE_INTEGER, sqlite3_column_type(selectClausesStmt, 1));

            sqlite3_int64 id = sqlite3_column_int64(selectClausesStmt, 1);
            const unsigned char* literals = sqlite3_column_text(selectClausesStmt, 0);
            //we are allowed to reinterpret_cast since we will not handle the full
            //utf-8 chars, but chars whithin [0-9]
            std::string clause(reinterpret_cast<const char*> (literals));
            std::string sortedClause(sortLiterals(clause));

            std::stringstream updateQuery;
            updateQuery << "UPDATE clause SET literals = '";
            updateQuery << sortedClause << "' WHERE id = ";
            updateQuery << id << ";";
            executeQuery(updateQuery.str().c_str());
        }
    }

    {
        //map the clause text with their clause id from the resolution table
        PreparedStatement selectStmt(internalDBEngine,
                "SELECT clause, resolvent from resolution;", produceLogs);
        sqlite3_stmt* stmt = selectStmt;
        int errorCode;
        for (errorCode = sqlite3_step(stmt);
                errorCode == SQLITE_ROW;
                errorCode = sqlite3_step(stmt)) {
            ASSERT_EQUAL(2, sqlite3_column_count(stmt));
            ASSERT_EQUAL(SQLITE_TEXT, sqlite3_column_type(stmt, 0)); //clause
            ASSERT_EQUAL(SQLITE_INTEGER, sqlite3_column_type(stmt, 1)); //clauseID

            sqlite3_int64 id = sqlite3_column_int64(stmt, 1);
            const unsigned char* literals = sqlite3_column_text(stmt, 0);
            //we are allowed to reinterpret_cast since we will not handle the full
            //utf-8 chars, but chars whithin [0-9]
            std::string clause(reinterpret_cast<const char*> (literals));
            std::string sortedClause(sortLiterals(clause));

            std::stringstream updateQuery;
            updateQuery << "UPDATE resolution ";
            updateQuery << "SET clauseID = ";
            updateQuery << "  (SELECT MAX(clause.id) ";
            updateQuery << "   FROM clause ";
            updateQuery << "   WHERE clause.literals = '" << sortedClause << "' ";
            updateQuery << "     AND clause.id < " << id << ") ";
            updateQuery << "WHERE resolution.clause = '" << clause << "'";
            updateQuery << "  AND resolution.resolvent = " << id << ";";
            executeQuery(updateQuery.str().c_str());
        }
    }

}

void DBWrapper::computeHeights() {
    if (internalDBEngine == nullptr) return;
#ifdef DEBUG
    if (produceLogs) fprintf(Logger::getDBLogger().getOutput(), "c [dblog] %s\n", __PRETTY_FUNCTION__);
#endif /* DEBUG */
    //set the height of the initial clauses
    executeQuery("UPDATE clause SET height = 0 WHERE NOT EXISTS (SELECT clauseId FROM resolution WHERE resolvent = clause.id);");

    //compute the height of the learnt clauses
    PreparedStatement stmt(internalDBEngine, "SELECT id FROM clause WHERE height IS NULL ORDER BY id;", produceLogs);
    sqlite3_stmt* selectClausesStmt = stmt;
    int errorCode;
    for (errorCode = sqlite3_step(selectClausesStmt);
            errorCode == SQLITE_ROW;
            errorCode = sqlite3_step(selectClausesStmt)) {
        ASSERT_EQUAL(1, sqlite3_column_count(selectClausesStmt));
        ASSERT_EQUAL(SQLITE_INTEGER, sqlite3_column_type(selectClausesStmt, 0));

        sqlite3_int64 id = sqlite3_column_int64(stmt, 0);

        std::stringstream q;
        q << "UPDATE clause ";
        q << "SET height = (";
        q << "    SELECT 1+MAX(c.height)";
        q << "    FROM  clause c INNER JOIN resolution ON";
        q << "        c.id = resolution.clauseId ";
        q << "    WHERE resolution.resolvent = " << id;
        q << "    ) ";
        q << "WHERE clause.id = " << id << " AND height IS NULL;";
        std::string query = q.str();
        executeQuery(query.c_str());
    }
}

class ResolutionChecker : public QueryTreatment {
public:

    ResolutionChecker(bool verb) : result(true), verbose(verb),
    currentClauseId(-1), currentClause(), resolutions() {
    }

    virtual ~ResolutionChecker();

    bool getResult() {
        if (resolutions.getSize() > 0 && result) {
            result = validateResolution();
            resolutions.pop(resolutions.getSize());
        }
        return result;
    }
#ifdef DEBUG

    virtual int execute(int nbColumns, char** columnsValue, char**) {
#else

    virtual int execute(int, char** columnsValue, char**) {
#endif /* DEBUG */
        ASSERT_EQUAL(3, nbColumns);
        char* endPtr;
        long resolventId = strtol(columnsValue[0], &endPtr, 10);
        ASSERT(endPtr > columnsValue[0]);
        if (resolventId != currentClauseId) {
            //make sure the previous resolution was correct
            if (!validateResolution()) {
                return -1;
            }

            currentClauseId = resolventId;
            currentClause.pop(currentClause.getSize());
            insertLiterals(columnsValue[2], currentClause);
            std::sort((Lit*) currentClause, currentClause.getSize() + (Lit*) currentClause);
            resolutions.pop(resolutions.getSize());
        }
        insertLiterals(columnsValue[1], resolutions);
        return SQLITE_OK;
    }



private:

    void insertLiterals(char* cnfRep, Array<Lit>& toFill) {
        char* end = nullptr;
        int64_t v = strtol(cnfRep, &end, 10);
        while (end != cnfRep) {
            Lit l = VariablesManager::fromDimacs(v);
            if (l != lit_Undef) {
                toFill.push(l);
            }
            cnfRep = end;
            v = strtol(cnfRep, &end, 10);
        }
    }

    bool validateResolution() {
        std::sort((Lit*) resolutions, resolutions.getSize() + (Lit*) resolutions);
        if (verbose) {
            fprintf(Logger::getDBLogger().getOutput(), "c [dblog] ");
            for (unsigned int k = 0; k < resolutions.getSize(); k++) {
                fprintf(Logger::getDBLogger().getOutput(), "%d ",
                        VariablesManager::toDimacs(resolutions[k]));
            }
            fprintf(Logger::getDBLogger().getOutput(), "-> ");
        }
        //first, remove doubles
        unsigned int i = 0, j = 0;
        while (resolutions.getSize() > 0 && i < resolutions.getSize()) {
            unsigned int lPos = i;
            //we search for the position with the highest index such that
            //resolutions[i] == resolutions[lPos]
            while (i + 1 < resolutions.getSize() &&
                    resolutions[i] == resolutions[i + 1]) {
                i++;
            }
            if (j == 0 || VariablesManager::oppositeLit(resolutions[j - 1]) !=
                    resolutions[lPos]) {
                resolutions[j] = resolutions[lPos];
                j++;
            } else {
                ASSERT(j > 0);
                ASSERT_EQUAL(VariablesManager::oppositeLit(resolutions[j - 1]), resolutions[lPos]);
                j--;
            }
            i++;
        }
        resolutions.pop(i - j);
        if (verbose) {
            for (unsigned int k = 0; k < resolutions.getSize(); k++) {
                fprintf(Logger::getDBLogger().getOutput(), "%d ",
                        VariablesManager::toDimacs(resolutions[k]));
            }
            fprintf(Logger::getDBLogger().getOutput(), "\n");
        }
        if (!(resolutions == currentClause)) {
            fprintf(Logger::getDBLogger().getOutput(), "c [dblog] error: clause ");
            for (unsigned int k = 0; k < currentClause.getSize(); k++) {
                fprintf(Logger::getDBLogger().getOutput(), "%d ",
                        VariablesManager::toDimacs(currentClause[k]));
            }
            fprintf(Logger::getDBLogger().getOutput(), "!= ");
            for (unsigned int k = 0; k < resolutions.getSize(); k++) {
                fprintf(Logger::getDBLogger().getOutput(), "%d ",
                        VariablesManager::toDimacs(resolutions[k]));
            }
            fprintf(Logger::getDBLogger().getOutput(), "\n");
            result = false;
            return false;
        }
        return true;
    }

    bool result;
    bool verbose;
    long currentClauseId;
    Array<Lit> currentClause;
    Array<Lit> resolutions;

};

ResolutionChecker::~ResolutionChecker() {
}

bool DBWrapper::checkResolutions() {
    if (internalDBEngine == nullptr) return true;
#ifdef DEBUG
    if (produceLogs) fprintf(Logger::getDBLogger().getOutput(), "c [dblog] %s\n", __PRETTY_FUNCTION__);
#endif /* DEBUG */

    ResolutionChecker check(produceLogs);
    executeQuery("SELECT resolution.resolvent, resolution.clause, clause.literals "
            "from resolution JOIN clause on resolution.resolvent = clause.id "
            "ORDER BY resolution.resolvent;", &check);

    return check.getResult();
}

class ClauseFinder : public QueryTreatment {
public:

    ClauseFinder() : nbClausesFound(0U) {
    }

#ifdef DEBUG

    virtual int execute(int nbColumns, char**, char**) {
#else

    virtual int execute(int, char**, char**) {
#endif /* DEBUG */
        ASSERT_EQUAL(1, nbColumns);
        nbClausesFound++;
        return SQLITE_OK;
    }

    unsigned int getNbClausesFound() {
        return nbClausesFound;
    }

private:

    unsigned int nbClausesFound;

};

bool DBWrapper::checkContainsClause(const Lit * const c) {
    if (internalDBEngine == nullptr) return true;
#ifdef DEBUG
    if (produceLogs) fprintf(Logger::getDBLogger().getOutput(), "c [dblog] %s\n", __PRETTY_FUNCTION__);
#endif /* DEBUG */
    Array<Lit> clause;
    for (unsigned int i = 0; c[i] != lit_Undef; i++) {
        clause.push(c[i]);
    }
    std::sort((Lit*) clause, clause.getSize() + (Lit*) clause);
    ClauseFinder finder;
    std::stringstream query;
    query << "SELECT id FROM clause WHERE literals='";
    for (unsigned int i = 0; i < clause.getSize(); i++) {
        query << VariablesManager::toDimacs(clause[i]);
        if (i + 1 < clause.getSize()) {
            query << " ";
        }
    }
    query << "';";
    executeQuery(query.str().c_str(), &finder);
    return finder.getNbClausesFound() > 0;
}

bool DBWrapper::executeQuery(const char* query) {
    return executeQuery(query, nullptr);
}

bool DBWrapper::executeQuery(const char* query, QueryTreatment* treat) {
    if (internalDBEngine == nullptr) return true;
    if (produceLogs) fprintf(Logger::getDBLogger().getOutput(), "c [dblog] executing '%s'\n", query);

    char* errorMessages = nullptr;
    int retVal;
    if (treat != nullptr) {
        retVal = sqlite3_exec(internalDBEngine,
                query, &executeTreatment, (void*) treat, &errorMessages);
    } else {
        retVal = sqlite3_exec(internalDBEngine,
                query, nullptr, nullptr, &errorMessages);
    }
    if (retVal != SQLITE_OK) {
        fprintf(Logger::getDBLogger().getOutput(),
#if SQLITE_VERSION_NUMBER > 3007016
                "c [dblog] error: %s, %s\n", sqlite3_errstr(retVal), errorMessages);
#else
                "c [dblog] error: %s, %s\n", sqlite3_errmsg(internalDBEngine), errorMessages);
#endif
        SATURNIN_TRACE(Logger::getDBLogger());
        sqlite3_free(errorMessages);
        return false;
    }
    return true;
}

#endif /* SATURNIN_DB */

