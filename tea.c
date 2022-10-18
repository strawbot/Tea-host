// TEA
#include "tea.h"
#include "printer.h"
#include "queue.c"

static QUEUE(10, afterq);
static QUEUE(10, actionq);

static ms_time origin;

ms_time timeInMilliseconds(void) {
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return (((ms_time)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}

Long uptime_ms() { return timeInMilliseconds() - origin; }

Long ups = 0;

void sleep_ms(Long tms) {
    struct timespec ts = {
        .tv_sec = tms / 1000,
        .tv_nsec = (tms % 1000) * 1000000
    };
    printf("\nnow @  %u ms  sleep for %u ms", uptime_ms(), tms);
    while (nanosleep(&ts, &ts)) ups++;
}

static void time_table() {
    Byte n = queryq(afterq) / 2;

    for (Byte i = 0; i < n; i++) {
        Long due = pullq(afterq);
        vector action = (vector)pullq(afterq);
        Long now = uptime_ms();
        Long delay = now - due;
        if (now >= due) {
            static Long maxdelay = 0;
            static Long mindelay = 1000;
            static Long avg = 0;
            if (delay > maxdelay)  maxdelay = delay;
            if (delay < mindelay)  mindelay = delay;
            avg = (3*avg+delay)/4;
            later(action);
            printf("\nrun: %u  delay %u ms  min %u  max %u  avg %u", now, delay, mindelay, maxdelay, avg);
            fflush(stdout);
        } else {
            pushq(due, afterq);
            pushq((Cell)action, afterq);
        }
    }
}

void after(Long offset, vector action) {
    Long due = uptime_ms() + offset;

    pushq(due, afterq);
    pushq((Cell)action, afterq);
    printf("\nset: %u ms,  due @ %u ms  offset %u", uptime_ms(), due, offset);
    fflush(stdout);
}

void when(Event event, vector action) {
    *event = action;
}

void later(vector action) {
    pushq((Cell)action, actionq);
}

void no_action() { print("\nno action "); }

vector run_action() {
    if (queryq(actionq)) {
        vector action = (vector)pullq(actionq);
        action();
        return action;
    }
    return NULL;
}

void serve_tea() {
    for (;;) {
        if (queryq(actionq))
            run_action();

        if (queryq(afterq)) {
            if (queryq(actionq) == 0)
                sleep_ms((q(afterq) - uptime_ms()));
            time_table();
        } else if (queryq(actionq) == 0)
            break;
    }
    printf("\nfinished @ %u ms  ups: %u", uptime_ms(), ups);
}

void init_tea() {
    origin = timeInMilliseconds();
}