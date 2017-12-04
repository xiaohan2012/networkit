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
#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../graph/Graph.h"

namespace NetworKit {

class CoreMaximizationTest: public testing::Test {
public:
	virtual void SetUp();

protected:
	Graph G;
	std::vector< std::pair<node, node> > edges;
};

} /* namespace NetworKit */

#endif /* COMPUTECOREGTEST_H_ */

#endif /* NOGTEST */
