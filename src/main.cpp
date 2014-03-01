#include "options.h"
#include "Parser.h"

int main(int argc, char** argv) {
    __options__.parse(argc, argv);

    Program program(*__options__.tnorm);
    Parser parser(program, cin);
    parser.parse();
    trace(std, 5, "Parsed program:\n%s\n", program.toString().c_str());
    
    if(__options__.mode != Options::ANSWER_SET_UNOPTIMIZED) {
        program.initInterpretation();

        if(hasTraceLevel(std, 5))
            program.printSourcePointers(cerr);
    }
    else
        program.setNaiveBounds();

    if(program.isInchoerent())
       return 0;

    if(__options__.mode == Options::WELL_FOUNDED || __options__.mode == __options__.ALL_APPROXIMATIONS)
        program.printInterpretation(cout);
    else
        program.computeFuzzyAnswerSet();

    return 0;
}
