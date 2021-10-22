# webc2e

Emscripten compiled version of the Creatures Evolution Engine

TODO: Implement SDL Soundlib from the c3-source
TODO: Get it running in the emscripten browser runner without javascript complaing about unreachable code after compilation

Currently the code compiles with no errors. There are still some warnings and issues.

I would like to upgrade to SDL2 and use SDL2_Mixer instead of SDL 1.0 + SDL_Mixer, so as to take advantage of WebGL rendering. OpenAL would be the closest implementation to the browser, and I have started work to implement that in another project

## Building:

```
make clean
make -j<number of CPU cores>
```
