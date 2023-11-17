#include "atto_colors.h"
#include <cstdlib> // For std::strtoul

namespace atto {
    glm::vec4 Colors::FromHex( const char * h ) {
        // Make sure the input string is not null and has the correct length for a hex color code
        if( h == nullptr || std::strlen( h ) != 7 ) {
            Assert( false );
            return glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ); // Default to black
        }

        // Convert the hex color code to integer values
        unsigned int hexValue = std::strtoul( h + 1, nullptr, 16 ); // Skip the '#' character

        // Extract individual color components
        int red = ( hexValue >> 16 ) & 0xFF;
        int green = ( hexValue >> 8 ) & 0xFF;
        int blue = hexValue & 0xFF;

        // Normalize the color components to the range [0.0, 1.0]
        float normRed = static_cast<float>( red ) / 255.0f;
        float normGreen = static_cast<float>( green ) / 255.0f;
        float normBlue = static_cast<float>( blue ) / 255.0f;

        // Return a glm::vec4 with the extracted color components
        return glm::vec4( normRed, normGreen, normBlue, 1.0f );
    }
}
