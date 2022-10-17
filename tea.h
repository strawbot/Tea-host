#include "queue.h"
#include "sys/time.h"

typedef long long ms_time;

#define mins(n) (60*secs(n))
#define secs(n) (1000*msec(n))
#define msec(n) (n)

typedef vector Event[1];
void after(Long offset, vector action);
void when(Event event, vector action);
void later(vector action);
void init_tea();
vector run_action();
void print(char * string);