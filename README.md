# RB Tree visualizer built using raylib only

This tool helps visualize how a red black is constructed.

The idea to build that small project came from [this other repo](https://github.com/Kenships/RedBlackTreeVisual)

This project is lightweight, the only library required is raylib. 

I have also compiled the code to webassembly, the visualizer can be tried on [my website](https://bartholomehubert.com/rbtreeviz/index.html)

## Build

This project is intended to be used on linux. The makefile contains a single build command, feel free to edit it.

The `buildweb.sh` script compiles the project and the raylib library to webassembly. It is required that emscripten is installed and that the PATH environment varialbe is set correctly.
