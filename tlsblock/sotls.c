#include <stdint.h>
#include <threads.h>

#define SIZE 100 MiB

//#define SECTION DATA
#define SECTION BSS

#define BSS
#define DATA = {1}

#define KiB * 1024
#define MiB KiB * 1024
#define GiB MiB * 1024

thread_local const uint8_t muchbigveryspacewow[SIZE] SECTION;
