/*
Copyright (c) <2015> <B.Hoessen>

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

#ifndef DBWRAPPERTEST_H
#define DBWRAPPERTEST_H

#ifdef SATURNIN_DB
#include <cppunit/extensions/HelperMacros.h>

class DBWrapperTest : public CppUnit::TestFixture {
public:

  CPPUNIT_TEST_SUITE(DBWrapperTest);
  CPPUNIT_TEST(testCreation);
  CPPUNIT_TEST(testInsertResolventClause);
  CPPUNIT_TEST(testInsertClause);
  CPPUNIT_TEST(testSanitize);
  CPPUNIT_TEST(testOpenWrongFile);
  CPPUNIT_TEST(testComputeHeights);
  CPPUNIT_TEST(testMemoryFootprint);
  CPPUNIT_TEST(testOpenExistingDB);
  CPPUNIT_TEST(testPrepareFailingStmt);
  CPPUNIT_TEST(testValidate);
  CPPUNIT_TEST(testValidateFail);
  CPPUNIT_TEST(testContainsClause);
  CPPUNIT_TEST_SUITE_END();

  void testCreation();
  void testInsertResolventClause();
  void testInsertClause();
  void testSanitize();
  void testOpenWrongFile();
  void testComputeHeights();
  void testMemoryFootprint();
  void testOpenExistingDB();
  void testPrepareFailingStmt();
  void testValidate();
  void testValidateFail();
  void testContainsClause();
  
private:
  
  /**
   * Check the content of a database
   * @param fileName the name of the file containing the sqlite database
   * @param query the query to perform
   * @param nbColumns the expected number of columns
   * @param values the expected values
   * @return SQLITE_OK if everything went well
   */
  int validateResults(const char* fileName, const char* query, int nbColumns, const char** values);

};

#endif /* SATURNIN_DB */
#endif /* DBWRAPPERTEST_H */