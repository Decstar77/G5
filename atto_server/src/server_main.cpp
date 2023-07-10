
#include "../../atto_core/src/shared/atto_server.h"

int main(int argc, char* argv[]) {
    atto::Server* s = new atto::Server(27164, 32);
    s->Run();
    return 0;
}
