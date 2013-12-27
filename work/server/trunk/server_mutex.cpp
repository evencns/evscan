/*
 * frep_mutex.cpp
 *
 *  Created on: Apr 27, 2013
 *      Author: root
 */

#include "server_mutex.h"

pthread_mutex_t CServerMutex::m_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t CMapMutex::m_mutex = PTHREAD_MUTEX_INITIALIZER;

CServerMutex::CServerMutex()
{
	pthread_mutex_lock(&m_mutex);
	locked = true;
}

CServerMutex::~CServerMutex()
{
	if(locked)
	{
		pthread_mutex_unlock(&m_mutex);
		locked = false;
	}
}

void CServerMutex::unlock()
{
	if(locked)
	{
		pthread_mutex_unlock(&m_mutex);
		locked = false;
	}
}

CMapMutex::CMapMutex()
{
	pthread_mutex_lock(&m_mutex);
}
CMapMutex::~CMapMutex()
{
	pthread_mutex_unlock(&m_mutex);
}
