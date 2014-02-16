#include <iostream>
#include <chrono>
#include <time.h>
#include "WebSocketLog.h"
#include "WebsocketServer.h"
using namespace std;
using namespace chrono;

WebsocketServer*& WebsocketLog::webserver()
{
	static WebsocketServer* ws = nullptr;
	return ws;
}

string getTime(std::chrono::high_resolution_clock::time_point logTime)
{
	milliseconds ms = duration_cast<milliseconds>(logTime.time_since_epoch());
	seconds s = duration_cast<seconds>(ms);
	time_t t = s.count();
	struct tm brokenDown;
	brokenDown = *localtime(&t);
	size_t msCount = ms.count()%1000;

	char buff[100];
	memset(buff,0,100);
	strftime(buff,100,"%X",&brokenDown);
	sprintf(buff,"%s.%i",buff,msCount);

	return string(buff);
}

void WebsocketLog::logMessage(std::chrono::high_resolution_clock::time_point logTime,
		LogLevel level, std::string file, unsigned int line, std::string function, std::string message)
{
	Json::Value jsonMessage;
	jsonMessage["type"] = "log";
	jsonMessage["logLine"] = Json::Value(Json::arrayValue);
	jsonMessage["logLine"][0] = getTime(logTime);
	jsonMessage["logLine"][1] = Log<WebsocketLog>::logLevelAsString(level);
	jsonMessage["logLine"][2] = message;
	jsonMessage["logLine"][3] = file;
	jsonMessage["logLine"][4] = line;
	webserver()->broadcastMessage(jsonMessage);
}
