/*
 * GreedyCommunityExpansion.h
 *
 *  Created on: 03.06.2013
 *  Author: cls, Yassine Marrakchi
 */

#ifndef GREEDYCOMMUNITYEXPANSION_H_
#define GREEDYCOMMUNITYEXPANSION_H_

#include <unordered_set>

#include "SelectiveCommunityDetector.h"
#include "CommunityTrimming.h"
#include "QualityObjective.h"
#include "Acceptability.h"


namespace NetworKit {


/**
 * The Greedy Community Expansion algorithm.
 *
 * Greedily adds nodes from the shell to improve community quality.
 */
class GreedyCommunityExpansion: public NetworKit::SelectiveCommunityDetector {

public:

	GreedyCommunityExpansion(const Graph& G, Acceptability& similarity, QualityObjective& objective, CommunityTrimming& trimming);

	GreedyCommunityExpansion(const Graph& G, std::string acceptability, std::string objective, std::string trimming);


	void run(std::set<unsigned int>& seeds) override;

	/**
	 * @param[in]	s	seed node
	 *
	 * @param[out]		community as a set of nodes
	 */
	std::unordered_set<node> expandSeed(node s);

protected:

	Acceptability* similarity;
	QualityObjective* objective;
	CommunityTrimming* trimming;

	virtual double clusterClusterSimilarity (std::unordered_set<node>& community1, std::unordered_set<node>& community2);


};

} /* namespace NetworKit */
#endif /* GREEDYCOMMUNITYEXPANSION_H_ */



