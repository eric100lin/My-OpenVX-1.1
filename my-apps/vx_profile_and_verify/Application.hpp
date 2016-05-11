#ifndef _APPLICATION_HPP_
#define _APPLICATION_HPP_
#include"vx.hpp"
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
		bool verifyTwoMat(cv::Mat inMat, cv::Mat resultMat);
	public:
		Application(Context &context);
		virtual ~Application();

		virtual void prepareInput() = 0;
		virtual void process() = 0;
		virtual bool verify() = 0;
		virtual void release() = 0;
	};

	class AppOneIOneO : public Application
	{
		cv::Mat lena_src, *resultVX;
		Image *in, *out;
		vx_kernel_e mKernel_e;

	public:
		AppOneIOneO(Context &context, vx_kernel_e kernel_e);
		virtual ~AppOneIOneO();

		void prepareInput();
		void process();
		bool verify();
		void release();
	};

	class AppTwoIOneO : public Application
	{
		cv::Mat src1, src2, *resultVX;
		Image *in1, *in2, *out;
		vx_kernel_e mKernel_e;

	public:
		AppTwoIOneO(Context &context, vx_kernel_e kernel_e);
		virtual ~AppTwoIOneO();

		void prepareInput();
		void process();
		bool verify();
		void release();
	};

	class AppTableLookup : public Application
	{
		cv::Mat src, *resultVX;
		Image *in, *out;
		vx_lut lut;
		vx_kernel_e mKernel_e;

	public:
		AppTableLookup(Context &context);
		virtual ~AppTableLookup();

		void prepareInput();
		void process();
		bool verify();
		void release();
	};

	class AppHistogram : public Application
	{
		cv::Mat src;
		Image *in;
		vx_distribution distribution;
		vx_kernel_e mKernel_e;

	public:
		AppHistogram(Context &context);
		virtual ~AppHistogram();

		void prepareInput();
		void process();
		bool verify();
		void release();
	};
}

#endif