# webc2e

Emscripten compiled version of the Creatures Evolution Engine

Currently the code compiles with no errors. There are still some warnings and issues.

I would like to upgrade to SDL2 and use SDL2_Mixer instead of SDL 1.0 + SDL_Mixer, so as to take advantage of WebGL rendering. OpenAL would be the closest implementation to the browser, and I have started work to implement that in another project

## Building:

Requires emscripten SDK to be avialable in `$EMSDK_PATH` and normal c++ build tools to be available

The code expects the linux version of the data files to be available in `../../C2E/GAMES/Creatures\ 3` directory.

```
./scripts/clean.sh
emcmake cmake -DCMAKE_TOOLCHAIN_FILE=$EMSDK_PATH/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake -DCMAKE_CROSSCOMPILING_EMULATOR=$EMSDK_PATH/node/14.15.5_64bit/bin/node --build .
emmake make -j<number of CPU cores>
```

## Running:

```
emrun --no_browser webc2e.html
```

Access http://localhost:6931/ in Chrome
