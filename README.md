# Krynth - a deduction game

I designed this game way back in 2020 after playing and loving the board game
[Cryptid][5]. The first iteration was the (now defunct) competitive online game
[krynth.io][1] that I hosted with my C++ [simple web game server][2]. I ended up
retiring the game after a year and a half because I did not see a future with
the design as a head-to-head game (the server worked fantastically though,
100% uptime... for all thirty-fourty unique players that it saw).

What this repo contains is a single-player Raylib client for mostly the same
puzzle generation logic found in the competitive game. It can be built to
a native binary or for the web with [Emscripten][3],
but the current build system is just a set of shell scripts.

The web build of the game can be played [on my website][4]!

[1]: https://github.com/permutationlock/krynth.io
[2]: https://github.com/permutationlock/simple_web_game_server
[3]: https://emscripten.org/
[4]: https://permutationlock.com/krynth/
[5]: https://www.playcryptid.com/
