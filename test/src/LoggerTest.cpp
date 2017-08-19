#define _CRT_SECURE_NO_WARNINGS

#include "LoggerTest.h"
#include "../../include/saturnin/Logger.h"

CPPUNIT_TEST_SUITE_REGISTRATION(LoggerTest);

void LoggerTest::testCreation(){
    saturnin::Logger l;
}

void LoggerTest::testInteger(){
    const char* fileName = ".LoggerTest.int~";
    FILE* f = fopen(fileName, "w+");
    CPPUNIT_ASSERT(f != NULL);
    saturnin::Logger* l = new saturnin::Logger(f);
    *l << 1 << -1;
    delete(l);

    f = fopen(fileName,"r+");
    char buf[5];
    buf[3]=0;
    buf[4]=0;
    unsigned int sz = static_cast<unsigned int>(fread(buf,sizeof(char), 4, f));
    bool isEnd = feof(f) != 0;
    fclose(f);
    CPPUNIT_ASSERT_EQUAL(3U,sz);
    CPPUNIT_ASSERT(isEnd);
    std::string result("1-1");
    CPPUNIT_ASSERT_EQUAL(result, std::string(buf));

}

void LoggerTest::testLong(){
    const char* fileName = ".LoggerTest.long~";
    FILE* f = fopen(fileName, "w+");
    CPPUNIT_ASSERT(f != NULL);
    saturnin::Logger* l = new saturnin::Logger(f);
    long a = 1;
    long b = -1;
    *l << a << b;
    delete(l);

    f = fopen(fileName,"r+");
    char buf[5];
    buf[3]=0;
    buf[4]=0;
    unsigned int sz = static_cast<unsigned int>(fread(buf,sizeof(char), 4, f));
    bool isEnd = feof(f) != 0;
    fclose(f);
    CPPUNIT_ASSERT_EQUAL(3U,sz);
    CPPUNIT_ASSERT(isEnd);
    std::string result("1-1");
    CPPUNIT_ASSERT_EQUAL(result, std::string(buf));
}

void LoggerTest::testUnsingedInteger(){
    const char* fileName = ".LoggerTest.uint~";
    FILE* f = fopen(fileName, "w+");
    CPPUNIT_ASSERT(f != NULL);
    saturnin::Logger* l = new saturnin::Logger(f);
    unsigned int a = 0;
    unsigned int b = 10;
    *l << a << b;
    delete(l);

    f = fopen(fileName,"r+");
    char buf[5];
    buf[3]=0;
    buf[4]=0;
    unsigned int sz = static_cast<unsigned int>(fread(buf,sizeof(char), 4, f));
    bool isEnd = feof(f) != 0;
    fclose(f);
    CPPUNIT_ASSERT_EQUAL(3U,sz);
    CPPUNIT_ASSERT(isEnd);
    std::string result("010");
    CPPUNIT_ASSERT_EQUAL(result, std::string(buf));
}

void LoggerTest::testDouble(){
    const char* fileName = ".LoggerTest.double~";
    FILE* f = fopen(fileName, "w+");
    CPPUNIT_ASSERT(f != NULL);
    saturnin::Logger* l = new saturnin::Logger(f);
    double a = 0.5;
    *l << a;
    delete(l);

    f = fopen(fileName,"r+");
    char buf[10];
    buf[8]=0;
    buf[9]=0;
    unsigned int sz = static_cast<unsigned int>(fread(buf,sizeof(char), 9, f));
    bool isEnd = feof(f) != 0;
    fclose(f);
    CPPUNIT_ASSERT_EQUAL(8U,sz);
    CPPUNIT_ASSERT(isEnd);
    std::string result("0.500000");
    CPPUNIT_ASSERT_EQUAL(result, std::string(buf));
}

void LoggerTest::testConstChar(){
    const char* fileName = ".LoggerTest.char~";
    FILE* f = fopen(fileName, "w+");
    CPPUNIT_ASSERT(f != NULL);
    saturnin::Logger* l = new saturnin::Logger(f);
    const char* a = "SAT";
    *l << a;
    delete(l);

    f = fopen(fileName,"r+");
    char buf[5];
    buf[3]=0;
    buf[4]=0;
    unsigned int sz = static_cast<unsigned int>(fread(buf,sizeof(char), 4, f));
    bool isEnd = feof(f) != 0;
    fclose(f);
    CPPUNIT_ASSERT_EQUAL(3U,sz);
    CPPUNIT_ASSERT(isEnd);
    std::string result("SAT");
    CPPUNIT_ASSERT_EQUAL(result, std::string(buf));
}

void LoggerTest::teststdoutLogger(){
    CPPUNIT_ASSERT_EQUAL(stdout,saturnin::Logger::getStdOutLogger().getOutput());
    CPPUNIT_ASSERT_EQUAL(stderr,saturnin::Logger::getStdErrLogger().getOutput());
}
