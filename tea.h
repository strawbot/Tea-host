#include "queue.h"
#include "time.h"

#define secs(n) n

typedef vector Event[1];
void after(Long offset, vector action);
void when(Event event, vector action);
void later(vector action);
void init_tea();
vector run_action();
void print(char * string);