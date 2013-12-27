#!/bin/sh
{
sleep 1
echo "wuzy"
sleep 1
echo "wuzy"
sleep 1
echo "sys"
sleep 5
ping -c 4 192.168.38.197 >0.txt
sleep 2
ping -c 4 192.168.38.181 >1.txt
sleep 2
ping -c 4 192.168.38.197 >2.txt
sleep 2
ping -c 4 192.168.38.181 >3.txt
sleep 2
ping -c 4 192.168.38.197 >4.txt
sleep 2
 }| telnet 192.168.38.202
