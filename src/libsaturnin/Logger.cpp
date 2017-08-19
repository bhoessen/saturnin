#include "saturnin/Logger.h"


using namespace saturnin;

Logger mainLogger;
Logger errLogger(stderr);
Logger dbLogger;

Logger::Logger(FILE* output) : stream(output) {
}

Logger::Logger(const Logger& source) : stream(source.stream){
}

Logger::~Logger(){
    if(stream != stdout && stream!= stderr){
        if(fclose(stream)){
            fprintf(stderr, "Unable to close a logger!");
        }
    }
}

void Logger::setOutput(FILE* output) {
    stream = output;
}

FILE* Logger::getOutput() {
    return stream;
}

Logger& Logger::getStdOutLogger() {
    return mainLogger;
}

Logger& Logger::getStdErrLogger(){
    return errLogger;
}

Logger& Logger::getDBLogger() {
    return dbLogger;
}

Logger& operator<<(saturnin::Logger& l, double d){
    fprintf(l.getOutput(), "%f",d);
    return l;
}

Logger& operator<<(Logger& l, int i){
    fprintf(l.getOutput(),"%d",i);
    return l;
}

Logger& operator<<(Logger& l, const char* i){
    fprintf(l.getOutput(),"%s",i);
    return l;
}

saturnin::Logger& operator<<(saturnin::Logger& l, long i){
    fprintf(l.getOutput(),"%ld",i);
    return l;
}

saturnin::Logger& operator<<(saturnin::Logger& l, unsigned int i){
    fprintf(l.getOutput(),"%d",i);
    return l;
}
