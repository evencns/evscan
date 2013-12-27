#!/usr/bin/env python
# -*- coding: utf-8 -*-
#

import os,sys,urllib
import statvfs
#import time
import traceback
import unicodedata
import codecs

import ConfigParser
cf = ConfigParser.ConfigParser()
cf.read("config.ini")


def get_config(section,item, default=''):
	if cf.has_option(section, item): 
		return cf.get(section,item)
	else: 
		return default

def	mysleep(n):
	time.sleep(n)


def	escapeString(s):
	if s==None: return ""
	s=s.replace("'", r"\'")
	s=s.replace('"', r'\"')
	return s

def procException(logger):
	traceback.print_exc()
	logger.error("exception:"+str(sys.exc_info()[1]))


#timeout = int(get_config("env", "s_timeout"))
