#include "MyNode.h"
#include "ProfileData.h"
#include "DataObject.h"
#include "RandomNodes.h"
#include <cstdarg>
#include <vector>
#include <algorithm>
#include <limits>
using namespace std;
using namespace OpenVX;

MyNode::MyNode(enum vx_kernel_e kernel_e) : m_kernel_e(kernel_e)
{
	mPtrRandomNodes = NULL;
	mPropagateSuccessor = NULL;
	visited = false;
	inCluster = false;
}

MyNode::~MyNode()
{
	if (mPtrRandomNodes != NULL)
		delete mPtrRandomNodes;
}

void MyNode::connect(int input_p_cnt, int output_p_cnt, ...)
{
	m_input_p_cnt = input_p_cnt; m_output_p_cnt = output_p_cnt;

	va_list parameter_list;
	va_start(parameter_list, output_p_cnt);
	for (int i = 0; i < input_p_cnt; i++)
	{
		vx_reference vx_data_object = va_arg(parameter_list, vx_reference);
		DataObject *dataReadFromThisNode = DataObject::generateFromVX(vx_data_object);
		dataReadFromThisNode->addReader(this);
		m_inputs.push_back(dataReadFromThisNode);
	}
	for (int i = 0; i < output_p_cnt; i++)
	{
		vx_reference vx_data_object = va_arg(parameter_list, vx_reference);
		DataObject *dataWriteByThisNode = DataObject::generateFromVX(vx_data_object);
		dataWriteByThisNode->setWriter(this);
		m_outputs.push_back(DataObject::generateFromVX(vx_data_object));
	}
	va_end(parameter_list);
}

Node *MyNode::getNode() const
{
	return mNode;
}

Target MyNode::getTarget() const
{
	return mTarget;
}

vx_kernel_e MyNode::getKernele() const
{
	return m_kernel_e;
}

void MyNode::setRandomNodesPoiter(RandomNodes *ptrRandomNodes)
{
	mPtrRandomNodes = ptrRandomNodes;
}

void MyNode::generateNodes(int n_nodes, vector<MyNode *> &nodes, vector<vx_kernel_e> &kernel_es)
{
	for (int i = 0; i < n_nodes; i++)
	{
		nodes.push_back(new MyNode(kernel_es[i]));
	}
}

int MyNode::getPropagateRank(vector<MyNode *> &nodes)
{
	int n_nodes = nodes.size();
	float propagateRank = 0.0f;
	for (int i = 0; i < n_nodes; i++)
	{
		for (int n_in = 0; n_in < nodes[i]->m_input_p_cnt; n_in++)
		{
			if (nodes[i]->m_inputs[n_in] == m_outputs[0] &&
				nodes[i]->mRank > propagateRank)
			{
				propagateRank = nodes[i]->mRank;
				mPropagateSuccessor = nodes[i];
			}
		}
	}
	return propagateRank;
}

class ImproveSorter
{
	ProfileData *mPd;
public:
	ImproveSorter(ProfileData *pd) : mPd(pd) {}

	bool operator()(MyNode *a, MyNode *b) const 
	{
		return (mPd->getImproveFator(a->getKernele()) < mPd->getImproveFator(b->getKernele()));
	}
};

float MyNode::rank(vector<MyNode *> &nodes, ProfileData &profileData)
{
	int n_nodes = nodes.size();
	float rank_tmp;

	//Output data should be only one
	rank_tmp = this->getPropagateRank(nodes);

	//Transfer time for output data
	rank_tmp += profileData.getTransferTime(this->m_kernel_e, this->mTarget);

	//This node's computation time
	rank_tmp += profileData.getComputationTime(this->m_kernel_e, this->mTarget);

	//Transfer time for input data
	for (int n_in = 0; n_in < this->m_input_p_cnt; n_in++)
	{
		rank_tmp += profileData.getTransferTime(this->m_kernel_e, this->mTarget);
	}

	return rank_tmp;
}

map< Target, vector<MyNode *> > clusters;

bool MyNode::addClusterIfValid(Target target, MyNode *node)
{
	vector<MyNode *> &targetCluster = clusters[target];
	if (find(targetCluster.begin(), targetCluster.end(), node) == targetCluster.end())
	{
		node->inCluster = true;
		clusters[target].push_back(node);
	}
	return true;
}

static void clearCluster()
{
	clusters.clear();
}

void MyNode::priority(Target pTarget, Graph& graph, vector<vx_kernel_e> &kernel_es, vector<MyNode *> &nodes)
{
	int n_nodes = nodes.size();
	// Assign to the priority target
	for (int i = 0; i < n_nodes; i++)
	{
		//nodes[i]->mTarget = profileData.getMiniComputeTimeTarget(kernel_es[i]);
		nodes[i]->mTarget = pTarget;
	}
	addNodesToVxGraph(graph, nodes);
}

void MyNode::nodeCoarsen(Graph& graph, ProfileData &profileData, 
						 vector<vx_kernel_e> &kernel_es, vector<MyNode *> &nodes)
{
	int n_nodes = nodes.size();

	// Assign to minimal compute target
	for (int i = 0; i < n_nodes; i++)
	{
		//nodes[i]->mTarget = profileData.getMiniComputeTimeTarget(kernel_es[i]);
		nodes[i]->mTarget = profileData.getMiniTurnAroundTimeTarget(kernel_es[i]);
		nodes[i]->mRank = 0.0f;
		nodes[i]->visited = false;
		nodes[i]->inCluster = false;
	}

	// Compute the critical path
	for (int i = n_nodes-1; i >= 0; i--)
	{
		nodes[i]->mRank = nodes[i]->rank(nodes, profileData);
	}

	// Find start nodes
	vector<MyNode *> start_nodes;
	for (int i = 0; i < n_nodes; i++)
	{
		bool not_start_node = false;
		for (int n_in = 0; n_in < nodes[i]->m_input_p_cnt; n_in++)
		{
			for (int otherNode = 0; otherNode < n_nodes; otherNode++)
			{
				if (i == otherNode)	continue;
				for (int n_out = 0; n_out < nodes[otherNode]->m_output_p_cnt; n_out++)
				{
					if (nodes[i]->m_inputs[n_in] == nodes[otherNode]->m_outputs[n_out])
					{
						not_start_node = true;
						break;
					}
				}
				if (not_start_node)	break;
			}
			if (not_start_node)	break;
		}
		if (!not_start_node)	start_nodes.push_back(nodes[i]);
	}

	vector<vector<MyNode *>> critical_paths;
	for (int n_start = 0; n_start < start_nodes.size(); n_start++)
	{
		vector<MyNode *> critical_path;
		for (MyNode *c_node = nodes[0]; c_node != NULL;)
		{
			critical_path.push_back(c_node);
			c_node = c_node->mPropagateSuccessor;
		}
		critical_paths.push_back(critical_path);
	}

	// Get unvisited data objects on CP
	vector<Lamda> lamdaOnCP;
	for (int n_critical_path = 0; n_critical_path < critical_paths.size(); n_critical_path++)
	{
		vector<MyNode *> &critical_path = critical_paths[n_critical_path];
		for (int i = 0; i < critical_path.size(); i++)
		{
			MyNode *critical_node = critical_path[i];
			for (int in = 0; in < critical_node->m_input_p_cnt; in++)
			{
				DataObject *data_in = critical_node->m_inputs[in];
				if (data_in->writer == NULL)	//src data object
					lamdaOnCP.push_back(
						Lamda(data_in, critical_node, NULL, profileData.getTransferTime(critical_node->m_kernel_e, critical_node->mTarget))
					);
				//Writer node also on critical path
				else if (find(critical_path.begin(), critical_path.end(), data_in->writer) != critical_path.end())
				{
					lamdaOnCP.push_back(
						Lamda(data_in, critical_node, data_in->writer,
							profileData.getTransferTime(data_in->writer->m_kernel_e, data_in->writer->mTarget) +
							profileData.getTransferTime(critical_node->m_kernel_e, critical_node->mTarget))
					);
				}
			}
			for (int out = 0; out < critical_node->m_output_p_cnt; out++)
			{
				DataObject *data_out = critical_node->m_outputs[out];
				if (data_out->readers.size() == 0)	//dst data object
					lamdaOnCP.push_back(
						Lamda(data_out, NULL, critical_node, profileData.getTransferTime(critical_node->m_kernel_e, critical_node->mTarget))
					);
				else
				{
					//Reader node also on critical path
					for (int r = 0; r < data_out->readers.size(); r++)
					{
						if (find(critical_path.begin(), critical_path.end(), data_out->readers[r]) != critical_path.end())
						{
							lamdaOnCP.push_back(
								Lamda(data_out, data_out->readers[r], critical_node,
									profileData.getTransferTime(critical_node->m_kernel_e, critical_node->mTarget) +
									profileData.getTransferTime(data_out->readers[r]->m_kernel_e, data_out->readers[r]->mTarget)
								)
							);
						}
					}
				}
			}
		}
	}
	sort(lamdaOnCP.begin(), lamdaOnCP.end());

	//clusterCP
	while (lamdaOnCP.size() != 0)	//Have unvisited data on CP
	{
		Lamda maxLamda = lamdaOnCP.back();
		lamdaOnCP.pop_back();
		if (maxLamda.writer != NULL &&  maxLamda.reader != NULL && maxLamda.writer->mTarget == maxLamda.reader->mTarget)
		{
			addClusterIfValid(maxLamda.writer->mTarget, maxLamda.writer);
			addClusterIfValid(maxLamda.writer->mTarget, maxLamda.reader);
		}
		else
		{
			float gain = maxLamda.value;
			float changeW = numeric_limits<float>::max();
			float changeR = numeric_limits<float>::max();
			if (maxLamda.writer != NULL && maxLamda.reader != NULL)
			{
				if (!maxLamda.writer->visited)
					changeW = profileData.getComputationTime(maxLamda.writer->m_kernel_e, maxLamda.reader->mTarget)
						- profileData.getComputationTime(maxLamda.writer->m_kernel_e, maxLamda.writer->mTarget);
				if (!maxLamda.reader->visited)
					changeR = profileData.getComputationTime(maxLamda.reader->m_kernel_e, maxLamda.writer->mTarget)
						- profileData.getComputationTime(maxLamda.reader->m_kernel_e, maxLamda.reader->mTarget);
			}
			float cost = min(changeW, changeR);
			if (gain > cost)
			{
				Target resideTarget = maxLamda.reader->mTarget;
				if (cost != changeW)
					resideTarget = maxLamda.writer->mTarget;
				maxLamda.reader->mTarget = resideTarget;
				addClusterIfValid(resideTarget, maxLamda.writer);
				addClusterIfValid(resideTarget, maxLamda.reader);
			}
		}
		if (maxLamda.writer != NULL)	maxLamda.writer->visited = true;
		if (maxLamda.reader != NULL)	maxLamda.reader->visited = true;
	}

	// Get nodes not on CP
	vector<MyNode *> non_critical_nodes;
	for (int i = 0; i < n_nodes; i++)
	{
		for (int n_critical_path = 0; n_critical_path < critical_paths.size(); n_critical_path++)
		{
			vector<MyNode *> &critical_path = critical_paths[n_critical_path];
			if (find(critical_path.begin(), critical_path.end(), nodes[i]) == critical_path.end())
			{
				non_critical_nodes.push_back(nodes[i]);
			}
		}
	}

	//clusterG
	sort(non_critical_nodes.begin(), non_critical_nodes.end(), ImproveSorter(&profileData));
	for (int i = 0; i < non_critical_nodes.size(); i++)
	{
		int diffCount = 0;
		MyNode *ncn = non_critical_nodes[i];
		for (int in = 0; in < ncn->m_input_p_cnt; in++)
		{
			DataObject *din2ncn = ncn->m_inputs[in];
			if (din2ncn->writer!=NULL && din2ncn->writer->visited && 
				din2ncn->writer->mTarget != ncn->mTarget)
				diffCount++;
		}
		for (int out = 0; out < ncn->m_output_p_cnt; out++)
		{
			DataObject *doutBYncn = ncn->m_outputs[out];
			for (int doutR = 0; doutR < doutBYncn->readers.size(); doutR++)
			{
				if (doutBYncn->readers[doutR]->visited &&
					doutBYncn->readers[doutR]->mTarget != ncn->mTarget)
					diffCount++;
			}
		}

		if(diffCount == 0)	addClusterIfValid(ncn->mTarget, ncn);
		else
		{
			vector<Lamda> lamdaAroundNCP;
			for (int in = 0; in < ncn->m_input_p_cnt; in++)
			{
				DataObject *din2ncn = ncn->m_inputs[in];
				float lamdaValue = profileData.getTransferTime(ncn->m_kernel_e, ncn->mTarget);
				if (din2ncn->writer != NULL)
					lamdaValue += profileData.getTransferTime(din2ncn->writer->m_kernel_e, din2ncn->writer->mTarget);
				lamdaAroundNCP.push_back(
					Lamda(din2ncn, ncn, din2ncn->writer, lamdaValue)
				);
			}
			for (int out = 0; out < ncn->m_output_p_cnt; out++)
			{
				DataObject *doutBYncn = ncn->m_outputs[out];
				for (int r = 0; r < doutBYncn->readers.size(); r++)
				{
					lamdaAroundNCP.push_back(
						Lamda(doutBYncn, doutBYncn->readers[r], ncn,
							profileData.getTransferTime(ncn->m_kernel_e, ncn->mTarget) +
							profileData.getTransferTime(doutBYncn->readers[r]->m_kernel_e, doutBYncn->readers[r]->mTarget)
						)
					);
				}
			}
			sort(lamdaAroundNCP.begin(), lamdaAroundNCP.end());
			Lamda maxLamda = lamdaAroundNCP.back();
			float gain = maxLamda.value;
			float cost = profileData.getComputationTime(ncn->m_kernel_e, ncn->mTarget);
			Target resideTarget;
			if (maxLamda.reader == ncn)
				resideTarget = maxLamda.writer->mTarget;
			else
				resideTarget = maxLamda.reader->mTarget;
			cost += profileData.getComputationTime(ncn->m_kernel_e, resideTarget);
			if (gain > cost)
			{
				addClusterIfValid(resideTarget, ncn);
				ncn->mTarget = resideTarget;
			}
		}
		ncn->visited = true;
	}

	DataObject::registerForLocalOptimized(graph, clusters);

	addNodesToVxGraph(graph, nodes);
}

void MyNode::clusterToCSV(std::fstream &csvCluster, int n_nodes)
{
	int inClusterCnt = 0;
	Target clusterTargetOrder[] = { TARGET_C_MODEL, TARGET_OPENCL, TARGET_HEXAGON };
	for (int i=0; i<3; i++)
	{
		vector<MyNode *> &targetCluster = clusters[clusterTargetOrder[i]];
		csvCluster << targetCluster.size() << ",";
		inClusterCnt += targetCluster.size();
	}
	csvCluster << n_nodes- inClusterCnt << std::endl;
}

void MyNode::addNodesToVxGraph(Graph& graph, vector<MyNode *> &nodes)
{
	int n_nodes = nodes.size();
	for (int i = 0; i < n_nodes; i++)
	{
		int idx = 0;
		nodes[i]->mNode = graph.addNode(nodes[i]->m_kernel_e, nodes[i]->mTarget);
		for (int in = 0; in < nodes[i]->m_input_p_cnt; in++)
		{
			nodes[i]->mNode->setParameterByIndex(idx++, nodes[i]->m_inputs[in]->m_vx_reference);
		}
		for (int out = 0; out < nodes[i]->m_output_p_cnt; out++)
		{
			nodes[i]->mNode->setParameterByIndex(idx++, nodes[i]->m_outputs[out]->m_vx_reference);
		}
	}
}

void MyNode::releaseNodes(Graph& graph, vector<MyNode *> &nodes)
{
	int n_nodes = nodes.size();
	for (int i = 0; i < n_nodes; i++)
	{
		graph.removeNode(nodes[i]->mNode);
		delete nodes[i];
	}
	DataObject::releaseDataObjects();
	clearCluster();
}