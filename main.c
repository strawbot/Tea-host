// Tea testing
#include "tea.c"

void state_machine() {
    static enum {START, MIDDLE, END} state = START;

    switch(state) {
    case START:
        state = MIDDLE;
        after(msec(2204), state_machine);
        print("\nStart");
        break;
    case MIDDLE:
        state = END;
        after(msec(1945), state_machine);
        print("\nMiddle");
        break;
    case END:
        state = START;
        print("\nEnd");
        break;
    }
}

void init_app() {
    print("\ninit");
    later(state_machine);
}

int main() {
    init_tea();
    init_app();
    serve_tea();
    return 0;
}