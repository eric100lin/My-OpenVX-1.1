#ifndef _APPLICATION_HPP_
#define _APPLICATION_HPP_
#include "vx.hpp"
#define IMG_WIDTH 640
#define IMG_HEIGHT 480
#define SRC_IMG_NAME1 "lena.jpg"
#define SRC_IMG_NAME2 "baboon.jpg"

namespace OpenVX
{
	class Application
	{
	protected:
		Context &mContext;
		Graph *mGraph;
		vx_kernel_e mKernel_e;
		bool verifyTwoMat(cv::Mat inMat, cv::Mat resultMat);
	public:
		Application(Context &context, vx_kernel_e kernel_e);
		virtual ~Application();
		virtual std::string getKernelFullName(enum Target target_e);
		
		virtual void prepareInput() = 0;
		virtual void setup() = 0;
		virtual void process(enum Target target_e) = 0;
		virtual bool verify() = 0;
		virtual void release() = 0;
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
		void process(enum Target target_e);
		bool verify();
		void release();
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
		void process(enum Target target_e);
		bool verify();
		void release();
	};

	class AppTableLookup : public Application
	{
		cv::Mat src, *resultVX;
		Image *in, *out;
		vx_lut lut;

	public:
		AppTableLookup(Context &context);
		virtual ~AppTableLookup();

		void prepareInput();
		void setup();
		void process(enum Target target_e);
		bool verify();
		void release();
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
		void process(enum Target target_e);
		bool verify();
		void release();
	};
}

#endif