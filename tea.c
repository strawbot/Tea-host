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

void sleep_ms(Long tms) {
    struct timeval tv;
    tv.tv_sec  = tms / 1000;
    tv.tv_usec = (tms % 1000) * 1000;
    select (0, NULL, NULL, NULL, &tv);
}

static void time_table() {
    Byte n = queryq(afterq) / 2;

    for (Byte i = 0; i < n; i++) {
        Long due = pullq(afterq);
        vector action = (vector)pullq(afterq);

        if (uptime_ms() >= due) {
            later(action);
            printf("\nrun: %u",(Long)uptime_ms());
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
    printf("\nset: %u ms,  due @ %u ms", uptime_ms(), due);
    fflush(stdout);
}

void when(Event event, vector action) {
    *event = action;
}

void later(vector action) {
    pushq((Cell)action, actionq);
}

void no_action() { printf("\nno action "); }

vector run_action() {
    if (queryq(actionq)) {
        vector action = (vector)pullq(actionq);
        action();
        return action;
    }
    return NULL;
}

void serve_tea() {
    bool busy = false;
    do {
        if ((busy = queryq(afterq)))
            time_table();
        if ((busy |= queryq(actionq)))
            run_action();
    } while (busy);
    printf("\nfinished @ %u ms", uptime_ms());
}

void init_tea() {
    origin = timeInMilliseconds();
    later(time_table);
}