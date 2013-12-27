PROGRAM = via_server
EXECNAME = __EXECNAME__="\"$(PROGRAM)\""
VERSION_ID = 2.0.0.0

CC = g++

# gcc libipq.c ipq_user.c -o ipq_user
CFLAGS = -DLINUX -D_LINUX_  -I. -g -Wall -Werror  `xml2-config --cflags` 


LIB_DIR = -L /usr/lib64/mysql
LIBS = -lrt -lpthread -lmysqlclient_r
OBJS = main.o
OBJS += server_log.o server_mutex.o  mysql_db.o thread_pool.o common_util.o

all: via_server

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@ 
.cpp.o:
	$(CC) -c $(CFLAGS) $< -o $@

via_server: $(OBJS)
	$(CC) -o via_server $(OBJS) $(LIB_DIR) $(LIBS)
	
clean:
	rm -f  *.bak *.o  *.core core.* via_server
	rm -rf ./log *.log

