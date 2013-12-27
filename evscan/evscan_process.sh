#!/bin/bash
#remote Database
remote_ip='211.151.82.35'
remote_db="vianet_panbit"
remote_user="vianet"
remote_pwd='vianet21.com'

#Local Database
local_db="vianet_export"
local_user="root"
local_pwd='vianet21.com'
EPATH=/usr/local/evscan

count=`mysql -u$remote_user -p$remote_pwd $remote_db -h$remote_ip -e"select count(*) from acceleration where counter>1;"`

num=`echo $count | awk '{print $2}'`

deldata=`mysql -u$local_user -p$local_pwd $local_db -e "DELETE FROM vianet_pass_zj1"`

#

evscan=`ps -ef | grep -v grep | grep evscan.py | wc -l`

if [ $evscan -gt 0 ];then
	echo 'evscan exsits...'
	echo 'Cureent Python Process is ' $evscan
	exit
fi


echo 'Evscan Startd...'

/usr/bin/python  $EPATH/evscan.py  -s 0       -e 200000  -n 500 &
/usr/bin/python  $EPATH/evscan.py  -s 200000  -e 400000  -n 500 &
/usr/bin/python  $EPATH/evscan.py  -s 400000  -e 600000  -n 500 &
/usr/bin/python  $EPATH/evscan.py  -s 600000  -e 800000  -n 500 &
/usr/bin/python  $EPATH/evscan.py  -s 800000  -e 1000000 -n 500 &
/usr/bin/python  $EPATH/evscan.py  -s 1000000 -e 1200000 -n 500 &
/usr/bin/python  $EPATH/evscan.py  -s 1200000 -e 1400000 -n 500 &
/usr/bin/python  $EPATH/evscan.py  -s 1400000 -e 1600000 -n 500 &
/usr/bin/python  $EPATH/evscan.py  -s 1600000 -e 1800000 -n 500 &
/usr/bin/python  $EPATH/evscan.py  -s 1800000 -e 2000000 -n 500 &
/usr/bin/python  $EPATH/evscan.py  -s 2000000 -e 2100000 -n 500 &
/usr/bin/python  $EPATH/evscan.py  -s 2100000 -e 2200000 -n 500 &
/usr/bin/python  $EPATH/evscan.py  -s 2200000 -e 2400000 -n 500 &
/usr/bin/python  $EPATH/evscan.py  -s 2400000 -e 2600000 -n 500 &
/usr/bin/python  $EPATH/evscan.py  -s 2600000 -e 2800000 -n 500 &
/usr/bin/python  $EPATH/evscan.py  -s 2800000 -e 3000000 -n 500 &
/usr/bin/python  $EPATH/evscan.py  -s 3000000 -e 3200000 -n 500 &
/usr/bin/python  $EPATH/evscan.py  -s 3200000 -e 3400000 -n 500 &
/usr/bin/python  $EPATH/evscan.py  -s 3400000 -e $num    -n 500 &

# check evscan.py  finish

while true
do
	evscan=`ps -ef | grep -v grep | grep evscan.py | wc -l`

	if [ $evscan -eq 0 ];then

		cd  $EPATH/wuzhiyuan
		sh  Exec.sh	
		break

	else
		echo 'Process evscan.py exsits...'
		sleep 30m
	fi

done
