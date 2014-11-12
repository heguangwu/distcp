/*
 * MutexLock.h
 *
 *  Created on: 2014-10-23
 *      Author: hgw
 */

#ifndef MUTEXLOCK_H_
#define MUTEXLOCK_H_

#include <pthread.h>

class MutexLock
{
public:
	MutexLock() { pthread_mutex_init(&mutex_, NULL);}
	~MutexLock() { pthread_mutex_destroy(&mutex_); }
	void lock()
	{
		pthread_mutex_lock(&mutex_);
	}

	void unlock()
	{
		pthread_mutex_unlock(&mutex_);
	}

private:
	pthread_mutex_t mutex_;

//noncopyable
private:
	MutexLock( const MutexLock& );
	MutexLock& operator=( const MutexLock& );
};

class MutexLockGuard
{
public:
	explicit MutexLockGuard(MutexLock& mutex)
		: mutex_(mutex)
	{
		mutex_.lock();
	}

	~MutexLockGuard()
	{
		mutex_.unlock();
	}

private:
	MutexLock& mutex_;

//noncopyable
private:
	MutexLockGuard( const MutexLockGuard& );
	MutexLockGuard& operator=( const MutexLockGuard& );
};

#endif /* MUTEXLOCK_H_ */
