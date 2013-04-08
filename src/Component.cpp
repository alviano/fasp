/*
 * Component.cpp
 *
 *  Created on: Mar 30, 2013
 *      Author: malvi
 */

#include "Component.h"

#include "Rule.h"
#include "trace.h"

#include <algorithm>
#include <cassert>
#include <unordered_set>

ostream& operator<<(ostream& out, const Component& component) {
    for(Component::const_iterator it = component.begin(); it != component.end(); ++it)
        out << *it << " ";
    if(component.isRecursive())
        out << "(recursive)";
    return out;
}

Component::Component() : _size(0), recursive(false), linearProgram(NULL), changedBounds(true) {
}

Component::~Component() {
    for(list<Atom>::iterator it = begin(); it != end(); ++it)
        it->setComponent(NULL);

    if(linearProgram != NULL) {
        glp_delete_prob(linearProgram);
        linearProgram = NULL;
    }
}

void Component::add(Atom atom) {
    assert(find(begin(), end(), atom) == end());
    list<Atom>::push_back(atom);
    atom.setComponent(this);
    if(++_size > 1)
        setRecursive();
}

void Component::setBounds(Atom atom) {
    glp_set_col_bnds(linearProgram, atom.getColumnIndexInLinearProgram(linearProgram), atom.getLowerBound() != atom.getUpperBound() ? GLP_DB : GLP_FX, atom.getLowerBound(), atom.getUpperBound());
    changedBounds = true;
}

void Component::addToRowBound(int row, double shift) {
    glp_set_row_bnds(linearProgram, row, GLP_UP, 0.0, glp_get_row_ub(linearProgram, row) + shift);
    changedBounds = true;
}

void Component::initLinearProgram() {
    assert(linearProgram == NULL);
    linearProgram = glp_create_prob();

    vector<int> matrixRow;//[1+1000];
    vector<int> matrixCol;//[1+1000];
    vector<double> matrixVal;//[1+1000];

    // add a fake cell
    matrixRow.push_back(0);
    matrixCol.push_back(0);
    matrixVal.push_back(0);

    for(iterator it = begin(); it != end(); ++it)
        it->buildLinearProgram(linearProgram, matrixRow, matrixCol, matrixVal);

    glp_load_matrix(linearProgram, matrixRow.size() - 1, matrixRow.data(), matrixCol.data(), matrixVal.data());
}

bool Component::updateLowerBoundsByLinearProgram() {
    assert(changedBounds);
    changedBounds = false;

    unordered_set<int> done;
    for(iterator it = begin(); it != end(); ++it) {
        Atom& atom = *it;
        if(done.find(atom.getId()) != done.end())
            continue;

        trace(std, 3, "Processing inequalities for %s\n", atom.getName().c_str());
        assert(glp_get_obj_coef(linearProgram, atom.getColumnIndexInLinearProgram()) == 0);

        glp_set_obj_coef(linearProgram, atom.getColumnIndexInLinearProgram(), 1);
        glp_simplex(linearProgram, NULL);

        const_iterator it2 = it;
        while(++it2 != end()) {
            if(glp_get_col_prim(linearProgram, it2->getColumnIndexInLinearProgram()) <= it2->getLowerBound())
                done.insert(it2->getId());
        }

        if(glp_get_obj_val(linearProgram) <= atom.getLowerBound()) {
            glp_set_obj_coef(linearProgram, atom.getColumnIndexInLinearProgram(), 0);
            continue;
        }

        glp_exact(linearProgram, NULL);
        double lb(glp_get_obj_val(linearProgram));
        //cout << "LINEAR PROGRAM: " << atom << " " << lb << " was " << atom.getLowerBound() << endl;
        glp_set_obj_coef(linearProgram, atom.getColumnIndexInLinearProgram(), 0);
        if(!atom.updateLowerBound(lb))
            return false;
    }

    return true;
}

bool Component::hasOddCycles() const {
    assert(isRecursive());

    Atom a = front();

    list<Atom> stack;
    stack.push_back(a);

    unordered_map<int, int> label;
    label[a.getId()] = 1;

    while(!stack.empty()) {
        Atom a = stack.back();
        stack.pop_back();
        int aLabel = label[a.getId()];
        for(list<Rule*>::const_iterator it = a.data->headOccurrences.begin(); it != a.data->headOccurrences.end(); ++it) {
            Rule& rule = **it;
            for(int i = 0; i < rule.getBodySize(); ++i) {
                Atom b = rule.getBodyAtom(i);
                if(!b.belongsTo(this))
                    continue;
                int bId = b.getId();
                pair<unordered_map<int,int>::iterator, bool> res = label.insert(unordered_map<int, int>::value_type(bId, 0));
                if(res.second)
                    stack.push_back(b);

                label[bId] |= rule.isNegative(i) ? 3 - aLabel : aLabel;
                if(label[bId] == 3)
                    return true;
            }
        }
    }
    return false;
}
