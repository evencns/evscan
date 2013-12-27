/*
 * log.h
 *
 *  Created on: Mar 27, 2013
 *      Author: root
 */

#ifndef LOG_H_
#define LOG_H_

#include <syslog.h>
#include <string.h>

#define LOG_INFO_MSG(format,args...) { \
    if (NULL != format) { \
    	char buffer[2048] = {'\0'}; \
    	strcat(buffer,"FILE %s LINE %d FUNCTION %s "); \
    	strcat(buffer,format); \
    	strcat(buffer," \n"); \
    	syslog(LOG_INFO,buffer,__FILE__,__LINE__,__FUNCTION__,##args); \
    } \
}

#define LOG_ERR_MSG(format,args...) { \
    if (NULL != format) { \
    	char buffer[2048] = {'\0'}; \
    	strcat(buffer,"FILE %s LINE %d FUNCTION %s "); \
    	strcat(buffer,format); \
    	strcat(buffer," \n"); \
    	syslog(LOG_ERR,buffer,__FILE__,__LINE__,__FUNCTION__,##args); \
    } \
}

#define LOG_WARN_MSG(format,args...) { \
    if (NULL != format) { \
    	char buffer[2048] = {'\0'}; \
    	strcat(buffer,"FILE %s LINE %d FUNCTION %s "); \
    	strcat(buffer,format); \
    	strcat(buffer," \n"); \
    	syslog(LOG_WARNING,buffer,__FILE__,__LINE__,__FUNCTION__,##args); \
    } \
}

#endif /* LOG_H_ */
