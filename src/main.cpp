#include "options.h"
#include "Parser.h"

int main(int argc, char** argv) {
    __options__.parse(argc, argv);

    Program program(*__options__.tnorm);
    Parser parser(program, cin);
    parser.parse();
    trace(std, 5, "Parsed program:\n%s\n", program.toString().c_str());
    
    program.initInterpretation();

    if(hasTraceLevel(std, 5))
        program.printSourcePointers(cerr);

    program.printInterpretation(cout);

    return 0;
}
