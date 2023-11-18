#pragma once

#include "atto_defines.h"

namespace atto {

    class ContentTextureProcesses {
    public:
        static void FixAplhaEdges( byte * data, int width, int height );// Assumes to be RGBA8
    };

}
