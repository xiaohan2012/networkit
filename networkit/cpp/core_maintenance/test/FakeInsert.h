/*
 * GraphGTest.h
 *
 *  Created on: 01.06.2014
 *      Author: Klara Reichard (klara.reichard@gmail.com), Marvin Ritter (marvin.ritter@gmail.com)
 */

#ifndef NOGTEST

#ifndef FAKEINSERTGTEST_H_
#define FAKEINSERTTEST_H_

#include <tuple>
#include <vector>
#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../glist.h"
#include "../../graph/Graph.h"

namespace NetworKit {

class FakeInsertTest: public testing::Test {
public:
	virtual void SetUp();

protected:
	Graph G;
	std::vector< std::pair<node, node> > edges;
	core::GLIST *glist_ptr;
	std::vector<index> core;

	std::vector<core::GLIST::CoreComponent> nc_list;
	std::vector<index> nc_ids;	
};

} /* namespace NetworKit */

#endif 

#endif /* NOGTEST */
