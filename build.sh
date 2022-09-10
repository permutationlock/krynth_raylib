rm -Rf build
mkdir build
gcc -o build/server echo_server_epoll.c
gcc -c generator.c
gcc -c game_client.c
gcc -o build/game_client -lraylib generator.o game_client.o
