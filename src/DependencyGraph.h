/*
 * DependencyGraph.h
 *
 *  Created on: Mar 30, 2013
 *      Author: malvi
 */

#ifndef DEPENDENCYGRAPH_H_
#define DEPENDENCYGRAPH_H_

#include <list>
#include <vector>

using namespace std;

class DependencyGraph {
    friend class Program;
public:
    DependencyGraph();
    virtual ~DependencyGraph();

    void addEdge(int a, int b);
    int computeSCC(vector<int>*& component);

private:
    list<int> selfLoops;
};

#endif /* DEPENDENCYGRAPH_H_ */
