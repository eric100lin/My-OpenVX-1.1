#ifndef _DATA_OBJECT_H_
#define _DATA_OBJECT_H_
#include "vx.hpp"
#include <vector>
#include <map>

namespace OpenVX
{
	class MyNode;

	class DataObject
	{
		friend class MyNode;

		private:
			vx_reference m_vx_reference;
			std::vector< MyNode *> readers;
			MyNode *writer;
		public:
			DataObject(vx_reference vx_data_object);

			void setWriter(MyNode *writerNode);

			void addReader(MyNode *readerNode);

			static DataObject *generateFromVX(vx_reference vx_data_object);

			static void registerForLocalOptimized(Graph & graph, std::map< Target, std::vector<MyNode *> > &clusters);

			static void releaseDataObjects();
	};

	class Lamda
	{
	public:
		DataObject *dataObject;
		MyNode *reader;
		MyNode *writer;
		float value;
		Lamda(DataObject *d, MyNode *r, MyNode *w, float v) : dataObject(d), reader(r), writer(w), value(v) {}

		bool operator < (const Lamda& anotherLamda) const
		{
			return (value < anotherLamda.value);
		}
	};
}

#endif