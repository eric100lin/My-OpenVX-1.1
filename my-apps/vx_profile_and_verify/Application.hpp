#ifndef _APPLICATION_HPP_
#define _APPLICATION_HPP_
#include "vx.hpp"
#define IMG_WIDTH 640
#define IMG_HEIGHT 480
#define SRC_IMG_NAME1 "lena.jpg"
#define SRC_IMG_NAME2 "baboon.jpg"

#define APP_ONE_NODE (1)
#define APP_TWO_NODE (2)
#define APP_THREE_NODE (3)

namespace OpenVX
{
	class Application
	{
	protected:
		Context &mContext;
		Graph *mGraph;
		std::vector<vx_kernel_e> mKernel_es;
		std::map<int, std::vector<enum Target>> support_targets;
		bool verifyTwoMat(cv::Mat inMat, cv::Mat resultMat);
		void printProfilingResult(int n_times, int n_nodes, Node *nodes[], enum Target targets[]);
		void getVariantTarget(int variant_numer, enum Target *ptrTargets);
	public:
		Application(Context &context, int n_kernels, ...);
		virtual ~Application();
		void setSupportTargets(int kernel_index, int n_targets, ...);
		int getVariantCount();
		std::string getKernelesType();
		std::vector<std::string> getNodesName(int variant_numer);
		
		virtual void prepareInput() = 0;
		virtual void setup() = 0;
		virtual void process(int variant_numer) = 0;
		virtual void profiling(int n_times, int variant_numer) = 0;
		virtual bool verify() = 0;
		virtual void release() = 0;
		virtual void releaseInput() = 0;
	};

	class AppOneIOneO : public Application
	{
		cv::Mat lena_src, *resultVX;
		Image *in, *out;

	public:
		AppOneIOneO(Context &context, vx_kernel_e kernel_e);
		virtual ~AppOneIOneO();

		void prepareInput();
		void setup();
		void process(int variant_numer);
		void profiling(int n_times, int variant_numer);
		bool verify();
		void release();
		void releaseInput();
		
		static void generateApps(Context &context, std::vector<Application *> *apps);
	};

	class AppTwoIOneO : public Application
	{
		cv::Mat src1, src2, *resultVX;
		Image *in1, *in2, *out;

	public:
		AppTwoIOneO(Context &context, vx_kernel_e kernel_e);
		virtual ~AppTwoIOneO();

		void prepareInput();
		void setup();
		void process(int variant_numer);
		void profiling(int n_times, int variant_numer);
		bool verify();
		void release();
		void releaseInput();

		static void generateApps(Context &context, std::vector<Application *> *apps);
	};

	class AppTableLookup : public Application
	{
		cv::Mat src, *resultVX;
		Image *in, *out;
		vx_lut lut;
		uchar *lut_data;

	public:
		AppTableLookup(Context &context);
		virtual ~AppTableLookup();

		void prepareInput();
		void setup();
		void process(int variant_numer);
		void profiling(int n_times, int variant_numer);
		bool verify();
		void release();
		void releaseInput();
	};

	class AppHistogram : public Application
	{
		cv::Mat src;
		Image *in;
		vx_distribution distribution;

	public:
		AppHistogram(Context &context);
		virtual ~AppHistogram();

		void prepareInput();
		void setup();
		void process(int variant_numer);
		void profiling(int n_times, int variant_numer);
		bool verify();
		void release();
		void releaseInput();
	};
	
	class App2Node_1I1O_1I1O : public Application
	{
		cv::Mat lena_src, *resultVX;
		Image *in, *out;
		VirtualImage *tmp;

	public:
		App2Node_1I1O_1I1O(Context &context, vx_kernel_e kernel_e1, vx_kernel_e kernel_e2);
		virtual ~App2Node_1I1O_1I1O();

		void prepareInput();
		void setup();
		void process(int variant_numer);
		void profiling(int n_times, int variant_numer);
		bool verify();
		void release();
		void releaseInput();
		
		static void generateApps(Context &context, std::vector<Application *> *apps);
	};

	class App2Node_2I1O_1I1O : public Application
	{
		cv::Mat src1, src2, *resultVX;
		Image *in1, *in2, *out;
		VirtualImage *tmp;

	public:
		App2Node_2I1O_1I1O(Context &context, vx_kernel_e kernel_e1, vx_kernel_e kernel_e2);
		virtual ~App2Node_2I1O_1I1O();

		void prepareInput();
		void setup();
		void process(int variant_numer);
		void profiling(int n_times, int variant_numer);
		bool verify();
		void release();
		void releaseInput();

		static void generateApps(Context &context, std::vector<Application *> *apps);
	};

	class App3Node_1I1O_1I1O_2I1O : public Application
	{
		cv::Mat src1, src2, *resultVX;
		Image *in1, *in2, *out;
		VirtualImage *tmp1, *tmp2;

	public:
		App3Node_1I1O_1I1O_2I1O(Context &context, vx_kernel_e kernel_e1, vx_kernel_e kernel_e2, vx_kernel_e kernel_e3);
		virtual ~App3Node_1I1O_1I1O_2I1O();

		void prepareInput();
		void setup();
		void process(int variant_numer);
		void profiling(int n_times, int variant_numer);
		bool verify();
		void release();
		void releaseInput();

		static void generateApps(Context &context, std::vector<Application *> *apps);
	};

	class AppAddSub : public Application
	{
		cv::Mat src1, src2, *resultVX;
		Image *in1, *in2, *out;
		vx_scalar spolicy;

	public:
		AppAddSub(Context &context, vx_kernel_e kernel_e);
		virtual ~AppAddSub();

		void prepareInput();
		void setup();
		void process(int variant_numer);
		void profiling(int n_times, int variant_numer);
		bool verify();
		void release();
		void releaseInput();

		static void generateApps(Context &context, std::vector<Application *> *apps);
	};

	class AppThreshold : public Application
	{
		cv::Mat src, *resultVX;
		Image *in, *out;
		vx_threshold mThreshold;

	public:
		AppThreshold(Context &context, int threshold);
		virtual ~AppThreshold();

		void prepareInput();
		void setup();
		void process(int variant_numer);
		void profiling(int n_times, int variant_numer);
		bool verify();
		void release();
		void releaseInput();

		static void generateApps(Context &context, std::vector<Application *> *apps);
	};
}

#endif