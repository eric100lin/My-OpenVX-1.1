#ifndef _LOG_STREAM_HPP_
#define _LOG_STREAM_HPP_
#include "vx.hpp"
#define RECODRD_FILE_NAME "record.txt"

namespace OpenVX
{
	class LogStream
	{
	private:
		std::ofstream recordfile;
	public:
		LogStream(const char *file_name);

		template <typename T>
		friend LogStream & operator<<(LogStream &logs, const T& value)
		{
			std::cout << value;
			logs.recordfile << value;
			return logs;
		}

		typedef std::ostream& (*ostream_manipulator)(std::ostream&);
		friend LogStream & operator<<(LogStream &logs, ostream_manipulator pf)
		{
			return operator<< <ostream_manipulator> (logs, pf);
		}

		void precision(int precision);

		void close();
	};

	LogStream& logs(const char *file_name = RECODRD_FILE_NAME);
}

#endif