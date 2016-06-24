#include <iostream>
#include <vector>
#include "vx.hpp"
#include "ProfileData.h"
#include "MyNode.h"
#include "Experiment.h"
#define N_TIMES 100
#define SCHEDULING_LOG_NAME "scheduling_logs.txt"
#define CSV_SPEEDUP_NAME "scheduling_speedup.csv"
#define CSV_CLUSTER_NAME "scheduling_cluster.csv"
#define NS_TO_MS (1000*1000*1.0)

//Scheduling method
#define PRIORITY_C 0
#define PRIORITY_OPENCL 1
#define PRIORITY_HEXAGON 2
#define COARSEN_SCHEDULING 3
using namespace cv;
using namespace OpenVX;

int main(int argc, char **argv)
{
	logs(SCHEDULING_LOG_NAME) << argv[0] << " start..." << endl;
	Context context;
	context.selfTest();
	logs() << std::endl;
	ProfileData profileData;
	fstream csvSpeedup(CSV_SPEEDUP_NAME, std::fstream::out);
	fstream csvCluster(CSV_CLUSTER_NAME, std::fstream::out);

	srand(time(NULL));
	Experiment *experiments[] = 
	{
		new SuperResolution(context),
		new RandomFaceDetection(context),
		new RandomCase1(context),
		new RandomCase2(context),
	};
	for (int n_exp = 0; n_exp < sizeof(experiments) / sizeof(experiments[0]); n_exp++)
	{
		Experiment *thisExperiment = experiments[n_exp];
		csvSpeedup << thisExperiment->name << ",";
		csvCluster << thisExperiment->name << ",";
		logs() << thisExperiment->name << std::endl;
		for (int method = 0; method < 4; method++)
		{
			Graph graph(context);

			vector<vx_kernel_e> kernel_es;
			vector<MyNode *> nodes;
			thisExperiment->prepareNodesAndDatas(graph, kernel_es, nodes);

			switch (method)
			{
			case PRIORITY_C:
				MyNode::priority(TARGET_C_MODEL, graph, kernel_es, nodes);
				break;
			case PRIORITY_OPENCL:
				MyNode::priority(TARGET_OPENCL, graph, kernel_es, nodes);
				break;
			case PRIORITY_HEXAGON:
				MyNode::priority(TARGET_HEXAGON, graph, kernel_es, nodes);
				break;
			case COARSEN_SCHEDULING:
				MyNode::nodeCoarsen(graph, profileData, kernel_es, nodes);
				MyNode::clusterToCSV(csvCluster, nodes.size());
				break;
			}

			if (graph.verify())
			{
				Graph *mGraph = &graph;
				int n_times = N_TIMES;

				vx_perf_t graphPref, firstTimePerfG;

				mGraph->process();
				firstTimePerfG = mGraph->getPerformance();

				for (int i = 0; i < n_times; i++)
					mGraph->process();

				logs().precision(2);
				logs() << std::fixed;

				graphPref = mGraph->getPerformance();
				logs() << "   "
					<< "first: " << firstTimePerfG.tmp / NS_TO_MS << " ms "
					<< "min: " << graphPref.min / NS_TO_MS << " ms "
					<< "max: " << graphPref.max / NS_TO_MS << " ms "
					<< "avg: " << (graphPref.sum - firstTimePerfG.sum) / (n_times*NS_TO_MS) << " ms " << std::endl;
				csvSpeedup << (graphPref.sum - firstTimePerfG.sum) / (n_times*NS_TO_MS) << ",";
			}

			thisExperiment->releaseDatas();
			MyNode::releaseNodes(graph, nodes);
		}
		csvSpeedup << std::endl;
	}
	
	for (int n_exp = 0; n_exp < sizeof(experiments) / sizeof(experiments[0]); n_exp++)
		delete experiments[n_exp];

	csvSpeedup.close();
	csvCluster.close();
	logs() << argv[0] << " done!!" << endl;
	logs().close();
	return 0;
}