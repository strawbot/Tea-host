#ifndef TEA_H
#define TEA_H

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
void action_slice(void);
bool pending_actions(void);

void init_tea();
void serve_tea();

// Aligning with the embedded tea.h's API (TimbreOS/tea.h): cli.c calls
// both of these directly. actionRun()/actor() already existed in tea.c
// (actionRun just calls the action; actor is a no-op stub - host builds
// don't need the embedded action-name/stats hash table these normally
// feed), so only the macros themselves were missing.
#define now(action) actionRun(action)
#define namedAction(m) actor(m, #m)
void actionRun(vector action);
void actor(vector action, const char *name);

void no_action();

#define sysTicks() clock()
#define SYS_TO_MS(t) (Long)((double)(t)*1000 / CLOCKS_PER_SEC)
#define SYS_TO_US(t) (Long)((double)(t)*1000*1000 / CLOCKS_PER_SEC)

#define safe(code) 	{ code }

#endif