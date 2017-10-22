#define _CRT_SECURE_NO_WARNINGS

#include "CNFReaderTest.h"
#include "../../include/saturnin/CNFReader.h"

#include <cstdint>

CPPUNIT_TEST_SUITE_REGISTRATION(CNFReaderTest);

void CNFReaderTest::testCreation() {
    saturnin::CNFReader r("test.cnf");
    CPPUNIT_ASSERT_EQUAL((unsigned int) 0, r.getNbVar());
    CPPUNIT_ASSERT_EQUAL((unsigned int) 0, r.getNbClauses());
    CPPUNIT_ASSERT_EQUAL((unsigned int) 0, r.getMaxLength());
}

void CNFReaderTest::makeTest(unsigned int nbVar, unsigned int nbClauses,
                             const char* fileName, unsigned int nbSectors) {
    FILE* f = fopen(fileName, "w");
    CPPUNIT_ASSERT(f != NULL);

    fprintf(f, "c This file was generated in CNFReaderTest.\n");
    fprintf(f, "c You can delete it if you want, it will be regenerated\n");
    fprintf(f, "p cnf %d %d\n", nbVar, nbClauses);
    for (unsigned int i = 0; i < nbClauses; i++) {
        for (unsigned int j = 1; j <= nbVar; j++) {
            fprintf(f, "%c%d ", (i % 2 == 0 ? ' ' : '-'), j);
        }
        fprintf(f, "0\n");
        //add some blank lines and some comments between
        fprintf(f, "c a comment\n\n");
    }
    fclose(f);

    saturnin::CNFReader r(fileName);

    saturnin::CNFReader::CNFReaderErrors error = r.read(nbSectors);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(saturnin::CNFReader::getErrorString(error),
                                 static_cast<int>(saturnin::CNFReader::CNFReaderErrors::cnfError_noError), static_cast<int>(error));

    CPPUNIT_ASSERT_EQUAL(nbVar, r.getNbVar());
    CPPUNIT_ASSERT_EQUAL(nbClauses, r.getNbClauses());

    for (unsigned int i = 0; i < nbClauses; i++) {
        const saturnin::Array<saturnin::Lit>& c = r.getClause(i);
        CPPUNIT_ASSERT_EQUAL(nbVar, c.getSize());
        for (unsigned int j = 0; j < nbVar; j++) {
            CPPUNIT_ASSERT_EQUAL(saturnin::VariablesManager::getLit(j, i % 2 == 0), c.get(j));
        }
    }
}

void CNFReaderTest::testNoLFAtEnd() {
    const char* fileName = ".CNFReaderTest.noLFatEnd.cnf~";

    FILE* f = fopen(fileName, "w");
    CPPUNIT_ASSERT(f != NULL);

    fprintf(f, "c This file was generated in CNFReaderTest.\n");
    fprintf(f, "c You can delete it if you want, it will be regenerated\n");
    fprintf(f, "p cnf 2 1\n1 -2 0");
    fclose(f);

    saturnin::CNFReader r(fileName);
    saturnin::CNFReader::CNFReaderErrors error = r.read(1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(saturnin::CNFReader::getErrorString(error),
                                 static_cast<int>(saturnin::CNFReader::CNFReaderErrors::cnfError_noError), static_cast<int>(error));

    CPPUNIT_ASSERT_EQUAL((unsigned int) 2, r.getNbVar());
    CPPUNIT_ASSERT_EQUAL((unsigned int) 1, r.getNbClauses());

    const saturnin::Array<saturnin::Lit>& c = r.getClause(0);
    CPPUNIT_ASSERT_EQUAL((unsigned int) 2, c.getSize());
    CPPUNIT_ASSERT_EQUAL((saturnin::Lit)0, c.get(0));
    CPPUNIT_ASSERT_EQUAL((saturnin::Lit)3, c.get(1));

    CPPUNIT_ASSERT_EQUAL(sizeof(int)==sizeof(std::intptr_t) ? (size_t) 7208 :(size_t) 8248, r.getMemoryFootprint());

}

void CNFReaderTest::testNoHeader() {
    const char* fileName = ".CNFReaderTest.testNoHeader.cnf~";

    FILE* f = fopen(fileName, "w");
    CPPUNIT_ASSERT(f != NULL);

    fprintf(f, "c This file was generated in CNFReaderTest.\n");
    fprintf(f, "c You can delete it if you want, it will be regenerated\n");
    fclose(f);

    saturnin::CNFReader r(fileName);
    saturnin::CNFReader::CNFReaderErrors error = r.read(1);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(saturnin::CNFReader::CNFReaderErrors::cnfError_noHeader), static_cast<int>(error));

    CPPUNIT_ASSERT_EQUAL(std::string("We didn't found the header of the file"),
                         std::string(saturnin::CNFReader::getErrorString(saturnin::CNFReader::CNFReaderErrors::cnfError_noHeader)));
}

void CNFReaderTest::testWrongHeader() {
    {
        const char* fileName = ".CNFReaderTest.testWrongHeader.1.cnf~";

        FILE* f = fopen(fileName, "w");
        CPPUNIT_ASSERT(f != NULL);

        fprintf(f, "c This file was generated in CNFReaderTest.\n");
        fprintf(f, "c You can delete it if you want, it will be regenerated\n");
        fprintf(f, "p cnf -1 2\n");
        fclose(f);

        saturnin::CNFReader r(fileName);
        saturnin::CNFReader::CNFReaderErrors error = r.read(1);
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(saturnin::CNFReader::CNFReaderErrors::cnfError_wrongHeader), static_cast<int>(error));
    }

    {
        const char* fileName = ".CNFReaderTest.testWrongHeader.2.cnf~";

        FILE* f = fopen(fileName, "w");
        CPPUNIT_ASSERT(f != NULL);

        fprintf(f, "c This file was generated in CNFReaderTest.\n");
        fprintf(f, "c You can delete it if you want, it will be regenerated\n");
        fprintf(f, "p cnf 1 -2\n");
        fclose(f);

        saturnin::CNFReader r(fileName);
        saturnin::CNFReader::CNFReaderErrors error = r.read(1);
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(saturnin::CNFReader::CNFReaderErrors::cnfError_wrongHeader), static_cast<int>(error));
    }

    CPPUNIT_ASSERT_EQUAL(std::string("The values inside the headers aren't legal (integer overflow or negative value)"),
                         std::string(saturnin::CNFReader::getErrorString(saturnin::CNFReader::CNFReaderErrors::cnfError_wrongHeader)));
}

void CNFReaderTest::testWrongLitValue() {
    {
        const char* fileName = ".CNFReaderTest.testWrongLitValue.1.cnf~";

        FILE* f = fopen(fileName, "w");
        CPPUNIT_ASSERT(f != NULL);

        fprintf(f, "c This file was generated in CNFReaderTest.\n");
        fprintf(f, "c You can delete it if you want, it will be regenerated\n");
        fprintf(f, "p cnf 1 2\n-4 0\n");
        fclose(f);

        saturnin::CNFReader r(fileName);
        saturnin::CNFReader::CNFReaderErrors error = r.read(1);
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(saturnin::CNFReader::CNFReaderErrors::cnfError_wrongLiteralValue), static_cast<int>(error));
    }

    {
        const char* fileName = ".CNFReaderTest.testWrongLitValue.2.cnf~";

        FILE* f = fopen(fileName, "w");
        CPPUNIT_ASSERT(f != NULL);

        fprintf(f, "c This file was generated in CNFReaderTest.\n");
        fprintf(f, "c You can delete it if you want, it will be regenerated\n");
        fprintf(f, "p cnf 1 2\n4294967297 0\n");
        fclose(f);

        saturnin::CNFReader r(fileName);
        saturnin::CNFReader::CNFReaderErrors error = r.read(1);
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(saturnin::CNFReader::CNFReaderErrors::cnfError_wrongLiteralValue), static_cast<int>(error));
    }

    CPPUNIT_ASSERT_EQUAL(std::string("The number of variable doesn't match a variable declaration"),
                         std::string(saturnin::CNFReader::getErrorString(saturnin::CNFReader::CNFReaderErrors::cnfError_wrongLiteralValue)));
}

void CNFReaderTest::testNoZeroAtEnd() {
    {
        const char* fileName = ".CNFReaderTest.testNoZeroAtEnd.1.cnf~";

        FILE* f = fopen(fileName, "w");
        CPPUNIT_ASSERT(f != NULL);

        fprintf(f, "c This file was generated in CNFReaderTest.\n");
        fprintf(f, "c You can delete it if you want, it will be regenerated\n");
        fprintf(f, "p cnf 1 2\n1\n");
        fclose(f);

        saturnin::CNFReader r(fileName);
        saturnin::CNFReader::CNFReaderErrors error = r.read(1);
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(saturnin::CNFReader::CNFReaderErrors::cnfError_noZeroEOL), static_cast<int>(error));
    }

    {
        const char* fileName = ".CNFReaderTest.testNoZeroAtEnd.2.cnf~";

        FILE* f = fopen(fileName, "w");
        CPPUNIT_ASSERT(f != NULL);

        fprintf(f, "c This file was generated in CNFReaderTest.\n");
        fprintf(f, "c You can delete it if you want, it will be regenerated\n");
        fprintf(f, "p cnf 1 2\n1 \n");
        fclose(f);

        saturnin::CNFReader r(fileName);
        saturnin::CNFReader::CNFReaderErrors error = r.read(1);
        CPPUNIT_ASSERT_EQUAL(static_cast<int>(saturnin::CNFReader::CNFReaderErrors::cnfError_noZeroEOL), static_cast<int>(error));
    }

    CPPUNIT_ASSERT_EQUAL(std::string("There was no '0' at the end of a clause"),
                         std::string(saturnin::CNFReader::getErrorString(saturnin::CNFReader::CNFReaderErrors::cnfError_noZeroEOL)));
}

void CNFReaderTest::testWrongFile() {
    const char* fileName = "noSuchFile";

    saturnin::CNFReader r(fileName);
    saturnin::CNFReader::CNFReaderErrors error = r.read(1);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(saturnin::CNFReader::CNFReaderErrors::cnfError_fileReading), static_cast<int>(error));
    CPPUNIT_ASSERT_EQUAL(std::string("There was a problem with the opening of the file"),
                         std::string(saturnin::CNFReader::getErrorString(error)));
}

void CNFReaderTest::testClauseMismatch() {
    const char* fileName = ".CNFReaderTest.testClauseMismatch.cnf~";

    FILE* f = fopen(fileName, "w");
    CPPUNIT_ASSERT(f != NULL);

    fprintf(f, "c This file was generated in CNFReaderTest.\n");
    fprintf(f, "c You can delete it if you want, it will be regenerated\n");
    fprintf(f, "p cnf 1 2\n1 0\n");
    fclose(f);

    saturnin::CNFReader r(fileName);
    saturnin::CNFReader::CNFReaderErrors error = r.read(1);
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(saturnin::CNFReader::CNFReaderErrors::cnfError_wrongNbClauses), static_cast<int>(error));
    CPPUNIT_ASSERT_EQUAL(std::string("The number of clauses present in the file doesn't match with the header"),
                         std::string(saturnin::CNFReader::getErrorString(error)));
}

void CNFReaderTest::testOtherError(){
    CPPUNIT_ASSERT_EQUAL(std::string("Unknown error"),
                         std::string(saturnin::CNFReader::getErrorString( (saturnin::CNFReader::CNFReaderErrors)-1 )));
}

void CNFReaderTest::testSmallFile() {
    makeTest(4, 5, ".CNFReaderTest.smallFile.cnf~");
}

void CNFReaderTest::testHugeLine() {
    makeTest(1 << 18, 1, ".CNFReaderTest.hugeLine.cnf~");
}

void CNFReaderTest::testHugeFile() {
    makeTest(10, 1 << 10, ".CNFReaderTest.hugeFile.cnf~", 2);
}
