/*
 * GraphGTest.h
 *
 *  Created on: 01.06.2014
 *      Author: Klara Reichard (klara.reichard@gmail.com), Marvin Ritter (marvin.ritter@gmail.com)
 */

#ifndef NOGTEST

#ifndef COMPUTECOREGTEST_H_
#define COMPUTECOREGTEST_H_

#include <tuple>
#include <vector>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../graph/Graph.h"

namespace NetworKit {

class ComputeCoreGTest: public testing::TestWithParam< std::tuple<bool, bool> > {
public:
	virtual void SetUp();

protected:
	Graph G;
	std::vector< std::pair<node, node> > edges;
	std::vector< std::vector<edgeweight> > Ahouse;
	count n_nodes;
	count n_edges;

	bool isGraph() const { return !isWeighted() && !isDirected(); }
	bool isWeightedGraph() const { return isWeighted() && !isDirected(); }
	bool isDirectedGraph() const { return !isWeighted() && isDirected(); }
	bool isWeightedDirectedGraph() const { return isWeighted() && isDirected(); }


	bool isWeighted() const;
	bool isDirected() const;
	Graph createGraph(count n = 0) const;
	count countSelfLoopsManually(const Graph &G);
};

} /* namespace NetworKit */

#endif /* COMPUTECOREGTEST_H_ */

#endif /* NOGTEST */
