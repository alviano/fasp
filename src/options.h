/*
 * options.h
 *
 *  Created on: Apr 4, 2013
 *      Author: malvi
 */

#ifndef OPTIONS_H_
#define OPTIONS_H_

#include <iostream>
#include <vector>
#include <unordered_set>
#include <string>

#include "trace.h"
#include "Tnorm.h"

using namespace std;

#define EPSILON .0000000001


/**
 * This structure contains a field for each command-line option.
 *
 * There is also a global instance of the structure, named options; probably,
 * the only instance in the program.
 */
struct Options
{
    enum Mode { WELL_FOUNDED = 0, ALL_APPROXIMATIONS, ANSWER_SET, ANSWER_SET_UNOPTIMIZED };

    Options();
    ~Options();

    void parse(int argc, char* const* argv);
    void printHelp() const;
    void setTnorm(const char* str);
    void setGlpkTermOut();
    void setBilevelProgram(const char* str);
    void setOctaveTermOut();

    Tnorm* tnorm;
    Mode mode;
    bool octaveTermOut;

#ifndef TRACE_OFF
    TraceLevels traceLevels;
#endif

};

extern struct Options __options__;

#endif /* OPTIONS_H_ */
