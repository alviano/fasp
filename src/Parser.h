#ifndef PARSER_H_
#define PARSER_H_

#include <iostream>
#include <string>

using namespace std;

#include "Program.h"

class Parser
{
public:
	Parser(Program& program, istream& in);
	virtual ~Parser();
	
	void parse();
	
private:
    Program& program;
    istream& in;
    
    void parseProgram();
    void parseRule();
    void parseAtoms();
};

#endif /*PARSER_H_*/
