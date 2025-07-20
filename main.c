// Tea testing
#include "tea.h"

void init_zc();
void init_decoder();
void init_correct();

int main() {
    init_tea();
    // init_zc();
    init_decoder();
    init_correct();
    serve_tea();
    return 0;
}
