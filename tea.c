// TEA
#include "tea.h"
#include "printers.h"
#include "queue.c"
#include <stdio.h>

static QUEUE(10, afterq); // {due, action}*
static QUEUE(5, laterq); // {action}*

static bool debug = false;

static ms_time origin;
static Long ups = 0;

ms_time timeInMilliseconds(void) {
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return (((ms_time)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}

Long uptime_ms() { return timeInMilliseconds() - origin; }

void sleep_ms(Long tms) {
    struct timespec ts = {
        .tv_sec = tms / 1000,
        .tv_nsec = (tms % 1000) * 1000000
    };
    if (debug) printf("\nnow @  %lu ms  sleep for %lu ms", uptime_ms(), tms);
    while (nanosleep(&ts, &ts)) ups++;
}

static void keep_stats(Long now, Long due) {
    static Long maxdelay = 0;
    static Long mindelay = 1000;
    static Long avg = 0;
    Long delay = now - due;
    if (delay > maxdelay)  maxdelay = delay;
    if (delay < mindelay)  mindelay = delay;
    avg = (3*avg+delay)/4;
    if (debug) printf("\nrun: %lu  delay %lu ms  min %lu  max %lu  avg %lu", now, delay, mindelay, maxdelay, avg);
    fflush(stdout);
}

static void time_table() {
    Long now = uptime_ms();
    for (Byte n = queryq(afterq) / 2; n--;) {
        if (q(afterq) <= now) {
            keep_stats(now, pullq(afterq));
            later((vector)pullq(afterq));
        }
    }
}

void after(Long offset, vector action) { // due soonest at front of queue; latest at end of queue
    Long due = uptime_ms() + offset;
    Byte n = queryq(afterq) / 2;

    for (; n; n--) // linear search for insertion point
        if (due < q(afterq))
            break;
        else
            rotateq(afterq, 2);
    
    pushq(due, afterq);
    pushq((Cell)action, afterq);

    rotateq(afterq, n * 2);

    if (debug) printf("\nset: %lu ms,  due @ %lu ms  offset %lu  dues: %lu", uptime_ms(), due, offset, queryq(afterq)/2);
    fflush(stdout);
}

void when(Event event, vector action) {
    *event = action;
}

void later(vector action) {
    pushq((Cell)action, laterq);
}

void no_action() { if (debug) print("\nno action "); }

vector run_action() {
    if (queryq(laterq)) {
        vector action = (vector)pullq(laterq);
        action();
        return action;
    }
    return NULL;
}

void stop(vector action) {
    for (Byte l = queryq(laterq); l--;)
        if ((vector)q(laterq) == action)
            pullq(laterq);
        else
            rotateq(laterq, 1);
    
    for (Byte a = queryq(afterq)/2; a--;) {
        Cell due = pullq(afterq);
        if ((vector)q(afterq) == action)
            pullq(afterq);
        else {
            pushq(due, afterq);
            rotateq(afterq, 1);
        }
    }
}

void serve_tea() {
    for (;;) {
        if (queryq(laterq))
            run_action();

        if (queryq(afterq)) {
            if (queryq(laterq) == 0)
                sleep_ms((q(afterq) - uptime_ms()));
            time_table();
        } else if (queryq(laterq) == 0)
            break;
    }
    printf("\nfinished @ %lu ms  ups: %lu\n", uptime_ms(), ups);
}

void init_tea() {
    origin = timeInMilliseconds();
}