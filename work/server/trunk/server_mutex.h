/*
 * frep_mutex.h
 *
 *  Created on: Apr 27, 2013
 *      Author: root
 */

#ifndef FREP_MUTEX_H_
#define FREP_MUTEX_H_

#include "pthread.h"

class CServerMutex
{
private:
	static pthread_mutex_t m_mutex;
	bool locked;
public:
	CServerMutex();
	~CServerMutex();
	void unlock();
};

class CMapMutex
{
private:
	static pthread_mutex_t m_mutex;
	bool locked;
public:
	CMapMutex();
	~CMapMutex();

};

#endif /* FREP_MUTEX_H_ */
