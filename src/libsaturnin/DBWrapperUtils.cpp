#include "./DBWrapperUtils.h"
#include "saturnin/Logger.h"
#include "saturnin/Assert.h"

#ifdef SATURNIN_DB
using namespace saturnin;

PreparedStatement::PreparedStatement(sqlite3* aDBEngine, const char* query, bool verbose) :
internalDBEngine(aDBEngine), preparedStatement(nullptr) {
    if (verbose) fprintf(Logger::getDBLogger().getOutput(), "c [dblog] preparing '%s'\n", query);
    ASSERT(aDBEngine != nullptr);
    int errorCode = sqlite3_prepare_v2(internalDBEngine, query, -1, &preparedStatement, nullptr);
    if (errorCode != SQLITE_OK) {
        fprintf(Logger::getDBLogger().getOutput(),
                "c [dblog] unable to prepare '%s' : %s [%d, e:%d]\n",
                query, sqlite3_errmsg(internalDBEngine), errorCode,
                sqlite3_extended_errcode(internalDBEngine));
        errorCode = sqlite3_finalize(preparedStatement);
        preparedStatement = nullptr;
        if (errorCode != SQLITE_OK) {
            fprintf(Logger::getDBLogger().getOutput(),
#if SQLITE_VERSION_NUMBER > 3007016
                    "c [dblog] %s", sqlite3_errstr(errorCode));
#else
                    "c [dblog] %s", sqlite3_errmsg(internalDBEngine));
#endif
        }
    }
}

PreparedStatement::~PreparedStatement() {
    if(preparedStatement == nullptr) return;
    int returnCode = sqlite3_finalize(preparedStatement);
    if (returnCode != SQLITE_OK) {
        fprintf(Logger::getDBLogger().getOutput(),
                "c [dblog] Unable to finalize prepared statement: %s (errorcode: %d)\n",
#if SQLITE_VERSION_NUMBER > 3007016
                sqlite3_errstr(returnCode), returnCode);
#else
                sqlite3_errmsg(internalDBEngine), returnCode);
#endif
    }
}

#endif /* SATURNIN_DB */
