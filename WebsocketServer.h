#ifndef WEBSOCKETSERVER_H_
#define WEBSOCKETSERVER_H_

#include <pthread.h>
#include <json/json.h>
#include "mongoose.h"
#include "Log.h"
#include "MTQueue.h"

class WebsocketServer
{
public:
	void broadcastMessage(const Json::Value& message);
	WebsocketServer();
	~WebsocketServer();
protected:
	bool acceptNewMessages;
	/** MTQueue is a simple queue with a mutex for synchronizing access */
	MTQueue<Json::Value> messageQueue;
	struct mg_context* mongooseContext;
	pthread_mutex_t mutex;
	std::vector<struct mg_connection*> clients;
	pthread_t thread;
	pthread_t pingThread;
	pthread_mutex_t pingMutex;
	pthread_cond_t pingCond;

	friend void* websocketServerThread(void* websocketServerPtr);
	friend void* websocketPingThread(void* websocketServerPtr);
	friend void websocketReadyHandler(struct mg_connection* connection);
	friend int websocketDataHandler(struct mg_connection* connection,
							int flags, char* data, unsigned int dataLength);
	friend void websocketClosedHandler(struct mg_connection* connection);
};


#endif /* WEBSOCKETSERVER_H_ */
