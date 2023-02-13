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
#include "eventloop.h"

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
	int status;											//whether there's a peding event or not
    long last_active;									//Record the timestamp of each event
    event_handler *callback;							//Callback function (accept_con,read_data and write_data)
    void *arg;
}s_event;

extern int epollfd;                                        //Global epoll file descriptor (returned by epoll create)
extern s_event events_t[EVENTS_MAX + 1];                    //Global events table

static void read_data(int fd,void*arg,long time_now){
    s_event *ev = (s_event*) arg;
    char buf[BUFFLEN];
    int len;
    len = read(fd,buf,sizeof(buf));

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
    // At this point, the read data would be processed

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
    
    // set socket to NON BLOCKING
    if ((flag = fcntl(cfd, F_SETFL, O_NONBLOCK)) < 0) {
        printf("%s: fcntl(), %s\n", __func__, strerror(errno));
        return;
    }

    // registers read_data() as the event_handler callback function 
    event_set(&events_t[i],cfd,read_data,&events_t[i],time_now);  

    // adds the EPOLLIN event as the interested event for the newly createad connection (events_t[i]->fd)
    event_add(&events_t[i],epollfd, EPOLLIN);    
}

#endif