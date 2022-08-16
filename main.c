#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <string.h>
#include <assert.h>
#include "eventloop.h"
#include "events.h"
#define PORT 8080
#define TIMEOUT 605

static inline void check_timeout(long now,int *checkpos);

int main()
{
    int port = PORT;
    epollfd = epoll_create(EVENTS_MAX+1);          
    assert (epollfd> 0);
    printf("epoll file descriptor [%d]\n ",epollfd);
    init_loop(epollfd, port);
    printf("%ld ",sizeof(s_event));
    struct epoll_event events[EVENTS_MAX+1];             
    printf("server running:port[%d]\n", port);

    int checkpos = 0, i;
    long end = time(NULL) + TIMEOUT;
    long now = time(NULL);
    while (1) {
        now=time(NULL);
        if(now>end)
            break;
        
        check_timeout(now,&checkpos);

        int nfd = epoll_wait(epollfd, events, EVENTS_MAX+1, 20);
        if (nfd < 0) {
            printf("epoll_wait error, exit\n");
            break;
        }
        
        for (i = 0; i < nfd; i++) {
            s_event *ev = (s_event*)events[i].data.ptr;  
            if ((events[i].events & EPOLLIN) && (ev->events & EPOLLIN))
                ev->callback(ev->fd, ev->arg,now);
        }
    }
    return 0;
}

static inline void check_timeout(long now,int *checkpos){
    int i;
    int ck = *checkpos;
    for (i = 0; i < 50; i++, ck++) {
            ck%=EVENTS_MAX;
            if (events_t[ck].status == EVENT_OFF){
                continue;
            }        
            long duration = now - events_t[ck].last_active;       
            if (duration >= 30L) {
                printf("[%d] ",ck);
                close(events_t[ck].fd);                           
                printf("[fd=%d] timeout\n", events_t[ck].fd);
                event_rm(&events_t[ck],epollfd);                   
            }
        
    }
    *checkpos=ck;
}