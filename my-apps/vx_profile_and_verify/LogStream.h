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
		LogStream(const char *file_name)
		{
			if (!recordfile.is_open())
			{
				recordfile.open(RECODRD_FILE_NAME);
				ERROR_CHECK(!recordfile.is_open());
			}
		}

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

		void precision(int precision)
		{
			std::cout.precision(precision);
			recordfile.precision(precision);
		}

		void close()
		{
			recordfile.close();
		}
	};

	inline LogStream& logs() { static LogStream l(RECODRD_FILE_NAME); return l; }
}

#endif