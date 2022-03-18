# LibNDI
Open-source, cross-platform ~~and maintained~~.

## Dependencies
* Ffmpeg: libavcodec, libavformat, libavutil
* Bonjour (MacOS and Windows)
* Avahi (Linux)

## Acknowledgements
This library is based on the work by Jean-Baptiste Kempf (https://code.videolan.org/jbk/libndi).

## License
LGPL v2.1

## Building (AA EDIT)

1. Install the following extra system dependencies, assuming you're on debian/ubuntu: `sudo apt install make cmake gcc g++ libavahi-client-dev libavcodec-dev libavformat-dev libglew-dev libglfw3-dev freeglut3-dev` 
2. `cmake -S . -B .` in the top directory
3. `cd examples/example-recv-gl`
5. `cmake -S . -B build`
6. `cd build && make`
* You may need to modify files in `examples/common/` to include the C standard lib.
