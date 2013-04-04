#ifndef ATOM_H_
#define ATOM_H_

#include <cassert>
#include <unordered_map>
#include <string>
#include <list>
#include <glpk.h>
#include <vector>

using namespace std;

class Rule;
class Program;
class Component;

class Atom
{
    friend class Component;
    friend class Program;
    friend ostream& operator<<(ostream& out, const Atom& atom);

    friend inline bool operator==(const Atom& a, const Atom& b);
    friend inline bool operator!=(const Atom& a, const Atom& b);
public:
	Atom();
	Atom(Program& program, int id);
	virtual ~Atom();
	
    bool isUnset() const { return data == NULL; }
    bool isConstant() const;
    
    int getId() const;
    const string& getName() const;
    double getLowerBound() const;
    double getUpperBound() const;
    
    void set(Program& program, int id);
    void setName(const string& value);
    
    void setComponent(Component* value);

    void addHeadOccurrence(Rule* rule);
    bool addPositiveBodyOccurrence(Rule* rule);
    bool addNegativeBodyOccurrence(Rule* rule);
    
    bool updateLowerBound(double value);
    bool updateSourcePointer(double value, Rule* sourcePointer);
    bool updateUpperBound(double value);
    bool initConstant();

    bool isInconsistent() const;

    inline const Rule* getSourcePointer() const;

    void buildLinearProgram(glp_prob* linearProgram, vector<int>& matrixRow, vector<int>& matrixCol, vector<double>& matrixVal);
    int getColumnIndexInLinearProgram(glp_prob* linearProgram);
    int getColumnIndexInLinearProgram() const;
    void addToRowBound(int row, double shift);

    inline bool belongsTo(const Component* component) const { assert(data != NULL); return data->component == component; }

private:
    class Data {
    public:
        int id;
        string name;
        
        Program* program;
        Component* component;

        list<Rule*> headOccurrences;
        list<Rule*> positiveBodyOccurrences;
        list<Rule*> negativeBodyOccurrences;
        
        double lowerBound;
        double upperBound;

        int columnInLinearProgram;

        Rule* sourcePointer;
    };
    
    Data* data;
        
    Atom(Data* data);
    void setBoundsForLinearProgram();
};

const Rule* Atom::getSourcePointer() const {
    assert(data != NULL);
    return data->sourcePointer;
}

bool operator==(const Atom& a, const Atom& b) {
    assert(a.data != NULL);
    assert(b.data != NULL);
    return a.data->id == b.data->id;
}

bool operator!=(const Atom& a, const Atom& b) {
    assert(a.data != NULL);
    assert(b.data != NULL);
    return a.data->id != b.data->id;
}

#endif /*ATOM_H_*/
