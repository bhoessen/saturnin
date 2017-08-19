//remove the 'assert' checks for this test
#ifndef NDEBUG
#define NDEBUG
#endif
#ifndef DEBUG
#define DEBUG
#endif
#include "AssertTest.h"
#include "../../include/saturnin/Assert.h"
#include <sstream>
CPPUNIT_TEST_SUITE_REGISTRATION(AssertTest);


using namespace saturnin;

void AssertTest::testEquality() {
    {
        int a = 1;
        int b = 1;
        std::stringstream ststream;
        checkEquality(a,b,"a", "b","src/AssertTest.cpp", __LINE__, ststream);
        ststream.flush();
        std::string output(ststream.str());
        CPPUNIT_ASSERT_EQUAL(std::string(""),output);
    }

    {
        int a = 1;
        int b = 2;
        std::stringstream ststream;
        checkEquality(a,b,"a", "b","src/AssertTest.cpp", 26, ststream);
        ststream.flush();
        std::string output(ststream.str());
        std::string expected("a == b is not respected\n"
            "Expected: 1\n"
            "Actual:   2\n"
#if !defined (SUNSTUDIO) && !defined(_WIN32)
            "In src/AssertTest.cpp:26\n"
            "#0 void saturnin::checkEquality");
#else
            );
#endif
        size_t count =
#if !defined (SUNSTUDIO) && !defined(_WIN32)
            104;
#else
            48;
#endif
        CPPUNIT_ASSERT_EQUAL(expected,output.substr(0,count));
    }

}

void AssertTest::testTruth() {

    {
        std::stringstream ststream;
        checkTruth(true,"true", "src/AssertTest.cpp", __LINE__, ststream);
        ststream.flush();
        std::string output(ststream.str());
        CPPUNIT_ASSERT_EQUAL(std::string(""), output);
    }

    {
        std::stringstream ststream;
        checkTruth(false,"false", "src/AssertTest.cpp", 54, ststream);
        ststream.flush();
        std::string output(ststream.str());
        std::string expected("false is not respected\n"
#if !defined (SUNSTUDIO) && !defined(_WIN32)
            "In src/AssertTest.cpp:54\n"
            "#0 void saturnin::checkTruth");
#else
            );
#endif
        size_t count =
#if !defined (SUNSTUDIO) && !defined(_WIN32)
            76;
#else
            23;
#endif
        CPPUNIT_ASSERT_EQUAL(expected, output.substr(0, count));
    }

}
