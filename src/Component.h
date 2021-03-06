/*
 * Component.h
 *
 *  Created on: Mar 30, 2013
 *      Author: malvi
 */

#ifndef COMPONENT_H_
#define COMPONENT_H_

#include "Atom.h"
#include <list>
#include <iostream>
#include <glpk.h>

using namespace std;

class Component : private list<Atom> {
    friend ostream& operator<<(ostream& out, const Component& component);
public:
    typedef list<Atom>::iterator iterator;
    typedef list<Atom>::const_iterator const_iterator;
    typedef list<Atom>::reverse_iterator reverse_iterator;
    typedef list<Atom>::const_reverse_iterator const_reverse_iterator;

    Component();
    ~Component();

    int size() const { return _size; }
    bool isRecursive() const { return true;/*recursive;*/ }
    bool hasOddCycles() const;
    bool hasChangedBounds() const { return changedBounds; }

    void setRecursive() { recursive = true; }

    void add(Atom atom);

    void setBounds(Atom atom);
    void addToRowBound(int row, double shift);

    void initLinearProgram();
    bool updateLowerBoundsByLinearProgram();

    using list<Atom>::begin;
    using list<Atom>::end;
    using list<Atom>::rbegin;
    using list<Atom>::rend;
    using list<Atom>::empty;
    using list<Atom>::front;
    using list<Atom>::back;

private:
    int _size;
    bool recursive;
    glp_prob* linearProgram;
    bool changedBounds;

    string matrixToString(int size, int row[], int col[], double val[]) const;
};

#endif /* COMPONENT_H_ */
