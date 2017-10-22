#define _CRT_SECURE_NO_WARNINGS 

#include "saturnin/CNFReader.h"
#include "saturnin/VariablesManager.h"
#include <errno.h>
#include <ctype.h>
#include <stdint.h>
#include <limits>

#include "./FileReader.h"
#include "../Utils.h"

#define SAT_ABS(x) ((x)>0?(x):-(x))


using namespace saturnin;

const char* cnfError_noError_str = "No error";
const char* cnfError_noZeroEOL_str =
        "There was no '0' at the end of a clause";
const char* cnfError_fileReading_str =
        "There was a problem with the opening of the file";
const char* cnfError_noHeader_str =
        "We didn't found the header of the file";
const char* cnfError_wrongHeader_str =
        "The values inside the headers aren't legal "
        "(integer overflow or negative value)";
const char* cnfError_wrongLiteralValue_str =
        "The number of variable doesn't match a variable declaration";
const char* cnfError_wrongNbClauses_str =
        "The number of clauses present in the file doesn't match with the header";
const char* cnfError_unknwown = "Unknown error";

const char* CNFReader::getErrorString(CNFReader::CNFReaderErrors error) {
    switch (error) {
        case CNFReaderErrors::cnfError_noError: return cnfError_noError_str;
        case CNFReaderErrors::cnfError_noZeroEOL: return cnfError_noZeroEOL_str;
        case CNFReaderErrors::cnfError_fileReading: return cnfError_fileReading_str;
        case CNFReaderErrors::cnfError_noHeader: return cnfError_noHeader_str;
        case CNFReaderErrors::cnfError_wrongHeader: return cnfError_wrongHeader_str;
        case CNFReaderErrors::cnfError_wrongLiteralValue: return cnfError_wrongLiteralValue_str;
        case CNFReaderErrors::cnfError_wrongNbClauses: return cnfError_wrongNbClauses_str;
        default: return cnfError_unknwown;
    }
}

CNFReader::CNFReader(const char* fileName) : inputFile(nullptr), clauses(256), 
        nbVar(0), memoryToRead(sizeof(FileReader)), presumptions(256), 
        lengthSum(0), maxLength(0) {
    inputFile = fopen(fileName, "r");
}

CNFReader::~CNFReader() {
    if (inputFile != nullptr) {
        fclose(inputFile);
    }
}

CNFReader::CNFReaderErrors CNFReader::read(unsigned int nbBlocks) {
    if (inputFile == nullptr) {
        return CNFReaderErrors::cnfError_fileReading;
    }
    FileReader reader(inputFile, nbBlocks);

    CNFReaderErrors error = CNFReaderErrors::cnfError_noError;
    //Read comments and the header
    int64_t nbClauses = 0;
    bool headerFound = false;
    do {
        const char* line;
        unsigned int lineLength = reader.getNextLine(line);
        const char* endOfLine = line + lineLength;
        if (lineLength > 0 && line[0] != 'c') {
            //we need a 'p cnf' line
            if (lineLength < 5 || strncmp(line, "p cnf", 5) != 0) {
                error = CNFReaderErrors::cnfError_noHeader;
            } else {
                line += 5;
                const char* tmp;
                int nbRead = 0;
                int64_t value = strntol(line, &tmp, endOfLine - line, nbRead);
                if (value < 0 || nbRead == 0 || (value == 0) || value > static_cast<int64_t>(std::numeric_limits<unsigned int>::max())) {
                    error = CNFReaderErrors::cnfError_wrongHeader;
                } else {
                    nbVar = (unsigned int) value;
                }

                line = tmp;
                nbRead = 0;
                value = strntol(line, &tmp, endOfLine - line, nbRead);
                if (value < 0 || nbRead == 0 || (value == 0) || value > (int64_t) std::numeric_limits<unsigned int>::max()) {
                    error = CNFReaderErrors::cnfError_wrongHeader;
                } else {
                    nbClauses = value;
                }

                headerFound = true;

            }
        }
    } while (!headerFound && error == CNFReaderErrors::cnfError_noError && !reader.isEOF());

    if (error != CNFReaderErrors::cnfError_noError) {
        return error;
    }else if (!headerFound){
        return CNFReaderErrors::cnfError_noHeader;
    }

    //create initial place for a clause of size 4 in order to 
    //reduce the total memory used
    Array<Lit> currentClause(4);
    do {
        const char* line;
        unsigned int lineLength = reader.getNextLine(line);
        const char* endOfLine = line + lineLength;
        //make sure the next line isn't empty nor is a comment
        if (lineLength > 0 && line[0] != 'c') {
            ASSERT(currentClause.getSize() == 0U);
            const char* end = line + lineLength;
            bool zeroFound = false;
            while (error == CNFReaderErrors::cnfError_noError && line < end) {
                const char* tmp;
                int nbRead = 0;
                int64_t value = strntol(line, &tmp, endOfLine - line, nbRead);
                if (SAT_ABS(value) > (int64_t)std::numeric_limits<unsigned int>::max()) {
                    //we avoid an integer overflow
                    error = CNFReaderErrors::cnfError_wrongLiteralValue;
                    line = end;
                } else if (nbRead == 0){
                    line = end;
                } else if (value == 0) {
                    zeroFound = true;
                    line = end;
                } else {
                    bool sign = value > 0 ? true : false;
                    Var v = static_cast<Var>(SAT_ABS(value) - 1);
                    if (v > nbVar) {
                        error = CNFReaderErrors::cnfError_wrongLiteralValue;
                    } else {
                        Lit a = VariablesManager::getLit(v, sign);
                        currentClause.push(a);
                    }
                    line = tmp;
                }
            }
            if (currentClause.getSize() > 0) {
                if (!zeroFound) {
                    error = CNFReaderErrors::cnfError_noZeroEOL;
                }
                if (error == CNFReaderErrors::cnfError_noError) {
                    unsigned int size = currentClause.getSize();
                    clauses.push(std::move(currentClause));
                    lengthSum += size;
                    if(size > maxLength) {
                        maxLength = size;
                    }
                }
            }
        }else if (lineLength > 2 && line[0] == 'c' && line[1] == 'p') {
            line += 2;
            const char * tmp;
            int nbRead = 0;
            int64_t value = strntol(line, &tmp, endOfLine - line, nbRead);
            if (SAT_ABS(value) > (int64_t) std::numeric_limits<unsigned int>::max()) {
                //we avoid an integer overflow
                error = CNFReaderErrors::cnfError_wrongLiteralValue;
            } else if (nbRead != 0 && value != 0) {
                {
                    bool sign = value > 0 ? true : false;
                    Var v = static_cast<Var>(SAT_ABS(value) - 1);
                    if (v > nbVar) {
                        error = CNFReaderErrors::cnfError_wrongLiteralValue;
                    } else {
                        Lit a = VariablesManager::getLit(v, sign);
                        presumptions.push(a);
                    }
                    line = tmp;
                }
            }
        }
    } while (error == CNFReaderErrors::cnfError_noError && !reader.isEOF());
    
    memoryToRead += reader.getMemoryFootprint();

    if (error == CNFReaderErrors::cnfError_noError && nbClauses != (long) clauses.getSize()) {
        return CNFReaderErrors::cnfError_wrongNbClauses;
    }

    return error;
}

unsigned int CNFReader::getNbClauses() const {
    return clauses.getSize();
}

unsigned int CNFReader::getNbVar() const {
    return nbVar;
}

const Array<Lit>& CNFReader::getClause(unsigned int i) const {
    ASSERT(i < clauses.getSize());
    return ((const Array<Lit>*)clauses)[i];
}

size_t CNFReader::getMemoryFootprint() const {
    size_t mem = clauses.getMemoryFootprint() + memoryToRead;
    for (unsigned int i = 0; i < clauses.getSize(); i++) {
        mem += clauses[i].getMemoryFootprint();
    }
    return mem;
}
