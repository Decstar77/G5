# Compile the source file using g++
cd atto_server/src

# Print a message indicating the start of compilation
echo "Compiling files from ../../atto_core/shared directory:"

# Create an empty string to store file paths
files=""

# Loop through all .cpp files in the "../../atto_core/shared" directory
for file in ../../atto_core/src/shared/*.cpp; do
    # Add each file to the compilation command
    echo " - $file"
    files="$files $file"
done

for file in ../../atto_core/src/sim/*.cpp; do
    # Add each file to the compilation command
    echo " - $file"
    files="$files $file"
done

g++ -o ../../server server_main.cpp atto_server_session.cpp $files ../../atto_core/src/platform/linux/atto_platform_linux.cpp -I../../vendor/glm -I../../vendor/fpm -I../../vendor/json -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -DATTO_SERVER -Wno-invalid-offsetof

