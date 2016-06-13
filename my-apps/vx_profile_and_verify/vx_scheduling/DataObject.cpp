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
		int readerCount = 0;
		bool allRWnodeInCluster = true;
		bool needTransferBack = false;
		DataObject *ptrDobject = allDataObjects[i];
		if (ptrDobject->writer == NULL || ptrDobject->readers.size()==0)
			allRWnodeInCluster = false;
		else
		{
			allRWnodeInCluster &= isInCluster(clusters, ptrDobject->writer);
			for (int r = 0; r < ptrDobject->readers.size(); r++)
			{
				allRWnodeInCluster &= isInCluster(clusters, ptrDobject->readers[r]);
				if (ptrDobject->readers[r]->getTarget() == ptrDobject->writer->getTarget())
					readerCount++;
				else needTransferBack = true;	/* TODO: NON-VIRTUAL DATA OBJECT CASE */
			}
		}
		if (allRWnodeInCluster)
		{
			vx_bool transferBack = needTransferBack ? vx_true_e : vx_false_e;
			DataObject::setLocalizedReference(ptrDobject->m_vx_reference, readerCount, transferBack);
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

void DataObject::setLocalizedReference(vx_reference ref, vx_uint32 readerCount, vx_bool transferBack)
{
	vx_status status;
	vx_bool localized = vx_true_e;

	status = vxSetReferenceAttribute(ref, VX_REF_DO_LOCAL_OPTIMIZED_FOR_REFERENCE, &localized, sizeof(vx_bool));
	ERROR_CHECK(status);
	status = vxSetReferenceAttribute(ref, VX_REF_LOCAL_OPTIMIZED_REFERENCE_READER_CNT, &readerCount, sizeof(vx_uint32));
	ERROR_CHECK(status);
	status = vxSetReferenceAttribute(ref, VX_REF_LOCAL_OPTIMIZED_NEED_TRANSFER_BACK, &transferBack, sizeof(vx_bool));
	ERROR_CHECK(status);
}