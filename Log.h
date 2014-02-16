#ifndef LOG_H_
#define LOG_H_
#include <sstream>
#include <ctime>
#include <chrono>
#include <string>
#include <iostream>

enum LogLevel
{
	LOG_ERROR,
	LOG_WARNING,
	LOG_INFO,
	LOG_DEBUG,
	LOG_TRACE
};

template<class T>
class Log
{
public:
	Log(LogLevel Level, unsigned int Line, const char* File, const char* Function);
	~Log();

	std::ostringstream& getStream();
	static LogLevel& reportingLevel();
	static std::string logLevelAsString(LogLevel level);

protected:
	std::ostringstream os;

	unsigned int line;
	std::string file;
	std::string function;
	LogLevel messageLevel;
	std::chrono::high_resolution_clock::time_point logTime;

	inline std::string getTime();

private:
	Log(const Log&);
	Log& operator=(const Log&);
};

template<class T>
Log<T>::Log(LogLevel Level, unsigned int Line, const char* File, const char* Function)
	: line(Line), file(File), function(Function), messageLevel(Level)
{
	logTime = std::chrono::high_resolution_clock::now();
}

template<class T>
std::string Log<T>::logLevelAsString(LogLevel level)
{
	static const char* const buffer[] = {"Error","Warning","Info","Debug","Trace"};
	return buffer[level];
}

template<class T>
std::ostringstream& Log<T>::getStream()
{
	return os;
}

template<class T>
Log<T>::~Log()
{
	T::logMessage(logTime,messageLevel,file,line,function,os.str());
}

#ifndef GLOBAL_LOG_LEVEL
#define GLOBAL_LOG_LEVEL LOG_INFO
#endif

#define LOG(level,type) if (level > GLOBAL_LOG_LEVEL) ; \
		else Log<type>(level,__LINE__,__FILE__,__FUNCTION__).getStream()

#endif /* LOG_H_ */
