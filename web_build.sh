emcc -c generator.c -Os -Wall -DPLATFORM_WEB
emcc -c game_logic.c -Os -Wall -DPLATFORM_WEB
emcc -c game_client.c -I../raylib/src/ -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -o web_server/build/client.html game_client.o generator.o game_logic.o --shell-file web_server/shell.html -Oz -Wall ./../lib_wasm/libraylib.a  -L../lib_wasm/ -s ASYNCIFY -s USE_GLFW=3 -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
