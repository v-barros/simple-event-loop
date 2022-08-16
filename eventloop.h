#ifndef EVENTLOOP_H_
#define EVENTLOOP_H_
#include "events.h"

void init_loop(int epoll_instance_fd, int port);
void event_set(s_event * ev, int fd, event_handler callback, void * arg,long time_now);
void event_rm(s_event * ev, int fd);
void event_add(s_event *ev, int fd, int event);

#endif