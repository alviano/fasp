#include "Rule.h"

#include <iostream>
#include "Program.h"
#include "options.h"
#include <cmath>

ostream& operator<<(ostream& out, const Rule& rule) {
    out << rule.getHead();
    bool first = true;
    for(int i = 0; i < rule.getBodySize(); ++i) {
        if(first) {
            out << " :- ";
            first = false;
        }
        else
            out << ", ";
        out << (i < rule.negativeLiterals ? "not " : "") << rule.getBodyAtom(i);
    }
    return out << ".";
}

Rule::Rule(Program& _program, const Atom& _head, int _bodySize, int _negativeLiterals) : program(_program), head(_head), bodySize(_bodySize), negativeLiterals(_negativeLiterals)
{
    assert(negativeLiterals >= 0);
    assert(bodySize >= negativeLiterals);
    if(bodySize == 0) {
        bodySize = negativeLiterals = 1;
        body = new Atom[bodySize];
        setBodyAtom(0, program.getFalseAtom());
    }
    else
        body = new Atom[bodySize];
    head.addHeadOccurrence(this);
}

Rule::~Rule()
{
    assert(body != NULL);
    delete[] body;
}

void Rule::setBodyAtom(int idx, const Atom& value) {
    assert(0 <= idx && idx < bodySize);
    assert(body[idx].isUnset());
    body[idx] = value;
    if(idx < negativeLiterals)
        body[idx].addNegativeBodyOccurrence(this);
    else
        body[idx].addPositiveBodyOccurrence(this);
}

double Rule::computeBodyLowerBound() const {
    double res = 1;
    for(int i = 0; i < bodySize; ++i) {
        res = program.getTnorm().conjunction(res, getLiteralLowerBound(i));
    }
    assert(0 <= res && res <= 1);
    return res;
}

double Rule::computeBodyUpperBound() const {
    double res = 1;
    for(int i = 0; i < bodySize; ++i) {
        res = program.getTnorm().conjunction(res, getLiteralUpperBound(i));
    }
    assert(0 <= res && res <= 1);
    return res;
}

double Rule::getLiteralLowerBound(int idx) const {
    assert(0 <= idx && idx < bodySize);
    if(idx < negativeLiterals)
        return program.getTnorm().negation(body[idx].getUpperBound());
    return body[idx].getLowerBound();
}

double Rule::getLiteralUpperBound(int idx) const {
    assert(0 <= idx && idx < bodySize);
    if(idx < negativeLiterals)
        return program.getTnorm().negation(body[idx].getLowerBound());
    return body[idx].getUpperBound();
}

bool Rule::onIncreaseLowerBound() {
    return head.updateLowerBound(computeBodyLowerBound());
}

bool Rule::onIncreaseUpperBound() {
    return head.updateSourcePointer(computeBodyUpperBound(), this);
}

bool Rule::onDecreaseUpperBound() {
    if(head.getSourcePointer() == this)
        return head.updateUpperBound(computeBodyUpperBound());
    return true;
}

void Rule::buildLinearProgram(glp_prob* linearProgram, vector<int>& matrixRow, vector<int>& matrixCol, vector<double>& matrixVal) {
    assert(linearProgram != NULL);
    glp_add_rows(linearProgram, 1);
    rowInLinearProgram = glp_get_num_rows(linearProgram);

    double rowBound = getPositiveBodySize() - 1;

    unordered_map<int, int> added;

    added[head.getColumnIndexInLinearProgram()] = matrixRow.size();
    matrixRow.push_back(rowInLinearProgram);
    matrixCol.push_back(head.getColumnIndexInLinearProgram());
    matrixVal.push_back(-1);

    for(int i = 0; i < bodySize; ++i) {
        if(body[i].isConstant())
            continue;

        int idx = body[i].getColumnIndexInLinearProgram(linearProgram);
        unordered_map<int, int>::iterator it = added.find(idx);
        if(it == added.end()) {
            added[idx] = matrixRow.size();
            matrixRow.push_back(rowInLinearProgram);
            matrixCol.push_back(idx);
            matrixVal.push_back(i < negativeLiterals ? -1 : 1);
        }
        else {
            assert(it->second < static_cast<int>(matrixVal.size()));
            matrixVal[it->second] += i < negativeLiterals ? -1 : 1;
        }
    }

    glp_set_row_bnds(linearProgram, rowInLinearProgram, GLP_UP, 0.0, rowBound);
}

void Rule::addToRowBound(double shift) {
    head.addToRowBound(rowInLinearProgram, shift);
}

void Rule::printBilevelProgram(ostream& out, int& nextIdInBilevelProgram) {
    if(!head.isConstant() && fabs(head.getLowerBound() - head.getUpperBound()) < EPSILON)
        return;

    printBilevelProgram(out, nextIdInBilevelProgram, head, false);
    out << " >=";
    if(bodySize > 0) {
        printBilevelProgram(out, nextIdInBilevelProgram, body[0], 0 < negativeLiterals);
        for(int i = 1; i < bodySize; ++i)
            printBilevelProgram(out, nextIdInBilevelProgram, body[i], i < negativeLiterals);
        if(bodySize > 1)
            out << " - " << (bodySize-1);
        out << "," << endl;
    }
    else
        out << "1," << endl;
}

void Rule::printBilevelProgram(ostream& out, int& nextIdInBilevelProgram, Atom& atom, bool negative) {
    out << (negative ? " + 1 - " : " + ");
    if(atom.isConstant() || fabs(atom.getLowerBound() - atom.getUpperBound()) < EPSILON)
        out << atom.getLowerBound();
    else
        out << (negative ? "o(" : "i(") << atom.getColumnIndexInBilevelProgram(nextIdInBilevelProgram) << ")";
}
