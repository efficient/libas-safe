#ifndef HOOK_H_
#define HOOK_H_

#include <stddef.h>

extern void (*_GLOBAL_OFFSET_TABLE_[])(void);

void hook_got(void (*got[])(void), void (*hook)(size_t));

#define hook_got(hook) hook_got(_GLOBAL_OFFSET_TABLE_, hook)

#endif
