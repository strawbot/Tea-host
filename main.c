// Tea testing
#include "tea.c"

void state_machine() {
    static enum {START, MIDDLE, END} state = START;

    switch(state) {
    case START:
        state = MIDDLE;
        after(secs(2), state_machine);
        print("\nStart");
        break;
    case MIDDLE:
        state = END;
        after(secs(2), state_machine);
        print("\nMiddle");
        break;
    case END:
        state = START;
        print("\nEnd");
        break;
    }
}

void init() {
    print("\ninit");
    init_tea();
    later(state_machine);
}

int main() {
    init();
    serve_tea();
    return 0;
}