#include "DataObject.h"
#include "MyNode.h"

using namespace std;
using namespace OpenVX;

vector< DataObject *> allDataObjects;

DataObject::DataObject(vx_reference vx_data_object)
{
	m_vx_reference = vx_data_object;
	writer = NULL;
}

void DataObject::setWriter(MyNode *writerNode)
{
	writer = writerNode;
}

void DataObject::addReader(MyNode *readerNode)
{
	readers.push_back(readerNode);
}

DataObject *DataObject::generateFromVX(vx_reference vx_data_object)
{
	for (int i = 0; i < allDataObjects.size(); i++)
	{
		if (allDataObjects[i]->m_vx_reference == vx_data_object)
			return allDataObjects[i];
	}
	DataObject *ptrNewDataObject = new DataObject(vx_data_object);
	allDataObjects.push_back(ptrNewDataObject);
	return ptrNewDataObject;
}

bool isInCluster(map< Target, vector<MyNode *> > &clusters, MyNode *node)
{
	map< Target, vector<MyNode *> >::iterator itM = clusters.begin();
	for (; itM != clusters.end(); ++itM)
	{
		vector<MyNode *> &targetCluster = itM->second;
		vector<MyNode *>::iterator itV = targetCluster.begin();
		for (; itV != targetCluster.end(); itV++)
		{
			if (*itV == node)
				return true;
		}
	}
	return false;
}

void DataObject::registerForLocalOptimized(Graph & graph, map< Target, vector<MyNode *> > &clusters)
{
	for (int i = 0; i < allDataObjects.size(); i++)
	{
		bool allRWnodeInCluster = true;
		DataObject *ptrDobject = allDataObjects[i];
		if (ptrDobject->writer == NULL || ptrDobject->readers.size()==0)
			allRWnodeInCluster = false;
		else
		{
			allRWnodeInCluster &= isInCluster(clusters, ptrDobject->writer);
			for (int r = 0; r < ptrDobject->readers.size(); r++)
			{
				allRWnodeInCluster &= isInCluster(clusters, ptrDobject->readers[r]);
			}
		}
		if (allRWnodeInCluster)
		{
			graph.setLocalOptimized(ptrDobject->m_vx_reference);
		}
	}
}

void DataObject::releaseDataObjects()
{
	for (int i = 0; i < allDataObjects.size(); i++)
	{
		delete allDataObjects[i];
	}
	allDataObjects.clear();
}