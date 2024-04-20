#pragma once

#include "../shared/atto_defines.h"
#include "../shared/atto_containers.h"
#include "../shared/atto_math.h"

namespace atto {

    class Core;
    class SpriteResource;

    class ContentSpriteAtlasProcessor {
    public:
        void Test();
        void Processs( Core * core, Span<SpriteResource> sprites );
        LargeString name;
    };
}
