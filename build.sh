rm -Rf build
mkdir build
gcc -o build/server echo_server_epoll.c
gcc -c generator.c -O2
gcc -c game_logic.c -O2
gcc -c game_client.c -O2
gcc -o build/game_client -O2 -lraylib generator.o game_logic.o game_client.o
