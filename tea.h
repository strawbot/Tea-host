#include "queue.h"
#include "sys/time.h"
#include "project_defs.h"

typedef long long ms_time;

#define mins(n) (60*secs(n))
#define secs(n) (1000*msec(n))
#define msec(n) (n)

typedef vector Event[1];

void after(Long offset, vector action);
void when(Event event, vector action);
void later(vector action);
void stop(vector action);

void init_tea();
void serve_tea();

void no_action();

#define sysTicks() clock()
#define SYS_TO_MS(t) (Long)((double)(t)*1000 / CLOCKS_PER_SEC)
#define SYS_TO_US(t) (Long)((double)(t)*1000*1000 / CLOCKS_PER_SEC)
