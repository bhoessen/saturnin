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

#ifndef FILEREADERTEST_H
#define	FILEREADERTEST_H

#include <cppunit/extensions/HelperMacros.h>

class CNFReaderTest : public CppUnit::TestFixture {
public:

    CPPUNIT_TEST_SUITE(CNFReaderTest);
    CPPUNIT_TEST(testCreation);
    CPPUNIT_TEST(testSmallFile);
    CPPUNIT_TEST(testHugeLine);
    CPPUNIT_TEST(testHugeFile);
    CPPUNIT_TEST(testNoLFAtEnd);
    CPPUNIT_TEST(testNoHeader);
    CPPUNIT_TEST(testWrongHeader);
    CPPUNIT_TEST(testWrongLitValue);
    CPPUNIT_TEST(testNoZeroAtEnd);
    CPPUNIT_TEST(testWrongFile);
    CPPUNIT_TEST(testClauseMismatch);
    CPPUNIT_TEST(testOtherError);
    CPPUNIT_TEST_SUITE_END();
    
    void testCreation();
    
    void testSmallFile();
    
    void testHugeLine();
    
    void testHugeFile();
    
    void testNoLFAtEnd();
    
    void testNoHeader();
    
    void testWrongHeader();
    
    void testWrongLitValue();
    
    void testNoZeroAtEnd();
    
    void testWrongFile();
    
    void testClauseMismatch();
    
    void testOtherError();
    
    void makeTest(unsigned int nbVar, unsigned int nbClauses, const char* fileName, unsigned int nbSectors = 256);

};


#endif	/* FILEREADERTEST_H */

