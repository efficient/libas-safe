#ifndef PROXY_H_
#define PROXY_H_

void *proxy(unsigned lmid, void *(*fun)(void *), void *arg);

#endif
