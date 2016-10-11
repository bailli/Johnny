# Johnny's World

Open Source C++/SDL implementation of the Screen Antics' Johnny Castaway screensaver

This is an attempt to recreate the old 16bit screensaver by Sierra/Screen Antics "Johnny Castaway" using the original resource files. You have to provide RESOURCE.MAP RESOUCRE.001 and SCRANTICS.SCR in the same directory as the executable.

Most of the file parsing code is ported from Hans Millings C# project: https://github.com/nivs1978/Johnny-Castaway-Open-Source

## Status

The program plays a few scenes almost perfect and many scenes okay. Some scenes break, because the timing between individual clips is off and of course because of unimplemented commands. Combining the individual movies into a smooth story has not been started at all, yet.

## Controls

* Space pauses/unpauses
* Return opens a menu to select a movie using left/right/up/down and return to select
* Escape closes the menu or exists the program

## Compilation

This has only been tested on Linux so far, but should work on every system with a C++11 compiler and SDL2. The includes might be incorrect for other environments.
The project depends on SDL2/Mixer/TTF/GFX. The library for the SDL2_gfx is added manually - not by a cmake modules (see CMakeLists.txt) .

Compile using:

cmake . && make
