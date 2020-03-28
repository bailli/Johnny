# Johnny's World

Open Source C++/SDL implementation of the Screen Antics' Johnny Castaway screensaver

This is an attempt to recreate the old 16bit screensaver by Sierra/Screen Antics "Johnny Castaway" using the original resource files. You have to provide `RESOURCE.MAP` `RESOUCRE.001` and `SCRANTICS.SCR` in the same directory as the executable or set commandline options accordingly (see below).

Most of the file parsing code is ported from Hans Millings C# project: https://github.com/nivs1978/Johnny-Castaway-Open-Source

## Status

The program plays a few scenes almost perfect and many scenes okay. Some scenes break, because the timing between individual clips is off and of course because of unimplemented commands. Combining the individual movies into a smooth story has not been started at all, yet.

## Controls

* Space pauses/unpauses
* Return opens a menu to select a movie using left/right/up/down and return to select
* Escape closes the menu or exists the program

## Resources (un)packing

The application is able to unpack all resources to "normal" file types and repack them into a `RESOUCRE.MAP/.001` archive. There are some blobs in `RESOURCE.MAP` which are not understood and just copied over. However, repacked resources seem to work fine with the original executable.

Scrantic BMP and SCR files are unpacked to 32bit RGBA bitmaps, other resources result in plain text files. The text parser for repacking the resources is not very robust, so make sure to only feed it correctly formatted files. Sound samples will be extract to the subdirectory `RIFF`, but they will not be repacked!

## Commandline options

There are a few commandline options:

argument | help
------------ | -------------
`--resourcePath folder/` | set location of resource files
`--unpackedResources` | use unpacked resources
`--unpack` | only unpack resource archive and exit
`--onlyFiles` | unpack only "raw" files from resource archive
`--repack` | only repack resource into resource archive and exit
`--prepackedPath folder/` | point to a folder containing "raw" resources files<br>If a file is found in this directory it will be copied<br>into the resource archive, otherwise the unpacked file<br>from the resourcePath is repacked.<br>This is useful as BMP files take a few seconds each<br>to repack (because of my slow compression implementation)
`--outputPath folder/` | set output directory for --repack and --unpack

## Compilation

This has only been tested on Linux so far, but should work on every system with a C++11 compiler and SDL2. The includes might be incorrect for other environments. The C++11 experimental filesystem library is also used and its library is added manually to `CMakeLists.txt`.
The project depends on SDL2/Mixer/TTF/GFX. The library for the SDL2_gfx is added manually, too.

Compile using:

cmake . && make
