# simple-event-loop

This is a very simplified implementation of an event loop, written in C. 
Event loop, also known as Event Queue or Run Loop, is a design pattern that is widely used on different application domains, from game programming to kernels. One of the most known implementation of event loops is the one backing Node Js., but many other famous applications use it, like Redis and NGINX. This simple example that I wrote was able to achieve 75k TCP transactions per sec -4,5M p/ minute- with little to no effort for tunning, showing how powerful this design pattern can be when it comes to performance.

The image bellow is an illustration of an event loop for sockets, and it's very similar to this implementation .
| ![space-1.jpg](https://user-images.githubusercontent.com/42453088/218470745-96f1a414-3e6a-43e6-8268-258c28899cdc.png) |
|:--:|
| <b>Fig.1 - https://www.nginx.com/blog/inside-nginx-how-we-designed-for-performance-scale/</b>|


# Running 

## Prerequisites:
- Must have GCC installed
- Must use a LINUX environment

## Step-by-step
``` 
  git clone https://github.com/v-barros/simple-event-loop.git
  cd simple-event-loop
  make
  ./server
```
  
