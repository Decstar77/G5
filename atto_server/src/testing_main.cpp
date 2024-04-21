
// Linux gaurd
#ifdef __linux__

#include <iostream>

#include "../../atto_core/src/shared/atto_defines.h"
#include "../../atto_core/src/shared/atto_containers.h"
#include "../../atto_core/src/shared/atto_binary_file.h"
#include "../../atto_core/src/shared/atto_network.h"
#include "../../atto_core/src/shared/atto_clock.h"
#include "../../atto_core/src/shared/atto_math.h"
#include "../../atto_core/src/shared/enki_task_scheduler.h"
#include "../../atto_core/src/shared/atto_reflection.h"
#include "../../atto_core/src/shared/atto_rpc.h"

#include <GLFW/glfw3.h>

int main() {
    if ( glfwInit() == 1 ) {
        std::cout << "GLFW initialized successfully" << std::endl;
    } else {
        std::cout << "GLFW initialization failed" << std::endl;
    }

    double theTime = glfwGetTime();
    std::cout << "Time: " << theTime << std::endl;

    return 0;
}

#endif
