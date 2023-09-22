mkdir web_server/build
clang generator.c -isystem ~/.emscripten_cache/sysroot/include/ -target wasm32-unknown-emscripten -c
clang game_logic.c -isystem ~/.emscripten_cache/sysroot/include/ -target wasm32-unknown-emscripten -c
clang game_client.c -isystem ~/.emscripten_cache/sysroot/include/ -I../raylib/src/ -target wasm32-unknown-emscripten -c
emcc -o web_server/build/index.html game_client.o generator.o game_logic.o --shell-file web_server/shell.html -Oz -Wall ./../lib_wasm/libraylib.a  -L../lib_wasm/ -s ASYNCIFY -s USE_GLFW=3 -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
