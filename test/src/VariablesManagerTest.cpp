#include "VariablesManagerTest.h"
#include "../../include/saturnin/VariablesManager.h"



CPPUNIT_TEST_SUITE_REGISTRATION(VariablesManagerTest);

void VariablesManagerTest::testGetVar() {
    for (unsigned int i = 0; i < 4; i++) {
        CPPUNIT_ASSERT_EQUAL(i / 2, saturnin::VariablesManager::getVar(i));
    }
}

void VariablesManagerTest::testGetLit() {
    for (unsigned int i = 0; i < 4; i++) {
        CPPUNIT_ASSERT_EQUAL(i, saturnin::VariablesManager::getLit(i / 2, i % 2 == 0));
    }
}

void VariablesManagerTest::testGetLitSign() {
    for (unsigned int i = 0; i < 4; i++) {
        CPPUNIT_ASSERT_EQUAL(i % 2 == 0, saturnin::VariablesManager::getLitSign(i));
    }
}

void VariablesManagerTest::testOppositeLit() {
    for (unsigned int i = 0; i < 4; i++) {
        CPPUNIT_ASSERT_EQUAL(saturnin::VariablesManager::getLit(i / 2, i % 2 == 1), saturnin::VariablesManager::oppositeLit(i));
    }
}

void VariablesManagerTest::testToDimacs() {
    CPPUNIT_ASSERT_EQUAL( 1, saturnin::VariablesManager::toDimacs(saturnin::VariablesManager::getLit(0, true)));
    CPPUNIT_ASSERT_EQUAL( -1, saturnin::VariablesManager::toDimacs(saturnin::VariablesManager::getLit(0, false)));
}