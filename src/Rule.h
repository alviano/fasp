#ifndef RULE_H_
#define RULE_H_

#include <cassert>
#include <glpk.h>

#include "Atom.h"

class Program;

class Rule
{
    friend ostream& operator<<(ostream& out, const Rule& rule);
public:
	Rule(Program& program, const Atom& head, int bodySize, int negativeLiterals);
	virtual ~Rule();
	
	const Atom& getHead() const { return head; }
	int getBodySize() const { return bodySize; }
	int getPositiveBodySize() const { return bodySize - negativeLiterals; }
	int getNegativeBodySize() const { return negativeLiterals; }
	inline const Atom& getBodyAtom(int idx) const;
	inline bool isPositive(int idx) const { return !isNegative(idx); }
	inline bool isNegative(int idx) const { return idx < negativeLiterals; }
	
	void setBodyAtom(int idx, const Atom& value);
	
	double computeBodyLowerBound() const;
	double computeBodyUpperBound() const;
	
    bool onIncreaseLowerBound();
    bool onDecreaseLowerBound();
    bool onIncreaseUpperBound();
    bool onDecreaseUpperBound();

    void buildLinearProgram(glp_prob* linearProgram, vector<int>& matrixRow, vector<int>& matrixCol, vector<double>& matrixVal);
    void addToRowBound(double shift);

    void printBilevelProgram(ostream& out, int& nextIdInBilevelProgram);

private:
    Program& program;
    Atom head;
    Atom* body;
    int bodySize;
    int negativeLiterals;
    int rowInLinearProgram;
    
    double getLiteralLowerBound(int idx) const;
    double getLiteralUpperBound(int idx) const;

    void printBilevelProgram(ostream& out, int& nextIdInBilevelProgram, Atom& atom, bool negative);
};

const Atom& Rule::getBodyAtom(int idx) const {
    assert(0 <= idx && idx < bodySize);
    return body[idx];
}

#endif /*RULE_H_*/
