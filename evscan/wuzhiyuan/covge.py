#!/usr/bin/python2.6
# -*-coding:utf-*-
import socket
import struct
import MySQLdb

def ip2int(ip):
	return struct.unpack('!L',socket.inet_aton(ip))[0]
	
route= [0,1,2,3,4]
for i in range(0,5):
	route[i] = []
fp = open("Reserve.cfg","r")
reserve = fp.readlines() 
for i in range(0,5):
	reserve[i] = ip2int(reserve[i])
fp.close()
print "This the reserve address!"
print reserve

	
def getaddr(i):
	connect = MySQLdb.connect(host="127.0.0.1",user="root",passwd="vianet21.com",db="vianet_export")
	cursor  = connect.cursor()	
	select = "select ip from vianet_pass_zj1 where port_channel ='"
	ch = chr(ord('A')+i)
	select +=ch
	select +="'"
	print select
	cursor.execute(select)
	print "There is total:%s selected" %(cursor.rowcount)
	for row in cursor.fetchall():
		addr=ip2int(row[0])
		if (0 == reserve.count(addr)):
			route[i].append(addr)
		else:
			print "Sorry! this address is reserved!!"
	route[i].sort()
	print  "after sort the %d list is :" % i
	#print route[i]	
	print "\n"
	
	cursor.close()
	connect.commit()	
	connect.close()
def cover(list):
	for j in range(len(list)-1):
		if((~(list[j]^list[j+1]))>= 0xFFFFFF00):
			del list[j+1]
			print "this 2 addr can be coverge !"
if __name__=="__main__":
	for i in range(0,5):
		getaddr(i)
		cover(route[i])
		#print route[i]
