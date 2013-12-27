#!/bin/sh
rm -rf *.o
rm -rf *.txt
rm -rf probe.sh
rm -rf Config.sh
./probe
chmod +x probe.sh
./probe.sh

grep "4 received, 0% packet loss" 0.txt >00.txt
grep "4 received, 0% packet loss" 1.txt >11.txt
grep "4 received, 0% packet loss" 2.txt >22.txt
grep "4 received, 0% packet loss" 3.txt >33.txt
grep "4 received, 0% packet loss" 4.txt >44.txt


touch Uport.txt
if [ -s "00.txt" ]; then
echo -n `date` >>Cov.log
echo  " Port A can be connect!" >>Cov.log
echo -n "A">>Uport.txt
else
echo -n "0">>Uport.txt
echo -n `date` >>Cov.log
echo  " Port A can not be connect!" >>Cov.log
fi

if [ -s "11.txt" ]; then
echo -n `date` >>Cov.log
echo  " Port B can be connect!" >>Cov.log
echo -n "B">>Uport.txt
else
echo -n "0">>Uport.txt
echo -n `date` >>Cov.log
echo  " Port B can not be connect!" >>Cov.log
fi

if [ -s "22.txt" ]; then
echo -n `date` >>Cov.log
echo  " Port C can be connect!" >>Cov.log
echo -n "C">>Uport.txt
else 
echo -n "0">>Uport.txt
echo -n `date` >>Cov.log
echo  " Port C can not be connect!" >>Cov.log
fi

if [ -s "33.txt" ]; then
echo -n `date` >>Cov.log
echo  " Port D can be connect!" >>Cov.log
echo -n "D">>Uport.txt
else 
echo -n "0">>Uport.txt
echo -n `date` >>Cov.log
echo  " Port D can  not be connect!" >>Cov.log
fi

if [ -s "44.txt" ]; then
echo -n `date` >>Cov.log
echo  " Port E can be connect!" >>Cov.log
echo -n "E">>Uport.txt
else 
echo -n "0">>Uport.txt
echo -n `date` >>Cov.log
echo  " Port E can  not be connect!" >>Cov.log
fi


if [ -f "new.txt" ]; then
echo -n `date` >>Cov.log
echo  " rename the new.txt to Lastest.cfg" >>Cov.log
rm -f new.txt
else 
echo -n `date` >>Cov.log
echo  " new.txt is not output" >>Cov.log
fi

if [ -f "Config.sh" ]; then
echo -n `date` >>Cov.log
echo  " remove the last config.sh file" >>Cov.log
rm -f Config.sh
else 
echo -n `date` >>Cov.log
echo  " Config.sh not output" >>Cov.log
fi


if [ -f "ipstatic.txt" ]; then
echo -n `date` >>Cov.log
echo  " remove the last ipstatic.txt file" >>Cov.log
rm -f ipstatic.txt
else 
echo -n `date` >>Cov.log
echo  " ipstatic.txt is not output" >>Cov.log
fi

sleep 1

./roucoverge
if [ -s "Lastest.cfg" ]; then
echo -n `date` >>Cov.log
echo  " Begin to compare" >>Cov.log
rm -rf Config.sh
rm -rf ipstatic.txt
./compare
else 
echo -n `date` >>Cov.log
echo  " old.txt is not exist or empty" >>Cov.log
fi


rm -rf Lasthop.cfg
cp Nexthop.cfg Lasthop.cfg
rm -rf Lastest.cfg
if [ -s "new.txt" ]; then
echo -n `date` >>Cov.log
echo  "Rename the new.txt to Lastest.cfg" >>Cov.log
echo  "Route Coverge has been complete successfully !" >>Cov.log
mv new.txt Lastest.cfg
else 
echo -n `date` >>Cov.log
echo  " new.txt is not output" >> Cov.log
fi

rm -rf *.txt
chmod +x Config.sh
exec ./Config.sh


