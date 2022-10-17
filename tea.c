// TEA
#include "tea.h"
#include "queue.c"
#include <stdio.h>

static QUEUE(10, afterq);
static QUEUE(10, actionq);

static void time_table() {
    Byte n = queryq(afterq) / 2;

    for (Byte i = 0; i < n; i++) {
        Long due = pullq(afterq);
        vector action = (vector)pullq(afterq);

        if (time(NULL) >= due) {
            later(action);
            printf("\nnow: %u",(Long)time(NULL));
            fflush(stdout);
        } else {
            pushq(due, afterq);
            pushq((Cell)action, afterq);
        }
    }

    if (n)
        later(time_table);
}

void after(Long offset, vector action) {
    Long due = time(NULL) + offset;

    pushq(due, afterq);
    pushq((Cell)action, afterq);
    printf("\nnow: %u,  due: %u",time(NULL), due);
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
    while (run_action());
}

void init_tea() {
    later(time_table);
}

void print(char * string) { printf(string); fflush(stdout); }