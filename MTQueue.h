#ifndef MTQUEUE_H_
#define MTQUEUE_H_

#include <queue>
#include <pthread.h>
#include <utility>

template <typename T>
class MTQueue
{
public:
	MTQueue()
	{
		pthread_mutex_init(&mutex,NULL);
		pthread_cond_init(&cond,NULL);
	}

	~MTQueue()
	{
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&cond);
	}

	T pop()
	{
		pthread_mutex_lock(&mutex);
		while (queue.empty())
		{
			pthread_cond_wait(&cond,&mutex);
		}
		auto item = queue.front();
		queue.pop();
		pthread_mutex_unlock(&mutex);
		return item;
	}

	void pop(T& item)
	{
		pthread_mutex_lock(&mutex);
		while (queue.empty())
		{
			pthread_cond_wait(&cond,&mutex);
		}
		item = queue.front();
		queue.pop();
		pthread_mutex_unlock(&mutex);
	}

	void push(const T& item)
	{
		pthread_mutex_lock(&mutex);
		pthread_cond_signal(&cond);
		queue.push(item);
		pthread_mutex_unlock(&mutex);
	}

private:
	std::queue<T> queue;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
};

#endif /* MTQUEUE_H_ */
