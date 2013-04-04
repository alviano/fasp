#include "Parser.h"

#include <cassert>

Parser::Parser(Program& _program, istream& _in) : program(_program), in(_in)
{
}

Parser::~Parser()
{
}

void Parser::parse() {
    parseProgram();
    parseAtoms();
}

void Parser::parseProgram() {
    int id;
    while(true) {
        in >> id;
        switch(id) {
        case 0:
            return;
            
        case 1:
            parseRule();
            break;
            
        default:
            cerr << "Parsing error." << endl;
            return;
        }
    }
}

void Parser::parseRule() {
    int headAtom;
    int bodySize;
    int negativeLiterals;
    in >> headAtom >> bodySize >> negativeLiterals;
    
    assert(headAtom > 0);
    assert(bodySize >= negativeLiterals);
    assert(negativeLiterals >= 0);
    
    Rule* rule = new Rule(program, Atom(program, headAtom), bodySize, negativeLiterals);
    int id;
    for(int i = 0; i < bodySize; ++i) {
        in >> id;
        assert(id > 0);
        rule->setBodyAtom(i, Atom(program, id));
    }
    
    program.insert(rule);
}

void Parser::parseAtoms() {
    int id;
    char buff[1024];
    
    while(true) {
        in >> id;
        switch(id) {
        case 0:
            return;
            
        default:
            assert(id > 0);
            in.getline(buff, 1024);
            Atom atom(program, id);
            assert(buff[0] = ' ');
            atom.setName(string(buff+1));
            break;
        }
    }
}
