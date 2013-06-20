/*
 * options.cpp
 *
 *  Created on: Apr 4, 2013
 *      Author: malvi
 */

#include "options.h"

#include <cassert>
#include <cstring>
#include <cstdlib>
#include <getopt.h>
#include <glpk.h>


#include "GodelTnorm.h"
#include "LukasiewiczTnorm.h"
#include "ProductTnorm.h"

struct Options __options__;

// Options with a short alias must use ascii codes.
// Here are all the short/long options ordered by code.
#define OPTIONID_help 'h'

// Options without a short alias must use non-ascii codes.
// Here are all the "only-long" options ordered by code.
#define OPTIONID_trace_std ('z' + 1)
#define OPTIONID_tnorm ('z' + 2)
#define OPTIONID_glpk_term_out ('z' + 3)
#define OPTIONID_mode ('z' + 4)
#define OPTIONID_octave_term_out ('z' + 5)

Options::Options() {
    glp_term_out(GLP_OFF);
    tnorm = new LukasiewiczTnorm();
    mode = ALL_APPROXIMATIONS;
    octaveTermOut = false;
}

Options::~Options() {
    assert(tnorm != NULL);
    delete tnorm;
}

void Options::parse(int argc, char* const* argv) {
    // It will store the option code.
    int code;

    do{
        // For each option there is an instance of option.
        // Each option specifies
        // - the long name;
        // - if it requires an argument (required_argument), accept an argument
        //   (optional_argument), or does not allow arguments (no_argument);
        // - the address of an integer flag to be setted followed by the value
        //   to use, or NULL followed by the code of the option.
        //
        // Here are all the long options ordered by long name.
        static struct option longOptions[] =
            {
                { "help", no_argument, NULL, OPTIONID_help },
                { "tnorm", required_argument, NULL, OPTIONID_tnorm },
                { "mode", required_argument, NULL, OPTIONID_mode },
                #ifndef TRACE_OFF
                { "trace-std", required_argument, NULL, OPTIONID_trace_std },
                { "glpk-term-out", no_argument, NULL, OPTIONID_glpk_term_out },
                { "octave-term-out", no_argument, NULL, OPTIONID_octave_term_out },
                #endif

                // The NULL-option indicates the end of the array.
                { NULL, 0, NULL, 0 }
            };

        // The function getopt_long() stores the option index here.
        int optionIndex = 0;

        // The third argument specifies all the short options.
        code = getopt_long(argc, argv, "hn:01", longOptions, &optionIndex);

        switch(code)
        {
        case -1:
            // All the command line was parsed.
            break;

        case OPTIONID_help:
            printHelp();
            exit(0);
            break;

        case OPTIONID_trace_std:
            setTraceLevel(std, atoi(optarg));
            break;

        case OPTIONID_tnorm:
            setTnorm(optarg);
            break;

        case OPTIONID_glpk_term_out:
            setGlpkTermOut();
            break;

        case OPTIONID_mode:
            setBilevelProgram(optarg);
            break;

        case OPTIONID_octave_term_out:
            setOctaveTermOut();
            break;

        default:
            cerr << "ERROR: Invalid command-line option: " << argv[optionIndex] << endl;
            exit(-1);
            break;
        }
    } while(code != -1);

    // TODO: Manage the reminder options.
    for(int i = optind; i < argc; i++) {
        cerr << "WARNING: Ignoring unknown argument: " << argv[i] << endl;
        cerr << "\tPrograms are read from STDIN" << endl;
    }
}

void Options::printHelp() const {
    cout << "FASP v1.0\n";
    cout << "\nusage: fasp [command-line options]";
    cout << "\n\tThe input program is read from STDIN\n";
    cout << "\ncommand-line options";
    cout << "\n\t-h, --help\t\tprints the help and exit";
    cout << "\n\t    --tnorm=VALUE\tsets the t-norm: VALUE can be lukasiewicz (default), godel or product";
    cout << "\n\t    --mode=VALUE: VALUE can be"
         << "\n\t\tall-approximations (default): computes all approximation operators"
         << "\n\t\twell-founded: computes the well-founded semantics"
         << "\n\t\tanswer-set: computes one fuzzy answer set after all approximation operators"
         << "\n\t\tanswer-set-unoptimized: computes one fuzzy answer set without computing approximation operators";
    #ifndef NDEBUG
    cout << "\n\t    --trace-std=N\tsets standard trace level to N";
    cout << "\n\t    --glpk-term-out\tactivate the terminal output of GLPK";
    cout << "\n\t    --octave-term-out\tactivate the terminal output of Octave";
    #endif
    cout << endl;
}

void Options::setTnorm(const char* str) {
    assert(tnorm != NULL);
    delete tnorm;
    if(strcmp(str, "lukasiewicz") == 0)
        tnorm = new LukasiewiczTnorm();
    else if(strcmp(str, "godel") == 0)
        tnorm = new GodelTnorm();
    else if(strcmp(str, "product") == 0)
        tnorm = new ProductTnorm();
    else {
        tnorm = new LukasiewiczTnorm();
        cerr << "ERROR: Invalid tnorm: " << str << endl;
        exit(-1);
    }
}

void Options::setGlpkTermOut() {
    glp_term_out(GLP_ON);
}

void Options::setBilevelProgram(const char* str) {
    if(strcmp(str, "well-founded") == 0)
        mode = WELL_FOUNDED;
    else if(strcmp(str, "all-approximations") == 0)
        mode = ALL_APPROXIMATIONS;
    else if(strcmp(str, "answer-set") == 0)
        mode = ANSWER_SET;
    else if(strcmp(str, "answer-set-unoptimized") == 0)
        mode = ANSWER_SET_UNOPTIMIZED;
    else {
        cerr << "ERROR: Invalid option for --well-founded. Should be well-founded, all-approximations, answer-set or answer-set-unoptimized." << endl;
        exit(-1);
    }
}

void Options::setOctaveTermOut() {
    octaveTermOut = true;
}
