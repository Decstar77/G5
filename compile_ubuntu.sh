# Compile the source file using g++
cd atto_server/src
g++ -o main main.cpp ../../atto_core/src/shared/atto_containers.cpp ../../atto_core/src/platform/linux/atto_platform_linux.cpp ../../atto_core/src/shared/enki_task_scheduler.cpp -I../../vendor/glm -I../../vendor/fpm -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl
