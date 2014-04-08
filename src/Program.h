#ifndef PROGRAM_H_
#define PROGRAM_H_

#include <iostream>
#include <list>
#include <glpk.h>

#include "Component.h"
#include "DependencyGraph.h"
#include "Rule.h"
#include "Tnorm.h"

using namespace std;

class Program
{
    friend ostream& operator<<(ostream& out, const Program& program);
    friend class Atom;
public:
	Program(const Tnorm& tnorm);
	virtual ~Program();
	
	void insert(Rule* rule);
	
	void printInterpretation(ostream& out) const;
	void printSourcePointers(ostream& out) const;
	
    void initInterpretation();
    void setNaiveBounds();
    
    const Tnorm& getTnorm() const { return *tnorm; }

    Atom getFalseAtom() { return Atom(*this, 1); }

    void onInconsistency();

    void addDependency(Atom head, Atom body);

    string toString() const;

    void computeFuzzyAnswerSet();
    bool computeFuzzyAnswerSet2();
    void printBilevelProgram(ostream& out);

    bool isInchoerent() const { return incoherent; }

    inline void addAssignment(Atom atom, double bound) { assignments.push_back(pair<Atom, double>(atom, bound)); }

    bool propagate();

    static void init(const Tnorm& tnorm);
    static void free();
    static inline Program& getInstance() { assert(instance != NULL); return *instance; }

private:
    static Program* instance;
    
    list<Rule*> rules;
    unordered_map<int, Atom::Data*> atoms;
    list<Atom::Data*> atomList;
    list<Atom::Data*> constants;
    Tnorm* tnorm;
    DependencyGraph dependencyGraph;
    list<Component*> components;
    int nextIdInBilevelProgram;
    bool incoherent;
    
    vector<pair<Atom, double> > assignments;
    unsigned processed;

    void printSCC(ostream& out) const;
    void computeSCC();
};

#endif /*PROGRAM_H_*/
