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
    return out << (atom.isProcessedConstant() ? atom.getName().c_str() + 1 : atom.getName());
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

bool Atom::isProcessedConstant() const {
    assert(data != NULL);
    return data->name[1] == '#';
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

double Atom::getBoundsDifference() const {
    assert(data != NULL);
    return data->upperBound - data->lowerBound > EPSILON ? data->upperBound - data->lowerBound : 0.0;
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
    this->data->columnInBilevelProgram = 0;
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
    assert(rule->getBodySize() > 0);
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
    return data->upperBound - data->lowerBound < -EPSILON;
}

bool Atom::updateLowerBound(double value) {
    assert(data != NULL);
    assert(0 <= value && value <= 1);
    if(value > data->lowerBound) {
        trace(std, 4, "Updating lower bound of %s from %g to %g\n", getName().c_str(), data->lowerBound, value);

        if(isProcessedConstant())
            return false;

        Program::getInstance().addAssignment(*this, data->lowerBound);
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

bool Atom::propagateLowerBound() {
    assert(data != NULL);
    trace(std, 4, "Propagating lower bound of %s\n", getName().c_str());

    setBoundsForLinearProgram();

    for(list<Rule*>::iterator it = data->positiveBodyOccurrences.begin(); it != data->positiveBodyOccurrences.end(); ++it)
        if(!(**it).onIncreaseLowerBound())
            return false;
    for(list<Rule*>::iterator it = data->negativeBodyOccurrences.begin(); it != data->negativeBodyOccurrences.end(); ++it)
        if(!(**it).onDecreaseUpperBound())
            return false;
    return true;
}

bool Atom::updateSourcePointer(double value, Rule* sourcePointer) {
    assert(data != NULL);
    assert(0 <= value && value <= 1);
    if(value > data->upperBound) {
        trace(std, 4, "Updating upper bound of %s from %g to %g\n", getName().c_str(), data->upperBound, value);

        if(isProcessedConstant()) {
            trace(std, 4, " Skip constant\n");
            return true;
        }

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
        assert(!isProcessedConstant());
        trace(std, 4, "Updating upper bound of %s from %g to %g\n", getName().c_str(), data->upperBound, value);
        Program::getInstance().addAssignment(*this, 10 + data->upperBound);
        data->upperBound = value;
    }
    return true;
}

bool Atom::propagateUpperBound() {
    assert(data != NULL);
    assert(!isProcessedConstant());
    trace(std, 4, "Propagating upper bound of %s\n", getName().c_str());

    for(list<Rule*>::iterator it = data->positiveBodyOccurrences.begin(); it != data->positiveBodyOccurrences.end(); ++it)
        if(!(**it).onDecreaseUpperBound())
            return false;
    for(list<Rule*>::iterator it = data->negativeBodyOccurrences.begin(); it != data->negativeBodyOccurrences.end(); ++it)
        if(!(**it).onIncreaseLowerBound())
            return false;

    if(!isConstant()) {
        if(!findSourcePointer())
            return false;
        assert(!isInconsistent());
        setBoundsForLinearProgram();
    }
    return true;
}

bool Atom::split() {
    assert(getBoundsDifference() > 0.0);
    assert(!isProcessedConstant());
    trace(std, 2, "Split on atom %s\n", getName().c_str());
    if(!updateLowerBound(getLowerBound() + getBoundsDifference()/2))
        return false;
    if(!Program::getInstance().propagate())
        return false;
    return Program::getInstance().computeFuzzyAnswerSet2();
}

bool Atom::split2() {
    assert(getBoundsDifference() > 0.0);
    assert(!isProcessedConstant());
    trace(std, 2, "Split2 on atom %s\n", getName().c_str());
    if(!updateUpperBound(getLowerBound() + getBoundsDifference()/2))
        return false;
    if(!Program::getInstance().propagate())
        return false;
    return Program::getInstance().computeFuzzyAnswerSet2();
}

void Atom::initConstant() {
    assert(isConstant());
    assert(!isProcessedConstant());

    data->lowerBound = data->upperBound = parseConstantDegree();

    // mark as processed constant
    setName(string("#") + getName());
}

bool Atom::processConstant() {
    assert(isConstant());
    assert(isProcessedConstant());

    for(list<Rule*>::iterator it = data->positiveBodyOccurrences.begin(); it != data->positiveBodyOccurrences.end(); ++it)
        if(!(**it).onIncreaseUpperBound())
            return false;
    for(list<Rule*>::iterator it = data->positiveBodyOccurrences.begin(); it != data->positiveBodyOccurrences.end(); ++it)
        if(!(**it).onIncreaseLowerBound())
            return false;
    for(list<Rule*>::iterator it = data->negativeBodyOccurrences.begin(); it != data->negativeBodyOccurrences.end(); ++it)
        if(!(**it).onDecreaseUpperBound() || !(**it).onIncreaseLowerBound())
            return false;

    for(list<Rule*>::iterator rule = data->positiveBodyOccurrences.begin(); rule != data->positiveBodyOccurrences.end(); ++rule)
        (**rule).addToRowBound(-getLowerBound());
    for(list<Rule*>::iterator rule = data->negativeBodyOccurrences.begin(); rule != data->negativeBodyOccurrences.end(); ++rule)
        (**rule).addToRowBound(getLowerBound());

    return true;
}

void Atom::parseBoundsForConstant() {
    assert(isConstant());

    data->lowerBound = data->upperBound = parseConstantDegree();
}

double Atom::parseConstantDegree() const {
    assert(isConstant());
    assert(!isProcessedConstant());

    double degree = -1;
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
        assert(ss.eof());
    }
    assert(0 <= degree && degree <= 1);
    return degree;
}

bool Atom::initSourcePointer() {
    assert(data != NULL);
    if(data->sourcePointer == NULL) {
        trace(std, 3, "Atom %s has no source pointers\n", getName().c_str());
        assert(data->upperBound == 0);

        /*for(list<Rule*>::iterator it = data->positiveBodyOccurrences.begin(); it != data->positiveBodyOccurrences.end(); ++it)
            if(!(**it).onDecreaseUpperBound())
                return false;*/
        for(list<Rule*>::iterator it = data->negativeBodyOccurrences.begin(); it != data->negativeBodyOccurrences.end(); ++it)
            if(!(**it).onIncreaseLowerBound())
                return false;
    }

    return true;
}

bool Atom::findSourcePointer() {
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
    return updateSourcePointer(max, sourcePointer);
}

bool Atom::checkConstant() const {
    assert(isConstant());
    assert(isProcessedConstant());
    for(list<Rule*>::const_iterator it = data->headOccurrences.begin(); it != data->headOccurrences.end(); ++it) {
        const Rule* rule = *it;
        double ub = rule->computeBodyLowerBound();
        if(ub > getLowerBound())
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

int Atom::getColumnIndexInBilevelProgram(int& nextIdInBilevelProgram) {
    assert(data != NULL);
    if(data->columnInBilevelProgram == 0)
        data->columnInBilevelProgram = nextIdInBilevelProgram++;
    return data->columnInBilevelProgram;
}

void Atom::unrollLowerBound(double value) {
    trace(std, 3, "Unrolling lower bound of %s to %g\n", getName().c_str(), value);
    data->lowerBound = value;
    setBoundsForLinearProgram();
}

void Atom::unrollUpperBound(double value) {
    trace(std, 3, "Unrolling upper bound of %s to %g\n", getName().c_str(), value);
    data->upperBound = value;
    setBoundsForLinearProgram();
}