#include "eventloop.h"
#include "events.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
// set up new listening socket on fd using port
int init_conn(int fd, int port){
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    assert(sockfd !=-1);
    // set socket to NON BLOCKING
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    
    assert((bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) == 0);
   
    assert((listen(sockfd, 20)) == 0);

    return sockfd;
        
}

void init_loop(int epoll_instance_fd,int port){
    int socket = init_conn(epoll_instance_fd,port);
    printf("listening socket fd [%d]\n",socket);    
    event_set(&events_t[EVENTS_MAX], socket, accept_con, &events_t[EVENTS_MAX],time(NULL));
    event_add(&events_t[EVENTS_MAX],epoll_instance_fd,EPOLLIN);
}

void event_set(s_event * ev, int fd, event_handler callback, void * arg,long time_now){
    ev->fd = fd;
    ev->callback = callback;
    ev->events = 0;
    ev->arg = arg;
    ev->status = EVENT_OFF;
    ev->last_active = time_now;    
}

void event_rm(s_event * ev, int fd){
    struct epoll_event e_event = {0, {0}};

    if (ev->status == EVENT_OFF)                                        
        return ;

    e_event.data.ptr = ev;
    ev->status = EVENT_OFF;                                    
    epoll_ctl(fd, EPOLL_CTL_DEL, ev->fd, &e_event);
}

void event_add(s_event *ev, int fd, int event){

    struct epoll_event e_event = {0, {0}};
    int op;
    e_event.data.ptr = ev;
    e_event.events = ev->events = event;    //EPOLLIN or EPOLLOUT

    if (ev->status == EVENT_ON) {                                          
        op = EPOLL_CTL_MOD;
    } else {                   
        op = EPOLL_CTL_ADD;
        ev->status = EVENT_ON;
    }
    if (epoll_ctl(fd, op, ev->fd, &e_event) < 0)
    {                   
        printf("%s: epoll_ctl, %s\n", __func__, strerror(errno));
    }
}