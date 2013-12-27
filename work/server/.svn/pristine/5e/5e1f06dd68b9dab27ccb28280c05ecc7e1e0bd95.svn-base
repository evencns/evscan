/*
 * common_util.cpp
 *
 *  Created on: Nov 7, 2013
 *      Author: root
 */
#include "common_header.h"

uint64_t ntoh64(uint64_t val)
{
    if (__BYTE_ORDER == __LITTLE_ENDIAN)
    {
    	return ___constant_swab64(val);
    }
    else if (__BYTE_ORDER == __BIG_ENDIAN)
    {
        return val;
    }
}

uint64_t htonll(uint64_t val)
{
    if (__BYTE_ORDER == __LITTLE_ENDIAN)
    {
    	return ___constant_swab64(val);
    }
    else if (__BYTE_ORDER == __BIG_ENDIAN)
    {
        return val;
    }
}





