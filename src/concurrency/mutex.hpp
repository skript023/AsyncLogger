#ifndef MUTEX_H
#define MUTEX_H

#if defined(__linux__) || defined(__APPLE__)
#include <pthread.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

class Mutex {
public:
	Mutex();
	~Mutex();
	void lock();
	bool trylock();
	void unlock();
#if defined(_WIN32)
	CRITICAL_SECTION* native_handle() noexcept { return &lck; }
#else
	pthread_mutex_t* native_handle() noexcept { return &lck; }
#endif
	//private:
#if defined(__linux__) || defined(__APPLE__)
	pthread_mutex_t lck;
#else
	CRITICAL_SECTION lck;
#endif
};

#endif