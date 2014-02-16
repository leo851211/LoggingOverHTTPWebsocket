#include <cstring>
#include <string>
#include <algorithm>
#include <iostream>
#include <time.h>
#include "WebsocketServer.h"
using namespace std;

void* websocketServerThread(void* websocketServerPtr);
void* websocketPingThread(void* websocketServerPtr);
void websocketReadyHandler(struct mg_connection* connection);
int websocketDataHandler(struct mg_connection* connection,
						int flags, char* data, unsigned int dataLength);
void websocketClosedHandler(struct mg_connection* connection);


WebsocketServer::WebsocketServer()
	: acceptNewMessages(true)
{
	pthread_mutex_init(&mutex,NULL);
	pthread_mutex_init(&pingMutex,NULL);
	pthread_cond_init(&pingCond,NULL);

	const char* options[] =
	{
			"listening_ports", "8080",
			NULL
	};

	struct mg_callbacks callbacks;
	memset(&callbacks,0,sizeof(callbacks));
	callbacks.websocket_ready = websocketReadyHandler;
	callbacks.websocket_data = websocketDataHandler;
	callbacks.connection_closed = websocketClosedHandler;

	mongooseContext = mg_start(&callbacks,this,options);
	pthread_create(&thread,NULL,websocketServerThread,this);
	pthread_create(&pingThread,NULL,websocketPingThread,this);
}

void WebsocketServer::broadcastMessage(const Json::Value& message)
{
	messageQueue.push(message);
}

WebsocketServer::~WebsocketServer()
{
	acceptNewMessages = false;
	Json::Value stopMessage;
	stopMessage["type"] = "stop";
	broadcastMessage(stopMessage);
	void* retVal;
	pthread_join(thread,&retVal);
	pthread_mutex_lock(&pingMutex);
	pthread_cond_signal(&pingCond);
	pthread_mutex_unlock(&pingMutex);
	pthread_join(pingThread,&retVal);
	mg_stop(mongooseContext);

	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&pingMutex);
	pthread_cond_destroy(&pingCond);
}

void websocketReadyHandler(struct mg_connection* connection)
{
	WebsocketServer* ws = (WebsocketServer*) mg_get_user_data_from_connection(connection);
	pthread_mutex_lock(&ws->mutex);
	ws->clients.push_back(connection);
	pthread_mutex_unlock(&ws->mutex);
}

int websocketDataHandler(struct mg_connection* connection,
						int flags, char* data, unsigned int dataLength)
{
	Json::Value message;
	Json::Reader reader;
	reader.parse(data,data+dataLength,message,false);
	if (message.get("type","n/a").asString().compare("close") == 0)
		return 0;
	else
		return 1;
}

void websocketClosedHandler(struct mg_connection* connection)
{
	WebsocketServer* ws = (WebsocketServer*) mg_get_user_data_from_connection(connection);
	pthread_mutex_lock(&ws->mutex);
	auto conn = find(ws->clients.begin(),ws->clients.end(),connection);
	if (conn != ws->clients.end())
	{
		ws->clients.erase(conn);
	}
	pthread_mutex_unlock(&ws->mutex);
}

void* websocketServerThread(void* websocketServerPtr)
{
	WebsocketServer* ws = (WebsocketServer*) websocketServerPtr;
	Json::Value message;
	string messageType;
	while (ws->acceptNewMessages)
	{
		ws->messageQueue.pop(message);
		messageType = message.get("type","n/a").asString();
		if (messageType.compare("stop") == 0)
		{
			return nullptr;
		}
		else if (messageType.compare("ping") == 0)
		{
			pthread_mutex_lock(&ws->mutex);
			for (auto client : ws->clients)
			{
				mg_websocket_write(client,WEBSOCKET_OPCODE_PING,nullptr,0);
			}
			pthread_mutex_unlock(&ws->mutex);
		} else
		{
			Json::FastWriter writer;
			string messageSerialized = writer.write(message);
			pthread_mutex_lock(&ws->mutex);
			for (auto client : ws->clients)
			{
				mg_websocket_write(client,WEBSOCKET_OPCODE_TEXT,messageSerialized.c_str(),messageSerialized.length());
			}
			pthread_mutex_unlock(&ws->mutex);
		}
	}
	return nullptr;
}

void* websocketPingThread(void* websocketServerPtr)
{
	WebsocketServer* ws = (WebsocketServer*) websocketServerPtr;
	pthread_mutex_lock(&ws->pingMutex);
	int result;
	struct timespec ts;
	time_t timer;
	time(&timer);
	ts.tv_sec = timer;
	ts.tv_nsec = timer*1000000000;
	Json::Value pingMessage;
	pingMessage["type"] = "ping";
	while (ws->acceptNewMessages)
	{
		ts.tv_sec += 5;
		ts.tv_nsec = ts.tv_sec * 1000000000;
		result = pthread_cond_timedwait(&ws->pingCond,&ws->pingMutex,&ts);
		if (result == ETIMEDOUT)
		{
			ws->broadcastMessage(pingMessage);
		} else
		{
			return nullptr;
		}
	}
	pthread_mutex_unlock(&ws->pingMutex);
	return nullptr;
}
