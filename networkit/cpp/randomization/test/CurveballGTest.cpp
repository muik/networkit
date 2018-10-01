/*
 * CurveballGTest.cpp
 *
 *  Created on: Jul 12, 2017
 *	Author: Author: Hung Tran <htran@ae.cs.uni-frankfurt.de>
 */

#include <gtest/gtest.h>
#include "../../graph/Graph.h"

#include "../Curveball.h"

#include "../../graph/Graph.h"
#include "../../Globals.h"
#include "../../generators/ErdosRenyiGenerator.h"
#include "../../generators/HyperbolicGenerator.h"
#include "../../auxiliary/Random.h"
#include "../CurveballUniformTradeGenerator.h"

namespace NetworKit {

class CurveballGTest : public testing::Test  {
protected:
    void checkWithGraph(NetworKit::Graph&, bool checkBuilder = false);
};


void CurveballGTest::checkWithGraph(NetworKit::Graph& G, bool checkBuilder) {
    node numNodes = G.numberOfNodes();
    const count numTrades = 5;
    const NetworKit::count numTradeRuns = 5;


    std::vector<count> degrees(numNodes + 1);

    // Add edge to node 0, if isolated node
    // If 0 itself is isolated, add new node and connect 0 to it
    G.forNodes([&](node u) {
        if (G.degree(u) > 0)
            degrees[u] = G.degree(u);
        else {
            if (u == 0) {
                numNodes++;
                G.addEdge(0, numNodes - 1);
                degrees[0]++;
                degrees[numNodes - 1] = 1;
            } else {
                G.addEdge(u, 0);
                degrees[0]++;
                degrees[u] = 1;
            }
        }
    });


    NetworKit::Curveball algo(G);
    for (NetworKit::count tradeRun = 0; tradeRun < numTradeRuns; tradeRun++) {
        CurveballUniformTradeGenerator gen(numTrades, numNodes);
        algo.run(gen.generate());
    }

    // check degrees
    NetworKit::Graph outG = algo.getGraph(false); // sequential
    outG.forNodes([&](node u){
        ASSERT_EQ(degrees[u], outG.degree(u));
    });

	// check builder: parallel is equal to sequential
	if (checkBuilder) {
		NetworKit::Graph outGpar = algo.getGraph(true);

		// check degrees
		outGpar.forNodes([&](node u){
			ASSERT_EQ(degrees[u], outGpar.degree(u));
		});

		// check equality of neighbours for each node
		outGpar.forNodes([&](node u){
			std::vector<node> par_neighbors;
			par_neighbors.reserve(outGpar.degree(u));

			std::vector<node> seq_neighbors;
			seq_neighbors.reserve(outG.degree(u));

			outGpar.forNeighborsOf(u, [&](node v) {
				par_neighbors.push_back(v);
			});

			outG.forNeighborsOf(u, [&](node v) {
				seq_neighbors.push_back(v);
			});

			ASSERT_EQ(par_neighbors.size(), seq_neighbors.size());
			ASSERT_TRUE(std::equal(par_neighbors.begin(),
								   par_neighbors.begin() + par_neighbors.size(),
								   seq_neighbors.begin()));
		});
	}
}


TEST_F(CurveballGTest, testCurveballErdosRenyi) {
    Aux::Random::setSeed(1, false);

    node numNodes = 1000;
    NetworKit::ErdosRenyiGenerator generator(numNodes, 0.3);
    NetworKit::Graph G = generator.generate();

    this->checkWithGraph(G, false);
}

TEST_F(CurveballGTest, testCurveballHyperbolic) {
    Aux::Random::setSeed(1, false);

    node numNodes = 1000;
    NetworKit::HyperbolicGenerator generator(numNodes);
    NetworKit::Graph G = generator.generate();

    this->checkWithGraph(G, false);
}

TEST_F(CurveballGTest, testCurveballMaterialization) {
	Aux::Random::setSeed(1, false);

	node numNodes = 500;
	NetworKit::ErdosRenyiGenerator generator(numNodes, 0.3);
	NetworKit::Graph G = generator.generate();

	this->checkWithGraph(G, true);
}

}

