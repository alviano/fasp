/*
 * DependencyGraph.cpp
 *
 *  Created on: Mar 30, 2013
 *      Author: malvi
 */

#include "DependencyGraph.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/strong_components.hpp>


static boost::adjacency_list<> graph;

DependencyGraph::DependencyGraph() {
}

DependencyGraph::~DependencyGraph() {
}

void DependencyGraph::addEdge(int a, int b) {
    if(a == b)
        selfLoops.push_back(a);
    boost::add_edge(a, b, graph);
}

int DependencyGraph::computeSCC(vector<int>*& component) {
    assert(component == NULL);

    // DISABLED
    component = new vector<int>(boost::num_vertices(graph));
    for(unsigned i = 0; i < boost::num_vertices(graph); i++)
        (*component)[i] = 0;
    return 1;

    vector<int> discover_time(boost::num_vertices(graph));
    vector<boost::default_color_type> color(boost::num_vertices(graph));
    vector<boost::graph_traits<boost::adjacency_list<> >::vertex_descriptor> root(boost::num_vertices(graph));
    int res = boost::strong_components(graph, &(*component)[0], boost::root_map(&root[0]).color_map(&color[0]).discover_time_map(&discover_time[0]));
    graph.clear();
    return res;
}
