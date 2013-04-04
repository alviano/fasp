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

/**
 * This structure contains a field for each command-line option.
 *
 * There is also a global instance of the structure, named options; probably,
 * the only instance in the program.
 */
struct Options
{
    Options();
    ~Options();

    void parse(int argc, char* const* argv);
    void printHelp() const;
    void setTnorm(const char* str);
    void setGlpkTermOut();

    Tnorm* tnorm;

#ifndef TRACE_OFF
    TraceLevels traceLevels;
#endif

};

extern struct Options __options__;

#endif /* OPTIONS_H_ */
