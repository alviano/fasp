#include "Atom.h"

#include "Program.h"
#include "trace.h"
#include "Component.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>

using namespace std;

ostream& operator<<(ostream& out, const Atom& atom) {
    return out << atom.getName();
}

Atom::Atom() : data(NULL)
{
}

Atom::Atom(Program& program, int _id) : data(NULL) {
    set(program, _id);
}

Atom::Atom(Data* _data) : data(_data) {
    assert(data != NULL);
}

Atom::~Atom()
{
    data = NULL;
}

bool Atom::isConstant() const {
    assert(data != NULL);
    return data->name[0] == '#';
}

int Atom::getId() const {
    assert(data != NULL);
    return data->id;
}

const string& Atom::getName() const {
    assert(data != NULL);
    return data->name;
}

double Atom::getLowerBound() const {
    assert(data != NULL);
    return data->lowerBound;
}

double Atom::getUpperBound() const {
    assert(data != NULL);
    return data->upperBound;
}

void Atom::set(Program& program, int id) {
    assert(data == NULL);
    pair<unordered_map<int, Data*>::iterator, bool> res = program.atoms.insert(unordered_map<int, Data*>::value_type(id, NULL));
    if(!res.second) {
        this->data = res.first->second;
        return;
    }
    
    this->data = res.first->second = new Data();
    program.atomList.push_back(res.first->second);

    this->data->id = id;
    
    stringstream ss;
    ss << "@" << id;
    this->data->name = ss.str();
    
    this->data->program = &program;
    this->data->component = NULL;

    this->data->lowerBound = 0;
    this->data->upperBound = 0;

    this->data->columnInLinearProgram = 0;
}

void Atom::setName(const string& value) {
    assert(data != NULL);
    data->name = value;
}

void Atom::setComponent(Component* value) {
    assert(data != NULL);
    data->component = value;
}

void Atom::addHeadOccurrence(Rule* rule) {
    assert(rule != NULL);
    assert(data != NULL);
    data->headOccurrences.push_back(rule);
}

bool Atom::addPositiveBodyOccurrence(Rule* rule) {
    assert(rule != NULL);
    assert(data != NULL);
    if(data->positiveBodyOccurrences.back() == rule)
        return false;
    data->positiveBodyOccurrences.push_back(rule);
    return true;
}

bool Atom::addNegativeBodyOccurrence(Rule* rule) {
    assert(rule != NULL);
    assert(data != NULL);
    if(!data->negativeBodyOccurrences.empty() && data->negativeBodyOccurrences.back() == rule)
        return false;
    data->negativeBodyOccurrences.push_back(rule);
    return true;
}

bool Atom::isInconsistent() const {
    return data->upperBound < data->lowerBound;
}

bool Atom::updateLowerBound(double value) {
    assert(data != NULL);
    assert(0 <= value && value <= 1);
    if(value > data->lowerBound) {
        trace(std, 4, "Updating lower bound of %s from %g to %g\n", getName().c_str(), data->lowerBound, value);
        data->lowerBound = value;
        if(isInconsistent())
            return false;

        setBoundsForLinearProgram();

        for(list<Rule*>::iterator it = data->positiveBodyOccurrences.begin(); it != data->positiveBodyOccurrences.end(); ++it)
            if(!(**it).onIncreaseLowerBound())
                return false;
        for(list<Rule*>::iterator it = data->negativeBodyOccurrences.begin(); it != data->negativeBodyOccurrences.end(); ++it)
            if(!(**it).onDecreaseUpperBound())
                return false;
    }
    return true;
}

bool Atom::updateSourcePointer(double value, Rule* sourcePointer) {
    assert(data != NULL);
    assert(0 <= value && value <= 1);
    if(value > data->upperBound) {
        trace(std, 4, "Updating upper bound of %s from %g to %g\n", getName().c_str(), data->upperBound, value);
        data->upperBound = value;
        data->sourcePointer = sourcePointer;
        if(isInconsistent())
            return false;

        setBoundsForLinearProgram();

        for(list<Rule*>::iterator it = data->positiveBodyOccurrences.begin(); it != data->positiveBodyOccurrences.end(); ++it)
            if(!(**it).onIncreaseUpperBound())
                return false;
    }
    return true;
}

bool Atom::updateUpperBound(double value) {
    assert(data != NULL);
    assert(0 <= value && value <= 1);
    if(value < data->upperBound) {
        trace(std, 4, "Updating upper bound of %s from %g to %g\n", getName().c_str(), data->upperBound, value);
        data->upperBound = value;
        if(isInconsistent())
            return false;

        setBoundsForLinearProgram();

        for(list<Rule*>::iterator it = data->positiveBodyOccurrences.begin(); it != data->positiveBodyOccurrences.end(); ++it)
            if(!(**it).onDecreaseUpperBound())
                return false;
        for(list<Rule*>::iterator it = data->negativeBodyOccurrences.begin(); it != data->negativeBodyOccurrences.end(); ++it)
            if(!(**it).onIncreaseLowerBound())
                return false;

        double max = 0.0;
        Rule* sourcePointer = NULL;
        for(list<Rule*>::iterator it = data->headOccurrences.begin(); it != data->headOccurrences.end(); ++it) {
            Rule* rule = *it;
            double ub = rule->computeBodyUpperBound();
            if(ub > max) {
                max = ub;
                sourcePointer = rule;
            }
        }
        if(!updateSourcePointer(max, sourcePointer))
            return false;
    }
    return true;
}

bool Atom::initConstant() {
    assert(getName()[0] == '#');

    double degree;
    stringstream ss(getName().c_str()+1);
    ss >> degree;
    if(!ss.eof()) {
        char c;
        ss >> c;
        assert(c == '/');
        double denominator;
        ss >> denominator;
        assert(denominator > 0);
        degree /= denominator;
    }
    assert(0 <= degree && degree <= 1);

    if(!updateSourcePointer(1, NULL) || !updateUpperBound(degree) || !updateLowerBound(getUpperBound()))
        return false;

    for(list<Rule*>::iterator rule = data->positiveBodyOccurrences.begin(); rule != data->positiveBodyOccurrences.end(); ++rule)
        (**rule).addToRowBound(-getLowerBound());
    for(list<Rule*>::iterator rule = data->negativeBodyOccurrences.begin(); rule != data->negativeBodyOccurrences.end(); ++rule)
        (**rule).addToRowBound(getLowerBound());
    return true;
}

bool Atom::initSourcePointer() {
    assert(data != NULL);
    if(data->sourcePointer == NULL) {
        assert(data->upperBound == 0);
        trace(std, 3, "Atom %s has no source pointers\n", getName().c_str());

        /*for(list<Rule*>::iterator it = data->positiveBodyOccurrences.begin(); it != data->positiveBodyOccurrences.end(); ++it)
            if(!(**it).onDecreaseUpperBound())
                return false;*/
        for(list<Rule*>::iterator it = data->negativeBodyOccurrences.begin(); it != data->negativeBodyOccurrences.end(); ++it)
            if(!(**it).onIncreaseLowerBound())
                return false;
    }

    return true;
}

void Atom::buildLinearProgram(glp_prob* linearProgram, vector<int>& matrixRow, vector<int>& matrixCol, vector<double>& matrixVal) {
    assert(data != NULL);

    glp_set_obj_coef(linearProgram, getColumnIndexInLinearProgram(linearProgram), 0);
    setBoundsForLinearProgram();
    for(list<Rule*>::iterator it = data->headOccurrences.begin(); it != data->headOccurrences.end(); ++it) {
        Rule& rule = **it;
        rule.buildLinearProgram(linearProgram, matrixRow, matrixCol, matrixVal);
    }
}

int Atom::getColumnIndexInLinearProgram(glp_prob* linearProgram) {
    assert(data != NULL);
    if(data->columnInLinearProgram != 0)
        return data->columnInLinearProgram;
    glp_add_cols(linearProgram, 1);
    return data->columnInLinearProgram = glp_get_num_cols(linearProgram);
}

int Atom::getColumnIndexInLinearProgram() const {
    assert(data != NULL);
    assert(data->columnInLinearProgram != 0);
    return data->columnInLinearProgram;
}

void Atom::setBoundsForLinearProgram() {
    assert(data != NULL);
    if(data->component != NULL)
        data->component->setBounds(*this);
}

void Atom::addToRowBound(int row, double shift) {
    assert(data != NULL);
    if(data->component != NULL)
        data->component->addToRowBound(row, shift);
}
