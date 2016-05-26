# Johnny's World

Open Source C++/SDL implementation of the Screen Antics' Johnny Castaway screensaver

This is an attempt to recreate the old 16bit screensaver by Sierra/Screen Antics "Johnny Castaway" using the original resource files. You have to provide RESOURCE.MAP RESOUCRE.001 and SCRANTICS.SCR in the same directory as the executable.

Most of the file parsing code is ported from Hans Millings C# project: https://github.com/nivs1978/Johnny-Castaway-Open-Source

## Status

The current status plays a few scenes perfect and many scenes okay. Scenes, which are supposed to play short clips in parallel, are probably the most important feature which is missing. Combining the individual movies into a smooth story has not been started at all, yet.

## Controls

* Space pauses/unpauses
* Return opens a menu to select a movie using left/right/up/down and return to select
* Escape closes the menu or exists the program

## Compilation

The project depends on SDL2/Mixer/TTF/GFX. The library for the SDL2_gfx is added manually - not by cmake modules.

Compile using:

cmake . && make
