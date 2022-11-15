// Tea testing
#include "tea.c"

Event happen;

void state_machine() {
    static enum {START, MIDDLE, END} state = START;

    switch(state) {
    case START:
        state = MIDDLE;
        after(msec(2065), state_machine);
        print("\nStart");
        break;
    case MIDDLE:
        state = END;
        after(msec(1945), state_machine);
        print("\nMiddle");
        break;
    case END:
        state = START;
        after(secs(1), state_machine);
        later(*happen);
        break;
    }
}

void stop_state_machine() {
    stop(state_machine);
    print("\nFull Stop");
}

void announce() { print("\nEnd event"); }

void init_app() {
    print("\ninit");
    later(state_machine);
    when(happen, announce);
    after(secs(20), stop_state_machine);
}

int main() {
    init_tea();
    init_app();
    serve_tea();
    return 0;
}
