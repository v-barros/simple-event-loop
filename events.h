#ifndef EVENTS_H_
#define EVENTS_H_
#define BUFFLEN 1024
#define EVENTS_MAX 1024 
#include <stdio.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

enum EVENT_STATUS{
	EVENT_ON = 1,
#define EVENT_ON EVENT_ON
    EVENT_OFF = 0
#define EVENT_OFF EVENT_OFF
};

static void read_data(int fd,void*arg,long time_now);
static void accept_con(int fd,void*arg,long time_now);
typedef void event_handler (int,void*,long);

typedef struct s_event {
	int fd;												//socket file descriptor
    int events;											// read or write
	void *arg;
	event_handler *callback;							//Callback function (accept_con,read_data and write_data)
	int status;											//whether there's a peding event or not
	char buf[BUFFLEN];                                  //content buffer
    int len;                                            //buffer length
    long last_active;									//Record the timestamp of each event
}s_event;

int epollfd;                                         //Global epoll file descriptor (returned by epoll create)
s_event events_t[EVENTS_MAX + 1];                    //Global events table

static void read_data(int fd,void*arg,long time_now){
    s_event *ev = (s_event*) arg;
    int len;
    len = read(fd,ev->buf,sizeof(ev->buf));
    if(len==0){
        event_rm(ev,epollfd);
        close(fd);
        printf("connection closed by client\n");
        return;
    }else if(len<0){
        event_rm(ev,epollfd);
        close(fd);
        printf("%s read(), error: %s\n",__func__,strerror(errno));
        return;
    }
    
    ev->len = len;
    ev->buf[len]='\0';
    if(write(fd,"OK",3)!=3){
        event_rm(ev,epollfd);
        printf("failed to send all data\n");
	    close(fd); /* failed to send all data at once, close */
        return;
    }
    ev->last_active=time_now;
}

static void accept_con(int fd,void*arg,long time_now)
{
    struct sockaddr_in cin;
    socklen_t addr_len = sizeof(cin);
    int cfd, i;

    if ((cfd = accept(fd, (struct sockaddr *)&cin,&addr_len)) == -1) {
        printf("%s: accept, %s\n", __func__, strerror(errno));
        return ;
    }
    //finds first available event on events table
    for (i = 0; i < EVENTS_MAX; i++) 
        if (events_t[i].status == EVENT_OFF)                              
            break;       

    if (i == EVENTS_MAX) {
        printf("%s: connection overflow [%d]\n", __func__, EVENTS_MAX);
        return;
    }
    int flag = 0;
    if ((flag = fcntl(cfd, F_SETFL, O_NONBLOCK)) < 0) {
        printf("%s: fcntl(), %s\n", __func__, strerror(errno));
        return;
    }
    event_set(&events_t[i],cfd,read_data,&events_t[i],time_now);  
    event_add(&events_t[i],epollfd, EPOLLIN);    
}

#endif