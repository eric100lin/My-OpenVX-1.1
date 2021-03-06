#include "Application.hpp"
#define CSV_NAME "profiling.csv"
#define NS_TO_MS (1000*1000*1.0)
using namespace OpenVX;
using namespace cv;

static int instance_count = 0;
static std::fstream csv(CSV_NAME, std::fstream::out);

Application::Application(Context &context, int n_kernels, ...) 
	: mContext(context)
{
	mGraph = new Graph(context);
	instance_count++;
	
	va_list parameter_list;
	va_start(parameter_list, n_kernels);
	for(int i=0; i<n_kernels; i++)
	{
		int int_kernel_e = va_arg(parameter_list, int);
		vx_kernel_e kernel_e = static_cast<vx_kernel_e>(int_kernel_e);
		mKernel_es.push_back(kernel_e);
		support_targets.insert( std::make_pair( i, std::vector<enum Target>()));
	}
	va_end(parameter_list);
}

Application::~Application()
{
	delete mGraph;
	if (--instance_count == 0)	csv.close();
}

void Application::setSupportTargets(int kernel_index, int n_targets, ...)
{
	va_list parameter_list;
	va_start(parameter_list, n_targets);
	for(int i=0; i<n_targets; i++)
	{
		int int_target_e = va_arg(parameter_list, int);
		Target target_e = static_cast<Target>(int_target_e);
		support_targets[kernel_index].push_back(target_e);
	}
	va_end(parameter_list);
}

int Application::getVariantCount()
{
	ERROR_CHECK(support_targets.size()==0);
	int variants = 1;
	for (std::map<int, std::vector<enum Target>>::iterator it=support_targets.begin(); 
		 it!=support_targets.end(); ++it)
	{
		std::vector<enum Target> targets = it->second;
		variants *= targets.size();
	}
	return variants;
}

std::string Application::getKernelesType()
{
	std::stringstream ss;
	ss << mKernel_es.size() << " kernels - ";
	ss << Kernel::getKernelNameOfType(mKernel_es[0]);
	for (int i = 1; i < mKernel_es.size(); i++)
	{
		ss << ", " << Kernel::getKernelNameOfType(mKernel_es[i]);
	}
	return ss.str();
}

std::vector<std::string> Application::getNodesName(int variant_numer)
{
	std::vector<std::string> nodeNames;
	enum Target *targets = new enum Target[mKernel_es.size()];
	getVariantTarget(variant_numer, targets);
	for (int i = 0; i < mKernel_es.size(); i++)
	{
		nodeNames.push_back(Kernel::getFullKernelName(mKernel_es[i], targets[i]));
	}
	delete[] targets;
	return nodeNames;
}

void Application::printProfilingResult(int n_times, int n_nodes, Node *nodes[], enum Target targets[])
{
	vx_perf_t graphPref, firstTimePerfG;
	vx_perf_t *firstTimeComputeN = new vx_perf_t[n_nodes];
	vx_perf_t *firstTimePerfN = new vx_perf_t[n_nodes];
	
	mGraph->process();
	firstTimePerfG = mGraph->getPerformance();
	for(int n=0; n<n_nodes; n++)
	{
		firstTimePerfN[n] = nodes[n]->getPerformance();
		firstTimeComputeN[n] = nodes[n]->getComputationTime();
	}
	
	for(int i=0; i<n_times; i++)
		mGraph->process();
	
	logs().precision(2);
	logs() << std::fixed;

	graphPref = mGraph->getPerformance();
	logs() << "   "
		   << "first: " << firstTimePerfG.tmp / NS_TO_MS << " ms "
		   << "min: " << graphPref.min / NS_TO_MS << " ms "
		   << "max: " << graphPref.max / NS_TO_MS << " ms "
		   << "avg: " << (graphPref.sum - firstTimePerfG.sum) / (n_times*NS_TO_MS) << " ms " << std::endl;

	for(int n=0; n<n_nodes; n++)
	{
		vx_perf_t nodePerf = nodes[n]->getPerformance();
		vx_perf_t nodeCompute = nodes[n]->getComputationTime();
		logs() << "\tnode[" << n << "] - " << std::endl
			   << "\t total    " 
			   << "first: " << firstTimePerfN[n].tmp/NS_TO_MS << " ms "
			   << "min: "   << nodePerf.min/NS_TO_MS << " ms "
			   << "max: "   << nodePerf.max/NS_TO_MS  << " ms "
			   << "avg: "   << (nodePerf.sum-firstTimePerfN[n].sum)/(n_times*NS_TO_MS) << " ms " << std::endl
			   << "\t compute  " 
			   << "first: " << firstTimeComputeN[n].tmp/NS_TO_MS << " ms "
			   << "min: "   << nodeCompute.min/NS_TO_MS << " ms "
			   << "max: "   << nodeCompute.max/NS_TO_MS  << " ms "
			   << "avg: "   << (nodeCompute.sum-firstTimeComputeN[n].sum)/(n_times*NS_TO_MS) << " ms " << std::endl
			   << "\t transfer " 
			   << "first: " << (firstTimePerfN[n].tmp-firstTimeComputeN[n].tmp)/NS_TO_MS << " ms "
			   << "min: "   << (nodePerf.min-nodeCompute.min)/NS_TO_MS << " ms "
			   << "max: "   << (nodePerf.max-nodeCompute.min)/NS_TO_MS  << " ms "
			   << "avg: "   << ((nodePerf.sum-firstTimePerfN[n].sum)-
								(nodeCompute.sum-firstTimeComputeN[n].sum))/(n_times*NS_TO_MS) << " ms "
			   << std::endl;

		csv << Kernel::getFullKernelName(mKernel_es[n], targets[n]) << ","
			<< (nodeCompute.sum - firstTimeComputeN[n].sum) / (n_times*NS_TO_MS) << ","
			<< ((nodePerf.sum - firstTimePerfN[n].sum) -
			    (nodeCompute.sum - firstTimeComputeN[n].sum)) / (n_times*NS_TO_MS) << std::endl;
	}
	
	for(int n=0; n<n_nodes; n++)
		mGraph->removeNode(nodes[n]);
	
	delete [] firstTimePerfN;
	delete [] firstTimeComputeN;
}

static bool getVariant(int *cnt, int goal,
	std::map<int, std::vector<enum Target>>::iterator it, 
	std::map<int, std::vector<enum Target>>::iterator end, enum Target *ptrTargets)
{
	if (it == end)
	{
		if ((*cnt) == goal)	return true;
		(*cnt)++;
	}
	else
	{
		std::vector<enum Target> targets = it->second;
		std::map<int, std::vector<enum Target>>::iterator nextit = it;
		std::advance(nextit, 1);
		for (int i = 0; i < targets.size(); i++)
		{
			*ptrTargets = targets[i];

			if (getVariant(cnt, goal, nextit, end, ptrTargets + 1))
				return true;
		}
	}
	return false;
}

void Application::getVariantTarget(int variant_numer, enum Target *ptrTargets)
{
	ERROR_CHECK(support_targets.size()==0);
	std::map<int, std::vector<enum Target>>::iterator it = support_targets.begin();
	
	int cnt=0;
	ERROR_CHECK(getVariant(&cnt, variant_numer, it, support_targets.end(), ptrTargets)!=true);
}

bool Application::verifyTwoMat(Mat inMat, Mat resultMat)
{
	int w, h, cnt = 0;
	int width = inMat.cols;
	int height = inMat.rows;
	unsigned char diff, max_diff=0, min_diff=255;
	long int sum_diff=0;

	unsigned char *ptr_inMat = inMat.data;
	unsigned char *ptr_resMat = resultMat.data;
	for (h = 0; h<height; h++)
	{
		for (w = 0; w<width; w++, ptr_inMat++, ptr_resMat++)
		{
			if (h == 0 || w == 0 || h == height - 1 || w == width - 1)
				continue;	//Don't verify boarder
			
			diff = abs(*ptr_resMat - *ptr_inMat);
			if (diff != 0)
			{
				if (++cnt < 11)
				{
					logs() << " fail at inMat[" << h << "," << w << "]:"
						   << (int)*ptr_inMat << " != resultMat[" << h << "," << w << "]:"
						   << (int)*ptr_resMat << " diff:" << (int)diff << std::endl;
				}
				if(diff>max_diff)	max_diff = diff;
				if(diff<min_diff)	min_diff = diff;
				sum_diff += diff;
			}
		}
	}
	if(cnt != 0)
	{
		int total_pixel = height*width;
		float percent = cnt*100.0f / total_pixel;
		logs() << " - error pixel: " << cnt << "/" << total_pixel 
			   << " (" << percent << " %)" << std::endl;
		logs() << " - max_diff: " << (int)max_diff << std::endl;
		logs() << " - min_diff: " << (int)min_diff << std::endl;
		logs() << " - average_diff: " << (int)(sum_diff / cnt) << std::endl;
	}
	return (cnt==0);
}
