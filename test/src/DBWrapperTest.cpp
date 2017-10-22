#include "DBWrapperTest.h"
#include "../../include/saturnin/DBWrapper.h"
#include "../../src/libsaturnin/DBWrapperUtils.h"
#include "../../include/saturnin/VariablesManager.h"
#include <cstdio>
#ifdef SATURNIN_DB
#include <sqlite3.h>

CPPUNIT_TEST_SUITE_REGISTRATION(DBWrapperTest);

class DBHelper {
public:

    DBHelper(const char* fileName) : internalDBEngine(NULL) {
        if (SQLITE_OK != sqlite3_open(fileName, &internalDBEngine)) {
            sqlite3_close(internalDBEngine);
        }
    }

    ~DBHelper() {
        if (internalDBEngine != NULL) {
            sqlite3_close(internalDBEngine);
        }
    }

    DBHelper(const DBHelper& other) : internalDBEngine(other.internalDBEngine) {
    }

    DBHelper& operator=(const DBHelper& other) {
        internalDBEngine = other.internalDBEngine;
        return *this;
    }

    sqlite3* internalDBEngine;
};

extern "C" {

    typedef struct validationData_t {
        int nbColumns;
        const char** expectedValues;
        int nbExecuted;
    } validationData;

    int validate(void* exp, int nbColumns, char** columnsValue, char** columnsName) {
        validationData* expected = (validationData*) exp;
        bool error = expected->nbColumns != nbColumns;
        if (error) {
            printf("Expected: %d columns, actual: %d columns\n", expected->nbColumns, nbColumns);
            return -1;
        }
        expected->nbExecuted++;
        for (int i = 0; i < nbColumns; i++) {
            if ((columnsValue[i] != NULL && expected->expectedValues[i] == NULL) ||
                    (columnsValue[i] == NULL && expected->expectedValues[i] != NULL) ||
                    (columnsValue[i] != NULL && expected->expectedValues[i] != NULL &&
                    strcmp(columnsValue[i], expected->expectedValues[i]) != 0)) {
                std::stringstream stack;
                SATURNIN_TRACE(stack);
                printf("'%s':\nexpected '%s'\n actual: '%s'\nStack: %s\n",
                        columnsName[i],
                        expected->expectedValues[i] != NULL ? expected->expectedValues[i] : "NULL",
                        columnsValue[i] != NULL ? columnsValue[i] : "NULL",
                        stack.str().c_str());
                error = true;
            }
        }
        return error ? -1 : SQLITE_OK;
    }
}

int DBWrapperTest::validateResults(const char* fileName, const char* query, int nbColumns, const char** values) {
    DBHelper aDBHelper(fileName);
    if (aDBHelper.internalDBEngine == NULL) {
        printf("Unable to open '%s'\n", fileName);
        return -1;
    }
    sqlite3* internalDBEngine = aDBHelper.internalDBEngine;

    char* errorMessage = NULL;
    validationData expected;
    expected.nbColumns = nbColumns;
    expected.expectedValues = values;
    expected.nbExecuted = 0;
    int returnValue = sqlite3_exec(internalDBEngine,
            query,
            &validate, &expected, &errorMessage);
    if (errorMessage != NULL) {
        printf("%s", errorMessage);
        sqlite3_free(errorMessage);
    }
    if (expected.nbExecuted == 0) {
        returnValue = SQLITE_NOTFOUND;
    }
    return returnValue;
}

void DBWrapperTest::testCreation() {
    const char* fileName = ".DBWrapperTest.testCreation.db~";
    remove(fileName);
    FILE* outputLog = fopen(".DBWrapperTest.testCreation.log~", "w+");
    saturnin::Logger::getDBLogger().setOutput(outputLog);
    saturnin::DBWrapper aWrapper(fileName, true);
    fclose(outputLog);
    saturnin::Logger::getDBLogger().setOutput(saturnin::Logger::getStdOutLogger().getOutput());
    CPPUNIT_ASSERT(aWrapper.isInitialized());
    CPPUNIT_ASSERT_EQUAL(0l, aWrapper.nbClausesAdded());
}

void DBWrapperTest::testInsertResolventClause() {
    const char* fileName = ".DBWrapperTest.testResolventClause.db~";
    remove(fileName);
    FILE* outputLog = fopen(".DBWrapperTest.testResolventClause.log~", "w+");
    saturnin::Logger::getDBLogger().setOutput(outputLog);
    {
        saturnin::DBWrapper aWrapper(fileName, true);
        saturnin::Lit clause[] = {7U, 0U, 2U, 5U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addResolution(clause, 2));
    }
    fclose(outputLog);
    saturnin::Logger::getDBLogger().setOutput(saturnin::Logger::getStdOutLogger().getOutput());

    const char* values[3];
    values[0] = "-4 1 2 -3";
    values[1] = "2";
    values[2] = NULL;

    CPPUNIT_ASSERT_EQUAL(SQLITE_OK, validateResults(fileName,
            "SELECT clause, resolvent, clauseID FROM resolution WHERE resolvent=2;", 3, values));

}

void DBWrapperTest::testInsertClause() {
    const char* fileName = ".DBWrapperTest.testInsertClause.db~";
    remove(fileName);
    FILE* outputLog = fopen(".DBWrapperTest.testInsertClause.log~", "w+");
    saturnin::Logger::getDBLogger().setOutput(outputLog);
    long size;
    {
        saturnin::DBWrapper aWrapper(fileName, true);
        saturnin::Lit clause[] = {7U, 0U, 2U, 4U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause, 2));
        size = aWrapper.nbClausesAdded();
    }
    fclose(outputLog);
    saturnin::Logger::getDBLogger().setOutput(saturnin::Logger::getStdOutLogger().getOutput());

    const int nbExpectedColumns = 4;
    const char* expectedData[nbExpectedColumns];
    expectedData[0] = "-4 1 2 3";
    expectedData[1] = "2";
    expectedData[2] = NULL;
    expectedData[3] = NULL;
    CPPUNIT_ASSERT_EQUAL(SQLITE_OK, validateResults(fileName,
            "SELECT literals, id, lbd, height FROM clause WHERE id=2;", nbExpectedColumns, expectedData));

    CPPUNIT_ASSERT_EQUAL(1l, size);
}

void DBWrapperTest::testMemoryFootprint() {
    const char* fileName = ".DBWrapperTest.testMemoryFootprint.db~";
    remove(fileName);
    FILE* outputLog = fopen(".DBWrapperTest.testMemoryFootprint.log~", "w+");
    size_t mem = 0;
    saturnin::Logger::getDBLogger().setOutput(outputLog);
    {
        saturnin::DBWrapper aWrapper(fileName, true);
        saturnin::Lit clause[] = {7U, 0U, 2U, 4U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause, 2));
        mem = aWrapper.getMemoryFootprint();
    }
    fclose(outputLog);
    saturnin::Logger::getDBLogger().setOutput(saturnin::Logger::getStdOutLogger().getOutput());
    CPPUNIT_ASSERT(mem != 0);
}

void DBWrapperTest::testOpenExistingDB() {
    const char* fileName = ".DBWrapperTest.testOpenExistingDB.db~";
    remove(fileName);
    FILE* outputLog = fopen(".DBWrapperTest.testOpenExistingDB.log~", "w+");
    saturnin::Logger::getDBLogger().setOutput(outputLog);
    {
        saturnin::DBWrapper aWrapper(fileName, true);

        saturnin::Lit clause[] = {7U, 0U, 2U, 4U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause, 2));

        saturnin::Lit clause1[] = {1U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause1, 1));

        saturnin::Lit clause2[] = {0U, 8U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause2, 0));

        saturnin::Lit clause3[] = {9U, 11U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause3, 5));
    }

    long size = 0;
    {
        saturnin::DBWrapper aWrapper(fileName, true);
        size = aWrapper.nbClausesAdded();
    }

    CPPUNIT_ASSERT_EQUAL(4l, size);
    fclose(outputLog);
    saturnin::Logger::getDBLogger().setOutput(saturnin::Logger::getStdOutLogger().getOutput());
}

void DBWrapperTest::testComputeHeights() {
    const char* fileName = ".DBWrapperTest.testComputeHeights.db~";
    remove(fileName);
    FILE* outputLog = fopen(".DBWrapperTest.testComputeHeights.log~", "w+");
    saturnin::Logger::getDBLogger().setOutput(outputLog);
    {
        saturnin::DBWrapper aWrapper(fileName, true);

        saturnin::Lit clause[] = {7U, 0U, 2U, 4U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause, 2));

        saturnin::Lit clause1[] = {1U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause1, 1));

        saturnin::Lit clause2[] = {0U, 8U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause2, 0));

        saturnin::Lit clause3[] = {9U, 11U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause3, 5));

        saturnin::Lit res[] = {7U, 2U, 4U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(res, 3));
        CPPUNIT_ASSERT(aWrapper.addResolution(clause1, 3));
        CPPUNIT_ASSERT(aWrapper.addResolution(clause, 3));

        saturnin::Lit res2[] = {8U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(res2, 4));
        CPPUNIT_ASSERT(aWrapper.addResolution(clause1, 4));
        CPPUNIT_ASSERT(aWrapper.addResolution(clause2, 4));

        saturnin::Lit res3[] = {11U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(res3, 6));
        CPPUNIT_ASSERT(aWrapper.addResolution(clause3, 6));
        CPPUNIT_ASSERT(aWrapper.addResolution(res2, 6));

        aWrapper.sanatize();
        aWrapper.computeHeights();
    }
    fclose(outputLog);
    saturnin::Logger::getDBLogger().setOutput(saturnin::Logger::getStdOutLogger().getOutput());

    const int nbExpectedColumns = 4;
    const char* expectedData[nbExpectedColumns];
    expectedData[0] = "-4 1 2 3";
    expectedData[1] = "2";
    expectedData[2] = NULL;
    expectedData[3] = "0";
    CPPUNIT_ASSERT_EQUAL(SQLITE_OK, validateResults(fileName,
            "SELECT literals, id, lbd, height FROM clause WHERE id=2;", nbExpectedColumns, expectedData));

    expectedData[0] = "-1";
    expectedData[1] = "1";
    expectedData[2] = NULL;
    expectedData[3] = "0";
    CPPUNIT_ASSERT_EQUAL(SQLITE_OK, validateResults(fileName,
            "SELECT literals, id, lbd, height FROM clause WHERE id=1;", nbExpectedColumns, expectedData));

    expectedData[0] = "-4 2 3";
    expectedData[1] = "3";
    expectedData[2] = NULL;
    expectedData[3] = "1";
    CPPUNIT_ASSERT_EQUAL(SQLITE_OK, validateResults(fileName,
            "SELECT literals, id, lbd, height FROM clause WHERE id=3;", nbExpectedColumns, expectedData));

    expectedData[0] = "-6";
    expectedData[1] = "6";
    expectedData[2] = NULL;
    expectedData[3] = "2";
    CPPUNIT_ASSERT_EQUAL(SQLITE_OK, validateResults(fileName,
            "SELECT literals, id, lbd, height FROM clause WHERE id=6;", nbExpectedColumns, expectedData));
}

void DBWrapperTest::testSanitize() {
    const char* fileName = ".DBWrapperTest.testSanitize.db~";
    remove(fileName);
    FILE* outputLog = fopen(".DBWrapperTest.testSanitize.log~", "w+");
    saturnin::Logger::getDBLogger().setOutput(outputLog);
    {
        saturnin::DBWrapper aWrapper(fileName, true);
        saturnin::Lit clause[] = {7U, 0U, 2U, 4U, saturnin::lit_Undef};
        saturnin::Lit clause2[] = {10U, 0U, 2U, 4U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause, 2));
        CPPUNIT_ASSERT(aWrapper.addClause(clause2, 3));
        CPPUNIT_ASSERT(aWrapper.addClause(clause2, 7));
        CPPUNIT_ASSERT(aWrapper.addClause(clause2, 0));
        saturnin::Lit clause3[] = {4U, 2U, 0U, 10U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addResolution(clause3, 4));
        aWrapper.sanatize();
    }
    fclose(outputLog);
    saturnin::Logger::getDBLogger().setOutput(saturnin::Logger::getStdOutLogger().getOutput());

    const char* expectedData[3];
    expectedData[0] = "1 2 3 6";
    expectedData[1] = "3";
    expectedData[2] = NULL;
    CPPUNIT_ASSERT_EQUAL(SQLITE_OK, validateResults(fileName,
            "SELECT literals, id, lbd FROM clause WHERE id=3;", 3, expectedData));

    expectedData[0] = "3";
    CPPUNIT_ASSERT_EQUAL(SQLITE_OK, validateResults(fileName,
            "SELECT clauseID FROM resolution WHERE resolvent=4", 1, expectedData));

}

void DBWrapperTest::testOpenWrongFile() {
    const char* fileName = "/.DBWrapperTest.testOpenWrongFile.db~";
    remove(fileName);
    FILE* outputLog = fopen(".DBWrapperTest.testOpenWrongFile.log~", "w+");
    saturnin::Logger::getDBLogger().setOutput(outputLog);
    {
        saturnin::DBWrapper aWrapper(fileName, true);
    }
    fclose(outputLog);
    saturnin::Logger::getDBLogger().setOutput(saturnin::Logger::getStdOutLogger().getOutput());

}

void DBWrapperTest::testPrepareFailingStmt() {
    const char* fileName = ".DBWrapperTest.testPrepareFailingStmt.db~";
    remove(fileName);
    FILE* outputLog = fopen(".DBWrapperTest.testPrepareFailingStmt.log~", "w+");
    saturnin::Logger::getDBLogger().setOutput(outputLog);
    {
        DBHelper help(fileName);
        saturnin::PreparedStatement stmt(help.internalDBEngine, "SELECT * from noTable;", true);
    }
    fclose(outputLog);
    saturnin::Logger::getDBLogger().setOutput(saturnin::Logger::getStdOutLogger().getOutput());
}

void DBWrapperTest::testValidate() {
    const char* fileName = ".DBWrapperTest.testValidate.db~";
    remove(fileName);
    FILE* outputLog = fopen(".DBWrapperTest.testValidate.log~", "w+");
    bool result;
    saturnin::Logger::getDBLogger().setOutput(outputLog);
    {
        saturnin::DBWrapper aWrapper(fileName, true);

        saturnin::Lit clause0[] = {0U, 8U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause0, 0));

        saturnin::Lit clause1[] = {1U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause1, 1));

        saturnin::Lit clause2[] = {7U, 0U, 2U, 4U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause2, 2));

        saturnin::Lit clause3[] = {9U, 11U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause3, 3));

        saturnin::Lit res[] = {7U, 2U, 4U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(res, 5));
        CPPUNIT_ASSERT(aWrapper.addResolution(clause1, 5));
        CPPUNIT_ASSERT(aWrapper.addResolution(clause2, 5));

        saturnin::Lit res2[] = {8U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(res2, 4));
        CPPUNIT_ASSERT(aWrapper.addResolution(clause1, 4));
        CPPUNIT_ASSERT(aWrapper.addResolution(clause0, 4));

        saturnin::Lit res3[] = {11U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(res3, 6));
        CPPUNIT_ASSERT(aWrapper.addResolution(clause3, 6));
        CPPUNIT_ASSERT(aWrapper.addResolution(res2, 6));

        //Add a more complex resolution
        saturnin::Lit clause7[] = {saturnin::VariablesManager::fromDimacs(-2),
            saturnin::VariablesManager::fromDimacs(3),
            saturnin::VariablesManager::fromDimacs(-4),
            saturnin::VariablesManager::fromDimacs(-5),
            saturnin::VariablesManager::fromDimacs(6), saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause7, 7));

        saturnin::Lit res8[] = {saturnin::VariablesManager::fromDimacs(3),
            saturnin::VariablesManager::fromDimacs(-4), saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(res8, 8));
        CPPUNIT_ASSERT(aWrapper.addResolution(clause7, 8));
        CPPUNIT_ASSERT(aWrapper.addResolution(clause0, 8));
        CPPUNIT_ASSERT(aWrapper.addResolution(clause1, 8));
        CPPUNIT_ASSERT(aWrapper.addResolution(clause3, 8));
        CPPUNIT_ASSERT(aWrapper.addResolution(res, 8));
        aWrapper.sanatize();
        result = aWrapper.checkResolutions();
    }
    fclose(outputLog);
    saturnin::Logger::getDBLogger().setOutput(saturnin::Logger::getStdOutLogger().getOutput());

    CPPUNIT_ASSERT(result);

}

void DBWrapperTest::testContainsClause() {
        const char* fileName = ".DBWrapperTest.testValidateFail.db~";
    remove(fileName);
    FILE* outputLog = fopen(".DBWrapperTest.testValidateFail.log~", "w+");
    saturnin::Logger::getDBLogger().setOutput(outputLog);
    bool contains0, contains1, notContainF;
    {
        saturnin::DBWrapper aWrapper(fileName, true);

        saturnin::Lit clause1[] = {1U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause1, 1));
        
        saturnin::Lit clause0[] = {saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause0, 0));

        aWrapper.sanatize();
        
        contains0 = aWrapper.checkContainsClause(clause0);
        contains1 = aWrapper.checkContainsClause(clause1);
        saturnin::Lit nclause[] = {25U, 3U, saturnin::lit_Undef};
        notContainF = aWrapper.checkContainsClause(nclause);
    }
    fclose(outputLog);
    saturnin::Logger::getDBLogger().setOutput(saturnin::Logger::getStdOutLogger().getOutput());
    CPPUNIT_ASSERT(contains0);
    CPPUNIT_ASSERT(contains1);
    CPPUNIT_ASSERT(!notContainF);
}

void DBWrapperTest::testValidateFail() {
    const char* fileName = ".DBWrapperTest.testValidateFail.db~";
    remove(fileName);
    FILE* outputLog = fopen(".DBWrapperTest.testValidateFail.log~", "w+");
    bool result;
    saturnin::Logger::getDBLogger().setOutput(outputLog);
    {
        saturnin::DBWrapper aWrapper(fileName, true);

        saturnin::Lit clause1[] = {1U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause1, 1));

        saturnin::Lit clause2[] = {7U, 0U, 2U, 4U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(clause2, 2));

        saturnin::Lit res[] = {7U, 2U, 4U, 9U, saturnin::lit_Undef};
        CPPUNIT_ASSERT(aWrapper.addClause(res, 5));
        CPPUNIT_ASSERT(aWrapper.addResolution(clause1, 5));
        CPPUNIT_ASSERT(aWrapper.addResolution(clause2, 5));

        aWrapper.sanatize();
        result = aWrapper.checkResolutions();
    }
    fclose(outputLog);
    saturnin::Logger::getDBLogger().setOutput(saturnin::Logger::getStdOutLogger().getOutput());

    CPPUNIT_ASSERT(!result);
}

#endif /* SATURNIN_DB */

