#include "LogStream.h"
using namespace OpenVX;

LogStream *l = NULL;

LogStream::LogStream(const char *file_name)
{
	if (!recordfile.is_open())
	{
		recordfile.open(file_name);
		ERROR_CHECK(!recordfile.is_open());
	}
}

void LogStream::precision(int precision)
{
	std::cout.precision(precision);
	recordfile.precision(precision);
}

void LogStream::close()
{
	recordfile.close();
	delete l;
	l = NULL;
}

LogStream& OpenVX::logs(const char *file_name)
{ 
	if (l==NULL)
		l = new LogStream(file_name);
	return *l; 
}