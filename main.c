// Tea testing
#include "tea.h"

void init_zc();
void init_decoder();


int main() {
    init_tea();
    // init_zc();
    init_decoder();
    serve_tea();
    return 0;
}
