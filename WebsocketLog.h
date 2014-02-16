#ifndef WEBSOCKETLOG_H_
#define WEBSOCKETLOG_H_
#include <json/json.h>
#include <vector>
#include "Log.h"

class WebsocketServer;

class WebsocketLog
{
public:
	static WebsocketServer*& webserver();

	static void logMessage(std::chrono::high_resolution_clock::time_point logTime,
				LogLevel level, std::string file, unsigned int line, std::string function, std::string message);
};

#define WLOG(level) Log<WebsocketLog>(level,__LINE__,__FILE__,__FUNCTION__).getStream()

#endif /* WEBSOCKETLOG_H_ */
